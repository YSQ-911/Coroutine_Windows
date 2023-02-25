#ifndef _COROUTINE_EPOLL_H_
#define _COROUTINE_EPOLL_H_

#include "ostype.h"
#include "util.h"
#include "wepoll.h"



//协程与epoll结合
class COROUTINE_BASE CCoroutineEpoll : public CSingleton<CCoroutineEpoll>
{
	friend class CSingleton<CCoroutineEpoll>;
public:
	//构造
	CCoroutineEpoll() { }

	//析构
	virtual ~CCoroutineEpoll() { }

	//创建epoll
	HANDLE CorEpollCreate();

	//epoll事件等待
	int CorEpollWait(struct timespec t);

	//
	int CorEpollRegisterTrigger(int socketfd);

};

#endif	//_COROUTINE_EPOLL_H_