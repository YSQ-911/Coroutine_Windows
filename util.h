#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdarg.h>

#include "ostype.h"
#include "SingleTon.h"

#ifdef _MSC_VER
#define strcasecmp	_stricmp
#define strncasecmp _strnicmp
#endif

#define nil 0

using namespace std;

#define _CRT_SECURE_NO_DEPRECATE					// remove warning C4996, 
#define NOTUSED_ARG(v)					((void)v)	// used this to remove warning C4100, unreferenced parameter
#define EXIT_CODE						(-1)		// 传递给Worker线程的退出信号
#define MAX_POST_ACCEPT					(10)		// 同时投递的Accept数量
#define WORKER_THREADS_PER_PROCESSOR	(2)			// 每个处理器上的线程数（线程池公式  核心数 * 2）
#define BUFF_SIZE						(1024 * 4)	// 一页为1K，缓冲区总共为4K
#define BUFSIZE							1024		// max size of incoming data buffer

#define SERVER_HEARTBEAT_INTERVAL	15000
#define SERVER_TIMEOUT				30000
#define CLIENT_HEARTBEAT_INTERVAL	30000
#define CLIENT_TIMEOUT				120000
#define READ_BUF_SIZE				4096

#define	snprintf	sprintf_s
#define printf		printf_s

#define HTTP_CONN_TIMEOUT				30000					//超时时间

//三种信号
#define SIGUSR1 0x01		
#define SIGUSR2 0x02
#define SIGHUP	0x03

//#if _MSC_VER == 1600
#ifndef _CRT_NO_TIME_T
struct timespec
{
	time_t tv_sec;  // Seconds - >= 0
	long   tv_nsec; // Nanoseconds - [0, 999999999]
};
#endif
//#endif

//字符串分割
class COROUTINE_BASE CStrExplode
{
public:
	//构造
	CStrExplode(char * str, char seperator);
	
	//析构
	virtual ~CStrExplode();

	//获取分隔数
	uint32_t GetItemCnt() { return m_item_cnt; }
	
	//获取分割界限间的字符
	char * GetItem(uint32_t idx) { return m_item_list[idx]; }

private:
	uint32_t	m_item_cnt;		//分割数
	char **		m_item_list;	//字符列表
};


COROUTINE_BASE inline uint64_t get_ms_tick_count(char * szTimer = NULL);
COROUTINE_BASE inline uint64_t get_tick_count();
COROUTINE_BASE inline void getcurrentTime(char * timebuf);
COROUTINE_BASE inline void gettimeofday(struct timeval * tp, void * tzp);
COROUTINE_BASE inline void util_sleep(uint32_t millisecond);
COROUTINE_BASE inline void itimeofday(long *sec, long *usec);
COROUTINE_BASE inline uint64_t iclock64(void);
COROUTINE_BASE inline char * replaceStr(char * pSrc, char oldChar, char newChar);
COROUTINE_BASE inline string int2string(uint32_t user_id);
COROUTINE_BASE inline uint32_t string2int(const string & value);
COROUTINE_BASE inline string double2string(double data);
COROUTINE_BASE inline double string2double(const string & value);
COROUTINE_BASE inline void replace_mark(string & str, string & new_value, uint32_t & begin_pos);
COROUTINE_BASE inline unsigned char toHex(const unsigned char & x);
COROUTINE_BASE inline unsigned char fromHex(const unsigned char & x);
COROUTINE_BASE inline string URLEncode(const string & sIn);
COROUTINE_BASE inline string URLDecode(const string & sIn);
COROUTINE_BASE inline int64_t get_file_size(const char * path);
COROUTINE_BASE const inline char * memfind(const char * src_str, size_t src_len, const char * sub_str, size_t sub_len, bool flag = true);
COROUTINE_BASE inline void writePid();
COROUTINE_BASE inline uint64_t MurmurHash2_x64(const void * key, int len, uint32_t seed);
COROUTINE_BASE inline uint32_t get_rand_conv(uint32_t socketfd);
COROUTINE_BASE inline string get_url_param(const string & strUrl, const char * param);

#endif	//__UTIL_H__
