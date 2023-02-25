#ifndef _COROUTINE_SCHEDULE_H_
#define _COROUTINE_SCHEDULE_H_

#include "ostype.h"
#include "util.h"
#include "Coroutine.h"

//协程调度器
class COROUTINE_BASE CCoroutineSchedule : public CSingleton<CCoroutineSchedule>
{
	friend class CSingleton<CCoroutineSchedule>;
public:
	//构造
	CCoroutineSchedule() { }

	//析构
	virtual ~CCoroutineSchedule() { }

	//创建调度器
	int CoroutineScheduleCreate(int stack_size);

	//释放调度器
	void CoroutineScheduleFree(coroutine_schedule * sched);

	//调度器运行
	void CorScheduleRun();

	//
	coroutine * CorScheduleDeschedWait(int fd);

	//
	void CorScheduleSchedWwait(coroutine * co, int fd, unsigned short events, uint64_t timeout);

	//休眠时间戳比较
	static inline int coroutine_sleep_cmp(_coroutine * co1, _coroutine * co2);

	//等待时间戳比较
	static inline int coroutine_wait_cmp(_coroutine * co1, _coroutine * co2);

private:
	//
	void CorSchedSleepDown(coroutine * co, uint64_t msecs);

	//
	void CorDeschedSleepDown(coroutine * co);

	//
	coroutine * CorScheduleSearchWait(int fd);

	//释放调度器
	coroutine * CorScheduleExpired(coroutine_schedule * sched);

	//查询休眠树、等待树、准备树中数据是否存在
	int CorScheduleIsDone(coroutine_schedule * sched);

	//协程最小超时时间
	uint64_t CorScheduleMinTimeout(coroutine_schedule * sched);

	//调度器查询epoll socket事件
	int CorScheduleEpoll(coroutine_schedule * sched);
};

#endif	//_COROUTINE_SCHEDULE_H_


