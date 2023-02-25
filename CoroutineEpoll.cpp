#include "CoroutineEpoll.h"
#include "CoroutineSchedule.h"


//创建epoll
HANDLE CCoroutineEpoll::CorEpollCreate()
{
	return epoll_create(1024);
}

//epoll事件等待
int CCoroutineEpoll::CorEpollWait(struct timespec t)
{
	coroutine_schedule * sched = nty_coroutine_get_sched();
	return epoll_wait(sched->m_poll_fd, sched->m_eventlist, COR_MAX_EVENTS, t.tv_sec * 1000.0 + t.tv_nsec / 1000000.0);
}

//
int CCoroutineEpoll::CorEpollRegisterTrigger(int socketfd)
{
	coroutine_schedule * sched = nty_coroutine_get_sched();

	if (!sched->m_eventfd)
	{
		sched->m_eventfd = socketfd;
		assert(sched->m_eventfd != -1);
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sched->m_eventfd;
	int ret = epoll_ctl(sched->m_poll_fd, EPOLL_CTL_ADD, sched->m_eventfd, &ev);

	//assert(ret != -1);

	return 0;
}