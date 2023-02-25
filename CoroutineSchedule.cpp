#include "CoroutineSchedule.h"
#include "CoroutineEpoll.h"

#define CorSchedSington	(CCoroutineSchedule::getInstance())
#define CorSington		(CCoroutine::getInstance())
#define FD_KEY(f, e)	(((int64_t)(f) << (sizeof(int32_t) * 8)) | e)
#define FD_EVENT(f)		((int32_t)(f))
#define FD_ONLY(f)		((f) >> ((sizeof(int32_t) * 8)))

RB_GENERATE(_coroutine_rbtree_sleep, _coroutine, m_sleep_node, CorSchedSington->coroutine_sleep_cmp);
RB_GENERATE(_coroutine_rbtree_wait, _coroutine, m_wait_node, CorSchedSington->coroutine_wait_cmp);


//创建调度器
int CCoroutineSchedule::CoroutineScheduleCreate(int stack_size)
{
	//创建调度器
	int sched_stack_size = stack_size ? stack_size : COR_MAX_STACKSIZE;
	coroutine_schedule * sched = (coroutine_schedule *)calloc(1, sizeof(coroutine_schedule));
	if (sched == NULL)
	{
		printf("Failed to initialize scheduler\n");
		return -1;
	}

	//创建epoll
	assert(pthread_setspecific(global_sched_key, sched) == 0);
	sched->m_poll_fd = CCoroutineEpoll::getInstance()->CorEpollCreate();
	if (sched->m_poll_fd == NULL)
	{
		printf("Failed to initialize epoller\n");
		CoroutineScheduleFree(sched);
		return -2;
	}

	//初始化调度器参数
	sched->m_eventlist = (struct epoll_event *)malloc(COR_MAX_EVENTS);
	sched->m_stack_size = sched_stack_size;
	sched->m_page_size = PAGE_SIZE;
	sched->m_spawned_coroutines = 0;
	sched->m_default_timeout = 3000000u;
	sched->m_fiber = ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);

	//初始化休眠树、等待树
	RB_INIT(&sched->m_sleeping);
	RB_INIT(&sched->m_waiting);

	sched->m_birth = coroutine_usec_now();

	//初始化
	TAILQ_INIT(&sched->m_ready);
	LIST_INIT(&sched->m_busy);

	return 0;
}

//释放调度器
void CCoroutineSchedule::CoroutineScheduleFree(coroutine_schedule * sched)
{
	//关闭epoll
	if (sched->m_poll_fd > 0)
	{
		epoll_close(sched->m_poll_fd);
	}

	//关闭监听socket
	if (sched->m_eventfd > 0)
	{
		closesocket(sched->m_eventfd);
	}

	//释放调度器资源
	free(sched);
	assert(pthread_setspecific(global_sched_key, NULL) == 0);
}

//
void CCoroutineSchedule::CorSchedSleepDown(coroutine * co, uint64_t msecs)
{
	uint64_t usecs = msecs * 1000u;

	//查找协程
	coroutine * co_tmp = RB_FIND(_coroutine_rbtree_sleep, &co->m_sched->m_sleeping, co);
	if (co_tmp != NULL)
	{
		RB_REMOVE(_coroutine_rbtree_sleep, &co->m_sched->m_sleeping, co_tmp);
	}

	//
	co->m_sleep_usecs = coroutine_diff_usecs(co->m_sched->m_birth, coroutine_usec_now()) + usecs;

	while (msecs)
	{
		co_tmp = RB_INSERT(_coroutine_rbtree_sleep, &co->m_sched->m_sleeping, co);
		if (co_tmp)
		{
			//printf("1111 sleep_usecs %" PRIu64 "\n", co->m_sleep_usecs);
			co->m_sleep_usecs++;
			continue;
		}
		co->m_corStatus = (coroutine_status)(co->m_corStatus | BIT(NTY_COROUTINE_STATUS_SLEEPING));
		break;
	}
}

//
void CCoroutineSchedule::CorDeschedSleepDown(coroutine * co)
{
	if (co->m_corStatus & BIT(NTY_COROUTINE_STATUS_SLEEPING))
	{
		RB_REMOVE(_coroutine_rbtree_sleep, &co->m_sched->m_sleeping, co);

		co->m_corStatus = (coroutine_status)((co->m_corStatus) & CLEARBIT(NTY_COROUTINE_STATUS_SLEEPING));
		co->m_corStatus = (coroutine_status)((co->m_corStatus) | BIT(NTY_COROUTINE_STATUS_READY));
		co->m_corStatus = (coroutine_status)((co->m_corStatus) & CLEARBIT(NTY_COROUTINE_STATUS_EXPIRED));
	}
}

//
coroutine * CCoroutineSchedule::CorScheduleSearchWait(int fd)
{
	coroutine find_it = { 0 };
	find_it.m_fd = fd;

	coroutine_schedule * sched = nty_coroutine_get_sched();

	coroutine * co = RB_FIND(_coroutine_rbtree_wait, &sched->m_waiting, &find_it);
	co->m_corStatus = (coroutine_status)0;

	return co;
}

//
coroutine * CCoroutineSchedule::CorScheduleExpired(coroutine_schedule * sched)
{
	uint64_t t_diff_usecs = coroutine_diff_usecs(sched->m_birth, coroutine_usec_now());
	coroutine * co = RB_MIN(_coroutine_rbtree_sleep, &sched->m_sleeping);
	if (co == NULL)
	{
		return NULL;
	}

	if (co->m_sleep_usecs <= t_diff_usecs)
	{
		RB_REMOVE(_coroutine_rbtree_sleep, &co->m_sched->m_sleeping, co);
		return co;
	}
	return NULL;
}

//
int CCoroutineSchedule::CorScheduleIsDone(coroutine_schedule * sched)
{
	return (RB_EMPTY(&sched->m_waiting) && LIST_EMPTY(&sched->m_busy) && RB_EMPTY(&sched->m_sleeping) && TAILQ_EMPTY(&sched->m_ready));
}

//
uint64_t CCoroutineSchedule::CorScheduleMinTimeout(coroutine_schedule * sched)
{
	uint64_t t_diff_usecs = coroutine_diff_usecs(sched->m_birth, coroutine_usec_now());
	uint64_t min = sched->m_default_timeout;

	coroutine * co = RB_MIN(_coroutine_rbtree_sleep, &sched->m_sleeping);
	if (!co)
	{
		return min;
	}

	min = co->m_sleep_usecs;
	if (min > t_diff_usecs)
	{
		return min - t_diff_usecs;
	}

	return 0;
}

//调度器查询epoll socket事件
int CCoroutineSchedule::CorScheduleEpoll(coroutine_schedule * sched)
{
	sched->m_num_new_events = 0;

	struct timespec t = { 0, 0 };
	uint64_t usecs = CorScheduleMinTimeout(sched);
	if (usecs && TAILQ_EMPTY(&sched->m_ready))
	{
		t.tv_sec = usecs / 1000000u;
		if (t.tv_sec != 0)
		{
			t.tv_nsec = (usecs % 1000u) * 1000u;
		}
		else
		{
			t.tv_nsec = usecs * 1000u;
		}
	}
	else
	{
		return 0;
	}

	int nready = 0;
	while (1)
	{
		nready = CCoroutineEpoll::getInstance()->CorEpollWait(t);
		if (nready == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else
			{
				assert(0);
			}
		}
		break;
	}

	sched->m_nevents = 0;
	sched->m_num_new_events = nready;

	return 0;
}

//调度器运行
void CCoroutineSchedule::CorScheduleRun()
{
	//获取调度器
	coroutine_schedule * sched = nty_coroutine_get_sched();
	if (sched == NULL)
	{
		return;
	}

	//存在协程
	while (!CorScheduleIsDone(sched))
	{
		// 1. expired --> sleep rbtree
		coroutine * expired = NULL; //
		while ((expired = CorScheduleExpired(sched)) != NULL)
		{
			CorSington->CoroutineResume(expired);
		}

		// 2. ready queue //
		coroutine * last_co_ready = TAILQ_LAST(&sched->m_ready, _coroutine_queue);
		while (!TAILQ_EMPTY(&sched->m_ready))
		{
			coroutine *co = TAILQ_FIRST(&sched->m_ready);
			TAILQ_REMOVE(&co->m_sched->m_ready, co, m_ready_next);

			if (co->m_corStatus & BIT(NTY_COROUTINE_STATUS_FDEOF))
			{
				CorSington->CoroutineFree(co);
				break;
			}

			CorSington->CoroutineResume(co);
			if (co == last_co_ready)
			{
				break;
			}
		}

		// 3. wait rbtree
		CorScheduleEpoll(sched);
		while (sched->m_num_new_events)
		{
			int idx = --sched->m_num_new_events;
			struct epoll_event * ev = sched->m_eventlist + idx;

			int fd = ev->data.fd;
			int is_eof = ev->events & EPOLLHUP;
			if (is_eof)
			{
				errno = ECONNRESET;
			}

			coroutine * co = CorScheduleSearchWait(fd);
			if (co != NULL)
			{
				if (is_eof)
				{
					co->m_corStatus = (coroutine_status)((co->m_corStatus) | BIT(NTY_COROUTINE_STATUS_FDEOF));
				}
				CorSington->CoroutineResume(co);
			}

			is_eof = 0;
		}
	}

	CoroutineScheduleFree(sched);

	return;
}

//
coroutine * CCoroutineSchedule::CorScheduleDeschedWait(int fd)
{
	coroutine find_it = { 0 };
	find_it.m_fd = fd;

	coroutine_schedule * sched = nty_coroutine_get_sched();

	coroutine * co = RB_FIND(_coroutine_rbtree_wait, &sched->m_waiting, &find_it);
	if (co != NULL)
	{
		RB_REMOVE(_coroutine_rbtree_wait, &co->m_sched->m_waiting, co);
	}
	co->m_corStatus = (coroutine_status)0;
	CorDeschedSleepDown(co);

	return co;
}

//
void CCoroutineSchedule::CorScheduleSchedWwait(coroutine * co, int fd, unsigned short events, uint64_t timeout)
{
	if (co->m_corStatus & BIT(NTY_COROUTINE_STATUS_WAIT_READ) || co->m_corStatus & BIT(NTY_COROUTINE_STATUS_WAIT_WRITE))
	{
		//printf("Unexpected event. lt id %" PRIu64 " fd %" PRId32 " already in %" PRId32 " state\n", co->id, co->fd, co->status);
		assert(0);
	}

	if (events & POLLIN)
	{
		co->m_corStatus = (coroutine_status)((co->m_corStatus) | NTY_COROUTINE_STATUS_WAIT_READ);
	}
	else if (events & POLLOUT)
	{
		co->m_corStatus = (coroutine_status)((co->m_corStatus) | NTY_COROUTINE_STATUS_WAIT_WRITE);
	}
	else
	{
		printf("events : %d\n", events);
		assert(0);
	}

	co->m_fd = fd;
	co->m_events = events;
	coroutine * co_tmp = RB_INSERT(_coroutine_rbtree_wait, &co->m_sched->m_waiting, co);

	assert(co_tmp == NULL);

	// printf("timeout --> %"PRIu64"\n", timeout);
	if (timeout == 1)
	{
		return; // Error
	}

	CorSchedSleepDown(co, timeout);
}

//
int CCoroutineSchedule::coroutine_sleep_cmp(_coroutine * co1, _coroutine * co2)
{
	if (co1->m_sleep_usecs < co2->m_sleep_usecs)
	{
		return -1;
	}

	if (co1->m_sleep_usecs == co2->m_sleep_usecs)
	{
		return 0;
	}

	return 1;
}

//
int CCoroutineSchedule::coroutine_wait_cmp(_coroutine * co1, _coroutine * co2)
{
#if CANCEL_FD_WAIT_UINT64
	if (co1->m_fd < co2->m_fd)
	{
		return -1;
	}
	else if (co1->m_fd == co2->m_fd)
	{
		return 0;
	}
	else
	{
		return 1;
	}

#else

	if (co1->m_fd_wait < co2->m_fd_wait)
	{
		return -1;
	}

	if (co1->m_fd_wait == co2->m_fd_wait)
	{
		return 0;
	}

#endif
	return 1;
}