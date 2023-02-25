#ifndef _COROUTINE_SOCKET_H_
#define _COROUTINE_SOCKET_H_

#include "util.h"
#include "Coroutine.h"


//协程集成Socket
class COROUTINE_BASE CCoroutineSocket : public CSingleton<CCoroutineSocket>
{
	friend class CSingleton<CCoroutineSocket>;
public:
	//构造
	CCoroutineSocket() { }

	//析构
	virtual ~CCoroutineSocket() { }

	//创建socket
	int CorSocket(int domain, int type, int protocol);

	//接收客户端连接
	int CorAccept(int fd, struct sockaddr * addr, socklen_t * len);

	//数据接收
	int CorRecv(int fd, void * buf, size_t len, int flags);

	//数据发送
	int CorSend(int fd, const void * buf, size_t len, int flags);

	//udp数据接收
	int CorRecvfrom(int fd, void * buf, size_t len, int flags, struct sockaddr * src_addr, socklen_t * addrlen);

	//udp数据发送
	int CorSendto(int fd, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr, socklen_t addrlen);

	//关闭连接
	int CorClose(int fd);

private:
	//
	uint32_t PollEvent2Epoll(short events);

	//
	short PollEvent2Epoll(uint32_t events);

	//
	int PollInner(struct pollfd * fds, int nfds, int timeout);

	//
	int SetNoBlock(uint32_t fd);

	//
	int SetNoDelay(uint32_t fd);
};

#endif	//_COROUTINE_SOCKET_H_