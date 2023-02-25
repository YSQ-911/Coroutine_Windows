#ifndef _COROUTINE_BASE_H_
#define _COROUTINE_BASE_H_

#include <memory>
#include <functional>
#include <queue>
#include <unordered_map>
#include <Windows.h>
#include <cstdlib>
#include <ctime>
#include <assert.h>

#include "ostype.h"
#include "util.h"
#include "CoroutineQueue.h"
#include "CoroutineRbTree.h"


#define MAX_COROUTINE_NUM		32768			//max coroutine num
#define COR_MAX_EVENTS			(1 * 1024)		//epoll events
#define COR_MAX_STACKSIZE		(16 * 1024)		// {http: 16 * 1024, tcp: 4 * 1024}
#define BIT(x)					(1 << (x))		//
#define CLEARBIT(x)				~(1 << (x))		//
#define PAGE_SIZE				4096			//内存页大小
#define CANCEL_FD_WAIT_UINT64	1				//

//协程回调函数
typedef void(*proc_coroutine)(void *);

//
LIST_HEAD(_coroutine_link, _coroutine);
TAILQ_HEAD(_coroutine_queue, _coroutine);
RB_HEAD(_coroutine_rbtree_sleep, _coroutine);
RB_HEAD(_coroutine_rbtree_wait, _coroutine);

//
typedef struct _coroutine_link coroutine_link;
typedef struct _coroutine_queue coroutine_queue;
typedef struct _coroutine_rbtree_sleep coroutine_rbtree_sleep;
typedef struct _coroutine_rbtree_wait coroutine_rbtree_wait;

//协程执行状态
typedef enum COROUTINE_BASE _state
{
	NTY_COROUTINE_STATUS_WAIT_READ,
	NTY_COROUTINE_STATUS_WAIT_WRITE,
	NTY_COROUTINE_STATUS_NEW,
	NTY_COROUTINE_STATUS_READY,
	NTY_COROUTINE_STATUS_EXITED,
	NTY_COROUTINE_STATUS_BUSY,
	NTY_COROUTINE_STATUS_SLEEPING,
	NTY_COROUTINE_STATUS_EXPIRED,
	NTY_COROUTINE_STATUS_FDEOF,
	NTY_COROUTINE_STATUS_DETACH,
	NTY_COROUTINE_STATUS_CANCELLED,
	NTY_COROUTINE_STATUS_PENDING_RUNCOMPUTE,
	NTY_COROUTINE_STATUS_RUNCOMPUTE,
	NTY_COROUTINE_STATUS_WAIT_IO_READ,
	NTY_COROUTINE_STATUS_WAIT_IO_WRITE,
	NTY_COROUTINE_STATUS_WAIT_MULTI
}coroutine_status;

//协程调度器
typedef struct _coroutine_schedule
{
	uint64_t	m_birth;				//创建时间
	size_t		m_stack_size;			//栈大小
	int			m_spawned_coroutines;	//协程数量
	uint64_t	m_default_timeout;		//默认超时时间
	uint32_t	m_page_size;			//内存页大小
	int			m_num_new_events;		//新事件数量
	HANDLE		m_poll_fd;				//epoll fd
	int			m_eventfd;				//监听socket fd
	int			m_nevents;				//epoll事件
	PVOID		m_fiber;				//windows纤程

	struct _coroutine  *	m_current_thread;	//当前协程
	struct epoll_event *	m_eventlist;		//epoll eventslist

	coroutine_queue			m_ready;		//协程等待队列
	coroutine_queue			m_defer;		//协程删除队列
	coroutine_link			m_busy;			//协程繁忙链表
	coroutine_rbtree_sleep	m_sleeping;		//协程休眠树
	coroutine_rbtree_wait	m_waiting;		//协程等待树
}coroutine_schedule;


//协程
typedef struct _coroutine
{
	uint64_t				m_corId;			//协程id
	coroutine_status		m_corStatus;		//协程执行状态
	proc_coroutine			m_callback;			//协程回调函数
	void		*			m_arg;				//协程参数
	PVOID					m_fiber;			//纤程
	bool					m_corFlag;			//协程是否释放标记
	size_t					m_stack_size;		//协程栈大小
	uint64_t				m_birth;			//创建时间戳
	uint64_t				m_sleep_usecs;		//休眠时间
	coroutine_schedule	*	m_sched;			//协程调度器

#if CANCEL_FD_WAIT_UINT64
	int					m_fd;				//当前协程对应的客户端fd
	unsigned short		m_events;			//POLL_EVENT
#else
	int64_t				m_fd_wait;			//fd wait
#endif

	//
	RB_ENTRY(_coroutine)	m_sleep_node;	//协程休眠节点
	RB_ENTRY(_coroutine)	m_wait_node;	//协程等待节点
	LIST_ENTRY(_coroutine)	m_busy_next;	//繁忙节点
	TAILQ_ENTRY(_coroutine) m_ready_next;	//等待节点
	TAILQ_ENTRY(_coroutine) m_defer_next;	//删除节点
}coroutine;


//获取当前时间戳
static inline uint64_t coroutine_usec_now(void)
{
	LARGE_INTEGER liCounter;
	LARGE_INTEGER liCurrent;

	if (!QueryPerformanceFrequency(&liCounter))
	{
		return GetTickCount();
	}

	QueryPerformanceCounter(&liCurrent);
	return (uint64_t)(liCurrent.QuadPart * 1000 / liCounter.QuadPart);
}

//计算时间差
static inline uint64_t coroutine_diff_usecs(uint64_t t1, uint64_t t2) { return t2 - t1; }

//获取调度器
extern pthread_key_t global_sched_key;
static inline coroutine_schedule * nty_coroutine_get_sched(void) { return (coroutine_schedule *)pthread_getspecific(global_sched_key); }

#endif	//_COROUTINE_BASE_H_
