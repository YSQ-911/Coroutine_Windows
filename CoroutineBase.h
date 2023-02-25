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
#define PAGE_SIZE				4096			//�ڴ�ҳ��С
#define CANCEL_FD_WAIT_UINT64	1				//

//Э�̻ص�����
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

//Э��ִ��״̬
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

//Э�̵�����
typedef struct _coroutine_schedule
{
	uint64_t	m_birth;				//����ʱ��
	size_t		m_stack_size;			//ջ��С
	int			m_spawned_coroutines;	//Э������
	uint64_t	m_default_timeout;		//Ĭ�ϳ�ʱʱ��
	uint32_t	m_page_size;			//�ڴ�ҳ��С
	int			m_num_new_events;		//���¼�����
	HANDLE		m_poll_fd;				//epoll fd
	int			m_eventfd;				//����socket fd
	int			m_nevents;				//epoll�¼�
	PVOID		m_fiber;				//windows�˳�

	struct _coroutine  *	m_current_thread;	//��ǰЭ��
	struct epoll_event *	m_eventlist;		//epoll eventslist

	coroutine_queue			m_ready;		//Э�̵ȴ�����
	coroutine_queue			m_defer;		//Э��ɾ������
	coroutine_link			m_busy;			//Э�̷�æ����
	coroutine_rbtree_sleep	m_sleeping;		//Э��������
	coroutine_rbtree_wait	m_waiting;		//Э�̵ȴ���
}coroutine_schedule;


//Э��
typedef struct _coroutine
{
	uint64_t				m_corId;			//Э��id
	coroutine_status		m_corStatus;		//Э��ִ��״̬
	proc_coroutine			m_callback;			//Э�̻ص�����
	void		*			m_arg;				//Э�̲���
	PVOID					m_fiber;			//�˳�
	bool					m_corFlag;			//Э���Ƿ��ͷű��
	size_t					m_stack_size;		//Э��ջ��С
	uint64_t				m_birth;			//����ʱ���
	uint64_t				m_sleep_usecs;		//����ʱ��
	coroutine_schedule	*	m_sched;			//Э�̵�����

#if CANCEL_FD_WAIT_UINT64
	int					m_fd;				//��ǰЭ�̶�Ӧ�Ŀͻ���fd
	unsigned short		m_events;			//POLL_EVENT
#else
	int64_t				m_fd_wait;			//fd wait
#endif

	//
	RB_ENTRY(_coroutine)	m_sleep_node;	//Э�����߽ڵ�
	RB_ENTRY(_coroutine)	m_wait_node;	//Э�̵ȴ��ڵ�
	LIST_ENTRY(_coroutine)	m_busy_next;	//��æ�ڵ�
	TAILQ_ENTRY(_coroutine) m_ready_next;	//�ȴ��ڵ�
	TAILQ_ENTRY(_coroutine) m_defer_next;	//ɾ���ڵ�
}coroutine;


//��ȡ��ǰʱ���
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

//����ʱ���
static inline uint64_t coroutine_diff_usecs(uint64_t t1, uint64_t t2) { return t2 - t1; }

//��ȡ������
extern pthread_key_t global_sched_key;
static inline coroutine_schedule * nty_coroutine_get_sched(void) { return (coroutine_schedule *)pthread_getspecific(global_sched_key); }

#endif	//_COROUTINE_BASE_H_
