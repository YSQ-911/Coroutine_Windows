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



//����Windows�Դ��˳�ʵ��Э��
class COROUTINE_BASE CCoroutine : public CSingleton<CCoroutine>
{
	friend class CSingleton<CCoroutine>;
public:
	//����
	CCoroutine() { }

	//����
	virtual ~CCoroutine() { }

	//����Э�̣������ò���
	inline int CoroutineCreate(coroutine ** new_cor, proc_coroutine func, void * arg);

	//�ͷ�Э��
	inline void CoroutineFree(coroutine * cor);
	
	//Э���ó�cpu
	inline int CoroutineYield(coroutine * cor);

	//�ָ�Э������Ȩ
	inline int CoroutineResume(coroutine * cor);

private:
	//��ʼ��Э�̲���
	static void coroutine_init(coroutine * co);

	//Э�̻ص�����
	static void CALLBACK coroutine_exec(LPVOID lpFiberParameter);

	//����ȫ��Ψһ������ֵ
	static void coroutine_sched_key_destructor(void * data);

	//����ȫ��Ψһ������ֵ
	static void coroutine_sched_key_creator(void);
};

#endif	//_COROUTINE_H_

