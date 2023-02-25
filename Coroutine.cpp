#include "Coroutine.h"
#include "CoroutineSchedule.h"

//
#define CorSchedSington	(CCoroutineSchedule::getInstance())

pthread_key_t global_sched_key;
static pthread_once_t sched_key_once = PTHREAD_ONCE_INIT;


//创建协程，并设置参数
int CCoroutine::CoroutineCreate(coroutine ** new_cor, proc_coroutine func, void * arg)
{
	//创建唯一调度器
	assert(pthread_once(&sched_key_once, coroutine_sched_key_creator) == 0);
	coroutine_schedule * sched = nty_coroutine_get_sched();
	if (sched == NULL)
	{
		CorSchedSington->CoroutineScheduleCreate(0);

		sched = nty_coroutine_get_sched();
		if (sched == NULL)
		{
			printf("Failed to create scheduler\n");
			return -1;
		}
	}

	//创建协程
	coroutine * co = (coroutine *)calloc(1, sizeof(coroutine));
	if (co == NULL)
	{
		printf("Failed to allocate memory for new coroutine\n");
		return -2;
	}

	//设置新建协程参数
	co->m_corId = sched->m_spawned_coroutines++;
	co->m_sched = sched;
	co->m_stack_size = sched->m_stack_size;
	co->m_corStatus = (coroutine_status)BIT(NTY_COROUTINE_STATUS_NEW);
	co->m_callback = func;
#if CANCEL_FD_WAIT_UINT64
	co->m_fd = -1;
	co->m_events = 0;
#else
	co->m_coroutine->m_fd_wait = -1;
#endif
	co->m_arg = arg;
	co->m_birth = coroutine_usec_now();
	co->m_fiber = CreateFiberEx(co->m_stack_size, 0, FIBER_FLAG_FLOAT_SWITCH, coroutine_exec, co);
	*new_cor = co;

	TAILQ_INSERT_TAIL(&sched->m_ready, co, m_ready_next);

	return 0;
}

//释放协程
void CCoroutine::CoroutineFree(coroutine * cor)
{
	if (cor == NULL)
	{
		return;
	}

	//删除windows纤程
	cor->m_sched->m_spawned_coroutines--;
	DeleteFiber(cor->m_fiber);

	if (cor)
	{
		free(cor);
		cor = NULL;
	}
}

//协程让出cpu
int CCoroutine::CoroutineYield(coroutine * cor)
{
	//切换到调度器纤程
	SwitchToFiber(cor->m_sched->m_fiber);

	return 0;
}

//恢复协程运行权
int CCoroutine::CoroutineResume(coroutine * cor)
{
	assert(cor != NULL);
	if (cor->m_corStatus & BIT(NTY_COROUTINE_STATUS_NEW))
	{
		//nty_coroutine_init(cor);
	}

	//
	coroutine_schedule * sched = nty_coroutine_get_sched();
	sched->m_current_thread = cor;
	SwitchToFiber(cor->m_fiber);
	sched->m_current_thread = NULL;

	if (cor->m_corStatus & BIT(NTY_COROUTINE_STATUS_EXITED))
	{
		if (cor->m_corStatus & BIT(NTY_COROUTINE_STATUS_DETACH))
		{
			CoroutineFree(cor);
		}
		return -1;
	}

	return 0;
}

//初始化windows纤程
void CCoroutine::coroutine_init(coroutine * co)
{
	co->m_corStatus = (coroutine_status)BIT(NTY_COROUTINE_STATUS_READY);
}

//协程回调函数
void CALLBACK CCoroutine::coroutine_exec(LPVOID lpFiberParameter)
{
	coroutine * Cor = (coroutine *)lpFiberParameter;
	if (Cor)
	{
		Cor->m_callback(Cor->m_arg);
		Cor->m_corStatus = (coroutine_status)((BIT(NTY_COROUTINE_STATUS_EXITED) | BIT(NTY_COROUTINE_STATUS_FDEOF) | BIT(NTY_COROUTINE_STATUS_DETACH)));
		SwitchToFiber(Cor->m_sched->m_fiber);
	}
}

//销毁全局唯一调度器值
void CCoroutine::coroutine_sched_key_destructor(void * data)
{
	free(data);
}

//创建全局唯一调度器值
void CCoroutine::coroutine_sched_key_creator(void)
{
	assert(pthread_key_create(&global_sched_key, coroutine_sched_key_destructor) == 0);
	assert(pthread_setspecific(global_sched_key, NULL) == 0);

	return;
}