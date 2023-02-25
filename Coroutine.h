/**
 * @Author: YSQ
 * @Date:   2023-02-24 21:54:06
 * @Last Modified by:   YSQ
 * @Last Modified time: 2023-02-25 21:36:03
 */
#ifndef _COROUTINE_H_
#define _COROUTINE_H_

#include "ostype.h"
#include "CoroutineBase.h"



//利用Windows自带纤程实现协程
class COROUTINE_BASE CCoroutine : public CSingleton<CCoroutine>
{
	friend class CSingleton<CCoroutine>;
public:
	//构造
	CCoroutine() { }

	//析构
	virtual ~CCoroutine() { }

	//创建协程，并设置参数
	inline int CoroutineCreate(coroutine ** new_cor, proc_coroutine func, void * arg);

	//释放协程
	inline void CoroutineFree(coroutine * cor);
	
	//协程让出cpu
	inline int CoroutineYield(coroutine * cor);

	//恢复协程运行权
	inline int CoroutineResume(coroutine * cor);

private:
	//初始化协程参数
	static void coroutine_init(coroutine * co);

	//协程回调函数
	static void CALLBACK coroutine_exec(LPVOID lpFiberParameter);

	//销毁全局唯一调度器值
	static void coroutine_sched_key_destructor(void * data);

	//创建全局唯一调度器值
	static void coroutine_sched_key_creator(void);
};

#endif	//_COROUTINE_H_

