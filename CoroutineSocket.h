#ifndef _COROUTINE_SOCKET_H_
#define _COROUTINE_SOCKET_H_

#include "util.h"
#include "Coroutine.h"


//Э�̼���Socket
class COROUTINE_BASE CCoroutineSocket : public CSingleton<CCoroutineSocket>
{
	friend class CSingleton<CCoroutineSocket>;
public:
	//����
	CCoroutineSocket() { }

	//����
	virtual ~CCoroutineSocket() { }

	//����socket
	int CorSocket(int domain, int type, int protocol);

	//���տͻ�������
	int CorAccept(int fd, struct sockaddr * addr, socklen_t * len);

	//���ݽ���
	int CorRecv(int fd, void * buf, size_t len, int flags);

	//���ݷ���
	int CorSend(int fd, const void * buf, size_t len, int flags);

	//udp���ݽ���
	int CorRecvfrom(int fd, void * buf, size_t len, int flags, struct sockaddr * src_addr, socklen_t * addrlen);

	//udp���ݷ���
	int CorSendto(int fd, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr, socklen_t addrlen);

	//�ر�����
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