#include "CoroutineSocket.h"
#include "CoroutineSchedule.h"
#include <fcntl.h>
#include "wepoll.h"

#define CorSchedSington	(CCoroutineSchedule::getInstance())
#define CorSington		(CCoroutine::getInstance())

//创建socket
int CCoroutineSocket::CorSocket(int domain, int type, int protocol)
{
	int fd = socket(domain, type, protocol);
	if (fd == -1)
	{
		printf("Failed to create a new socket\n");
		return -1;
	}

	SetNoBlock(fd);
	SetNoDelay(fd);

	return fd;
}

//接收客户端连接
int CCoroutineSocket::CorAccept(int fd, struct sockaddr * addr, socklen_t * len)
{
	int sockfd = -1;
	int timeout = 1;
	coroutine * co = nty_coroutine_get_sched()->m_current_thread;

	while (1)
	{
		struct pollfd fds;
		fds.fd = fd;
		fds.events = POLLIN | POLLERR | POLLHUP;
		PollInner(&fds, 1, timeout);

		sockfd = accept(fd, addr, len);
		if (sockfd < 0)
		{
			if (errno == EAGAIN)
			{
				continue;
			}
			else if (errno == ECONNABORTED)
			{
				printf("accept : ECONNABORTED\n");
			}
			else if (errno == EMFILE || errno == ENFILE)
			{
				printf("accept : EMFILE || ENFILE\n");
			}
			return -1;
		}
		else
		{
			break;
		}
	}

	SetNoBlock(fd);
	//SetNoDelay(fd);

	return sockfd;
}

//数据接收
int CCoroutineSocket::CorRecv(int fd, void * buf, size_t len, int flags)
{
	struct pollfd fds;
	fds.fd = fd;
	fds.events = POLLIN | POLLERR | POLLHUP;

	PollInner(&fds, 1, 1);

	int ret = recv(fd, (char *)buf, len, flags);
	if (ret < 0)
	{
		// if (errno == EAGAIN) return ret;
		if (errno == ECONNRESET)
		{
			return -1;
		}
		// printf("recv error : %d, ret : %d\n", errno, ret);
	}

	return ret;
}

//数据发送
int CCoroutineSocket::CorSend(int fd, const void * buf, size_t len, int flags)
{
	int sent = 0;

	int ret = send(fd, ((char *)buf) + sent, len - sent, flags);
	//printf("send --> len : %d\n", ret);
	if (ret == 0)
	{
		return ret;
	}

	if (ret > 0)
	{
		sent += ret;
	}

	while (sent < len)
	{
		struct pollfd fds;
		fds.fd = fd;
		fds.events = POLLOUT | POLLERR | POLLHUP;

		PollInner(&fds, 1, 1);
		ret = send(fd, ((char *)buf) + sent, len - sent, flags);
		
		if (ret <= 0)
		{
			break;
		}
		sent += ret;
	}

	if (ret <= 0 && sent == 0)
	{
		return ret;
	}

	return sent;
}

//udp数据接收
int CCoroutineSocket::CorRecvfrom(int fd, void * buf, size_t len, int flags, struct sockaddr * src_addr, socklen_t * addrlen)
{
	struct pollfd fds;
	fds.fd = fd;
	fds.events = POLLIN | POLLERR | POLLHUP;

	PollInner(&fds, 1, 1);

	int ret = recvfrom(fd, (char *)buf, len, flags, src_addr, addrlen);
	if (ret < 0)
	{
		if (errno == EAGAIN)
		{
			return ret;
		}

		if (errno == ECONNRESET)
		{
			return 0;
		}

		printf("recv error : %d, ret : %d\n", errno, ret);
		assert(0);
	}
	return ret;
}

//udp数据发送
int CCoroutineSocket::CorSendto(int fd, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr, socklen_t addrlen)
{
	int sent = 0;

	while (sent < len)
	{
		struct pollfd fds;
		fds.fd = fd;
		fds.events = POLLOUT | POLLERR | POLLHUP;

		PollInner(&fds, 1, 1);
		int ret = sendto(fd, ((char *)buf) + sent, len - sent, flags, dest_addr, addrlen);
		if (ret <= 0)
		{
			if (errno == EAGAIN)
			{
				continue;
			}
			else if (errno == ECONNRESET)
			{
				return ret;
			}
			printf("send errno : %d, ret : %d\n", errno, ret);
			assert(0);
		}
		sent += ret;
	}
	return sent;
}

//关闭连接
int CCoroutineSocket::CorClose(int fd)
{
#if 0
	nty_schedule *sched = nty_coroutine_get_sched();

	nty_coroutine *co = sched->curr_thread;
	if (co) {
		TAILQ_INSERT_TAIL(&nty_coroutine_get_sched()->ready, co, ready_next);
		co->status |= BIT(NTY_COROUTINE_STATUS_FDEOF);
	}
#endif

	return closesocket(fd);
}

//
uint32_t CCoroutineSocket::PollEvent2Epoll(short events)
{
	uint32_t e = 0;
	if (events & POLLIN) 	e |= EPOLLIN;
	if (events & POLLOUT)  e |= EPOLLOUT;
	if (events & POLLHUP) 	e |= EPOLLHUP;
	if (events & POLLERR)	e |= EPOLLERR;
	if (events & POLLRDNORM) e |= EPOLLRDNORM;
	if (events & POLLWRNORM) e |= EPOLLWRNORM;
	return e;
}

//
short CCoroutineSocket::PollEvent2Epoll(uint32_t events)
{
	short e = 0;
	if (events & EPOLLIN) 	e |= POLLIN;
	if (events & EPOLLOUT) e |= POLLOUT;
	if (events & EPOLLHUP) e |= POLLHUP;
	if (events & EPOLLERR) e |= POLLERR;
	if (events & EPOLLRDNORM) e |= POLLRDNORM;
	if (events & EPOLLWRNORM) e |= POLLWRNORM;
	return e;
}

/*
* nty_poll_inner --> 1. sockfd--> epoll, 2 yield, 3. epoll x sockfd
* fds :
*/
int CCoroutineSocket::PollInner(struct pollfd * fds, int nfds, int timeout)
{
	if (timeout == 0)
	{
#if ((defined _WIN32) || (defined __APPLE__))
		return WSAPoll(fds, nfds, timeout);
#else
		return poll(fds, nfds, timeout);
#endif		
	}

	if (timeout < 0)
	{
		timeout = INT_MAX;
	}

	coroutine_schedule * sched = nty_coroutine_get_sched();
	coroutine * co = sched->m_current_thread;

	int i = 0;
	for (i = 0; i < nfds; i++)
	{

		struct epoll_event ev;
		ev.events = PollEvent2Epoll(fds[i].events);
		ev.data.fd = fds[i].fd;
		epoll_ctl(sched->m_poll_fd, EPOLL_CTL_ADD, fds[i].fd, &ev);

		co->m_events = fds[i].events;
		CorSchedSington->CorScheduleSchedWwait(co, fds[i].fd, fds[i].events, timeout);
	}
	CorSington->CoroutineYield(co); // 1

	for (i = 0; i < nfds; i++)
	{

		struct epoll_event ev;
		ev.events = PollEvent2Epoll(fds[i].events);
		ev.data.fd = fds[i].fd;
		epoll_ctl(sched->m_poll_fd, EPOLL_CTL_DEL, fds[i].fd, &ev);

		CorSchedSington->CorScheduleDeschedWait(fds[i].fd);
	}

	return nfds;
}

//
int CCoroutineSocket::SetNoBlock(uint32_t fd)
{
	u_long nonblock = 1;
	int ret = ioctlsocket(fd, FIONBIO, &nonblock);
	if (ret == SOCKET_ERROR)
	{
		closesocket(fd);
		return -1;
	}

	return 0;
}

//
int CCoroutineSocket::SetNoDelay(uint32_t fd)
{
	int nodelay = 1;
	int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
	if (ret == SOCKET_ERROR)
	{
		return -1;
	}

	return 0;
}