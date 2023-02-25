#ifndef _SINGLE_TON_H_
#define _SINGLE_TON_H_

//µ¥Àý»ùÀà
template<typename T>
class CSingleton 
{
public:
	static T & Instance() 
	{
		if (CSingleton::s_instance == 0)
		{
			CSingleton::s_instance = CreateInstance();
		}
		return *(CSingleton::s_instance);
	}

	static T * GetInstance() 
	{
		if (CSingleton::s_instance == 0) 
		{
			CSingleton::s_instance = CreateInstance();
		}
		return CSingleton::s_instance;
	}

	static T * getInstance() 
	{
		if (CSingleton::s_instance == 0) 
		{
			CSingleton::s_instance = CreateInstance();
		}
		return CSingleton::s_instance;
	}

	static void Destroy() 
	{
		if (CSingleton::s_instance != 0)
		{
			DestroyInstance(CSingleton::s_instance);
			CSingleton::s_instance = 0;
		}
	}

protected:
	CSingleton()
	{
		CSingleton::s_instance = static_cast<T *>(this);
	}

	~CSingleton()
	{
		CSingleton::s_instance = 0;
	}

private:
	static T * CreateInstance() 
	{
		return new T();
	}

	static void DestroyInstance(T * p) 
	{
		delete p;
	}

private:
	static T * s_instance;

private:
	explicit CSingleton(CSingleton const &) { }
	CSingleton & operator=(CSingleton const &) { return *this; }
};

template<typename T>
T * CSingleton<T>::s_instance = 0;

#endif	//_SINGLETON_H_