#ifndef _COROUTINE_SCHEDULE_H_
#define _COROUTINE_SCHEDULE_H_

#include "ostype.h"
#include "util.h"
#include "Coroutine.h"

//Э�̵�����
class COROUTINE_BASE CCoroutineSchedule : public CSingleton<CCoroutineSchedule>
{
	friend class CSingleton<CCoroutineSchedule>;
public:
	//����
	CCoroutineSchedule() { }

	//����
	virtual ~CCoroutineSchedule() { }

	//����������
	int CoroutineScheduleCreate(int stack_size);

	//�ͷŵ�����
	void CoroutineScheduleFree(coroutine_schedule * sched);

	//����������
	void CorScheduleRun();

	//
	coroutine * CorScheduleDeschedWait(int fd);

	//
	void CorScheduleSchedWwait(coroutine * co, int fd, unsigned short events, uint64_t timeout);

	//����ʱ����Ƚ�
	static inline int coroutine_sleep_cmp(_coroutine * co1, _coroutine * co2);

	//�ȴ�ʱ����Ƚ�
	static inline int coroutine_wait_cmp(_coroutine * co1, _coroutine * co2);

private:
	//
	void CorSchedSleepDown(coroutine * co, uint64_t msecs);

	//
	void CorDeschedSleepDown(coroutine * co);

	//
	coroutine * CorScheduleSearchWait(int fd);

	//�ͷŵ�����
	coroutine * CorScheduleExpired(coroutine_schedule * sched);

	//��ѯ���������ȴ�����׼�����������Ƿ����
	int CorScheduleIsDone(coroutine_schedule * sched);

	//Э����С��ʱʱ��
	uint64_t CorScheduleMinTimeout(coroutine_schedule * sched);

	//��������ѯepoll socket�¼�
	int CorScheduleEpoll(coroutine_schedule * sched);
};

#endif	//_COROUTINE_SCHEDULE_H_


