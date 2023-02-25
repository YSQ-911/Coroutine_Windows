#include "util.h"
#include <sstream>

//构造
CStrExplode::CStrExplode(char * str, char seperator)
{
	m_item_cnt = 1;
	char * pos = str;
	while (*pos) 
	{
		if (*pos == seperator) 
		{
			m_item_cnt++;
		}

		pos++;
	}

	m_item_list = new char *[m_item_cnt];

	int idx = 0;
	char * start = pos = str;
	while (*pos)
	{
		if (pos != start && *pos == seperator)
		{
			uint32_t len = pos - start;
			m_item_list[idx] = new char[len + 1];
			strncpy(m_item_list[idx], start, len);
			m_item_list[idx][len] = '\0';
			idx++;

			start = pos + 1;
		}

		pos++;
	}

	uint32_t len = (uint32_t)(pos - start);
	if (len != 0)
	{
		m_item_list[idx] = new char[len + 1];
		strncpy(m_item_list[idx], start, len);
		m_item_list[idx][len] = '\0';
	}
}

//析构
CStrExplode::~CStrExplode()
{
	for (uint32_t i = 0; i < m_item_cnt; i++) 
	{
		delete[] m_item_list[i];
	}

	delete[] m_item_list;
}

//获取毫秒级时间戳
uint64_t get_ms_tick_count(char * szTimer)
{
	uint64_t nTimer = 0;
	SYSTEMTIME currentTime;
	GetLocalTime(&currentTime);

	tm temptm = 
	{ 
		currentTime.wSecond, currentTime.wMinute, currentTime.wHour,
		currentTime.wDay, currentTime.wMonth - 1, currentTime.wYear - 1900
	};
	nTimer = mktime(&temptm) * 1000 + currentTime.wMilliseconds;

	if (szTimer != NULL)
	{
		sprintf(szTimer, "%llu", nTimer);
	}

	return nTimer;
}

//获取时间戳
uint64_t get_tick_count()
{
#ifdef _WIN32
	LARGE_INTEGER liCounter;
	LARGE_INTEGER liCurrent;

	if (!QueryPerformanceFrequency(&liCounter))
	{
		return GetTickCount();
	}

	QueryPerformanceCounter(&liCurrent);
	return (uint64_t)(liCurrent.QuadPart * 1000 / liCounter.QuadPart);
#else
	struct timeval tval;
	uint64_t ret_tick;

	gettimeofday(&tval, NULL);

	ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
	return ret_tick;
#endif
}

//获取当前时间
void getcurrentTime(char * timebuf)
{
	time_t timer = time(NULL);
	strftime(timebuf, 40, "[%Y-%m-%d %H:%M:%S]", localtime(&timer));

	int len = strlen(timebuf);
}

//获取时间
void gettimeofday(struct timeval * tp, void * tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;

	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wHour;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;

	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
}

//执行休眠
void util_sleep(uint32_t millisecond)
{
#ifdef _WIN32
	Sleep(millisecond);
#else
	usleep(millisecond * 1000);
#endif
}

/* get system time */
void itimeofday(long * sec, long * usec)
{
#if defined(__unix)
	struct timeval time;
	gettimeofday(&time, NULL);
	if (sec) *sec = time.tv_sec;
	if (usec) *usec = time.tv_usec;
#else
	static long mode = 0, addsec = 0;
	BOOL retval;
	static uint64_t freq = 1;
	uint64_t qpc;
	
	if (mode == 0)
	{
		retval = QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		freq = (freq == 0) ? 1 : freq;
		retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
		addsec = (long)time(NULL);
		addsec = addsec - (long)((qpc / freq) & 0x7fffffff);
		mode = 1;
	}

	retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
	retval = retval * 2;
	if (sec) *sec = (long)(qpc / freq) + addsec;
	if (usec) *usec = (long)((qpc % freq) * 1000000 / freq);
#endif
}

/* get clock in millisecond 64 */
uint64_t iclock64(void)
{
	long s, u;
	uint64_t value;
	itimeofday(&s, &u);
	value = ((uint64_t)s) * 1000 + (u / 1000);
	return value;
}

//
char * replaceStr(char * pSrc, char oldChar, char newChar)
{
	if (NULL == pSrc)
	{
		return NULL;
	}

	char * pHead = pSrc;
	while (*pHead != '\0')
	{
		if (*pHead == oldChar)
		{
			*pHead = newChar;
		}
		++pHead;
	}
	return pSrc;
}

//int 转 string
string int2string(uint32_t user_id)
{
	stringstream ss;
	ss << user_id;
	return ss.str();
}

//string 转 int
uint32_t string2int(const string & value)
{
	return (uint32_t)atoi(value.c_str());
}

//double 转 string
string double2string(double data)
{
	stringstream ss;
	ss << data;
	return ss.str();
}

//string 转 double
double string2double(const string & value)
{
	stringstream ss;
	double tmp;

	ss << value;
	ss >> tmp;

	return tmp;
}

// 由于被替换的内容可能包含?号，所以需要更新开始搜寻的位置信息来避免替换刚刚插入的?号
void replace_mark(string & str, string & new_value, uint32_t & begin_pos)
{
	string::size_type pos = str.find('?', begin_pos);
	if (pos == string::npos) 
	{
		return;
	}

	string prime_new_value = "'" + new_value + "'";
	str.replace(pos, 1, prime_new_value);

	begin_pos = (uint32_t)(pos + prime_new_value.size());
}

//
void replace_mark(string & str, uint32_t new_value, uint32_t & begin_pos)
{
	stringstream ss;
	ss << new_value;

	string str_value = ss.str();
	string::size_type pos = str.find('?', begin_pos);
	if (pos == string::npos) 
	{
		return;
	}

	str.replace(pos, 1, str_value);
	begin_pos = (uint32_t)(pos + str_value.size());
}

//写入当前进程ID
void writePid()
{
	uint32_t curPid;

#ifdef _WIN32
	curPid = (uint32_t)GetCurrentProcess();
#else
	curPid = (uint32_t)getpid();
#endif
	
	FILE * f = fopen("server.pid", "w");
	assert(f);
	char szPid[32];
	snprintf(szPid, sizeof(szPid), "%d", curPid);
	fwrite(szPid, strlen(szPid), 1, f);
	fclose(f);
}

//10进制转16进制
inline unsigned char toHex(const unsigned char &x)
{
	return x > 9 ? x - 10 + 'A' : x + '0';
}

//16进制转10进制
inline unsigned char fromHex(const unsigned char &x)
{
	return isdigit(x) ? x - '0' : x - 'A' + 10;
}

//URL编码
string URLEncode(const string & sIn)
{
	string sOut;
	for (size_t ix = 0; ix < sIn.size(); ix++)
	{
		unsigned char buf[4];
		memset(buf, 0, 4);
		if (isalnum((unsigned char)sIn[ix]))
		{
			buf[0] = sIn[ix];
		}
		else if (isspace( (unsigned char)sIn[ix] ) ) //貌似把空格编码成%20或者+都可以
		{
		    buf[0] = '+';
		}
		else
		{
			buf[0] = '%';
			buf[1] = toHex((unsigned char)sIn[ix] >> 4);
			buf[2] = toHex((unsigned char)sIn[ix] % 16);
		}
		sOut += (char *)buf;
	}
	return sOut;
}

//URL解码
string URLDecode(const string & sIn)
{
	string sOut;
	for (size_t ix = 0; ix < sIn.size(); ix++)
	{
		unsigned char ch = 0;
		if (sIn[ix] == '%')
		{
			ch = (fromHex(sIn[ix + 1]) << 4);
			ch |= fromHex(sIn[ix + 2]);
			ix += 2;
		}
		else if (sIn[ix] == '+')
		{
			ch = ' ';
		}
		else
		{
			ch = sIn[ix];
		}
		sOut += (char)ch;
	}
	return sOut;
}

//获取文件大小
int64_t get_file_size(const char * path)
{
	int64_t filesize = -1;
	struct stat statbuff;
	if (stat(path, &statbuff) < 0) 
	{
		return filesize;
	}
	else
	{
		filesize = statbuff.st_size;
	}
	return filesize;
}

//
const char * memfind(const char * src_str, size_t src_len, const char * sub_str, size_t sub_len, bool flag)
{
	if (NULL == src_str || NULL == sub_str || src_len <= 0)
	{
		return NULL;
	}

	if (src_len < sub_len)
	{
		return NULL;
	}

	const char * p;
	if (sub_len == 0)
	{
		sub_len = strlen(sub_str);
	}
	if (src_len == sub_len)
	{
		if (0 == (memcmp(src_str, sub_str, src_len)))
		{
			return src_str;
		}
		else
		{
			return NULL;
		}
	}

	if (flag)
	{
		for (int i = 0; i < src_len - sub_len; i++)
		{
			p = src_str + i;
			if (0 == memcmp(p, sub_str, sub_len))
			{
				return p;
			}
		}
	}
	else
	{
		for (int i = (uint32_t)(src_len - sub_len); i >= 0; i--)
		{
			p = src_str + i;
			if (0 == memcmp(p, sub_str, sub_len))
			{
				return p;
			}

		}
	}
	return NULL;
}

//生成hash值
uint64_t MurmurHash2_x64(const void * key, int len, uint32_t seed)
{
	const uint64_t m = 0xc6a4a7935bd1e995;
	const int r = 47;

	uint64_t h = seed ^ (len * m);

	const uint64_t * data = (const uint64_t *)key;
	const uint64_t * end = data + (len / 8);

	while (data != end)
	{
		uint64_t k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const uint8_t * data2 = (const uint8_t*)data;

	switch (len & 7)
	{
	case 7: h ^= ((uint64_t)data2[6]) << 48;
	case 6: h ^= ((uint64_t)data2[5]) << 40;
	case 5: h ^= ((uint64_t)data2[4]) << 32;
	case 4: h ^= ((uint64_t)data2[3]) << 24;
	case 3: h ^= ((uint64_t)data2[2]) << 16;
	case 2: h ^= ((uint64_t)data2[1]) << 8;
	case 1: h ^= ((uint64_t)data2[0]);
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

//获取随机conv值
uint32_t get_rand_conv(uint32_t socketfd)
{
	uint32_t tick = get_tick_count();
	uint32_t tick_left = tick % (uint32_t)(pow(10.00, 6));
	uint32_t conv = tick_left + socketfd * (uint32_t)(pow(10.00, 6));

	return conv;
}

//比较字符
static inline int judge_equeal(const char * pos, const char * compare, size_t & clen)
{
	for (size_t i = 0; i < clen; i++)
	{
		if (pos[i] != compare[i])
		{
			return -1;
		}
	}

	return 0;
}

//返回字符串位置
static inline char * string_find_pos(const char * u, const char * compare)
{
	size_t clen = strlen(compare);
	size_t ulen = strlen(u);

	if (clen > ulen)
	{
		return NULL;
	}

	const char * pos = u;
	const char * posend = u + ulen - 1;
	for (; pos <= posend - clen; pos++)
	{
		if (judge_equeal(pos, compare, clen) == 0)
		{
			return (char *)pos;
		}
	}

	return NULL;
}

//解析url参数
string get_url_param(const string & strUrl, const char * param)
{
	int point = -1;
	const char * ustart = strUrl.c_str();
	const char * start = string_find_pos(ustart, "?");

	if (start != NULL)
	{
		++start;

		string par = param;
		par += "=";
		start = string_find_pos(start, par.c_str());
		if (start != NULL)
		{
			const char * j = start - 1;
			char c = *j;
			if (c == '&' || c == '?')
			{
				start += par.length();
				const char * end = string_find_pos(start, "&");
				if (end != NULL)
				{
					return string(start, end);
				}

				return string(start);
			}
		}
	}

	return "";
}