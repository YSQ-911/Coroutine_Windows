#ifndef _COROUTINE_EPOLL_H_
#define _COROUTINE_EPOLL_H_

#include "ostype.h"
#include "util.h"
#include "wepoll.h"



//Э����epoll���
class COROUTINE_BASE CCoroutineEpoll : public CSingleton<CCoroutineEpoll>
{
	friend class CSingleton<CCoroutineEpoll>;
public:
	//����
	CCoroutineEpoll() { }

	//����
	virtual ~CCoroutineEpoll() { }

	//����epoll
	HANDLE CorEpollCreate();

	//epoll�¼��ȴ�
	int CorEpollWait(struct timespec t);

	//
	int CorEpollRegisterTrigger(int socketfd);

};

#endif	//_COROUTINE_EPOLL_H_