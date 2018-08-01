#ifndef _ASYNC_PROC_PROXY_H_
#define _ASYNC_PROC_PROXY_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "Utils.h"
#include "WorkerThread.h"

#include <queue>


using namespace std;


namespace ImmSocketService {

template <typename T>
class CAsyncProcProxy;

template <typename T>
class IAsyncHandler;


template <typename T>
struct ST_ASYNC_QUEUE {
public:
	ST_ASYNC_QUEUE (void)
		:mpAsyncHandler (NULL)
		,isUsed (false)
	{}

	explicit ST_ASYNC_QUEUE (T arg)
		:mpAsyncHandler (NULL)
		,isUsed (true)
	{
		msg = arg;
	}

	ST_ASYNC_QUEUE (T arg, IAsyncHandler<T> *pHandler)
		:mpAsyncHandler (NULL)
		,isUsed (true)
	{
		msg = arg;
		mpAsyncHandler = pHandler;
	}

	ST_ASYNC_QUEUE (const ST_ASYNC_QUEUE& obj)
		:msg (obj.msg)
		,mpAsyncHandler (obj.mpAsyncHandler)
		,isUsed (obj.isUsed)
	{}

	~ST_ASYNC_QUEUE (void) {}


	ST_ASYNC_QUEUE &operator=(const ST_ASYNC_QUEUE &obj)
	{
		msg = obj.msg;
		mpAsyncHandler = obj.mpAsyncHandler;
		isUsed = obj.isUsed;
		return *this;
	}

	T msg;
	IAsyncHandler<T> *mpAsyncHandler;
	bool isUsed;
};

template <typename T>
class IAsyncHandler
{
public:
	IAsyncHandler (void)
		:mIsDeletable (false)
		,mIsDone (false)
	{}
	virtual ~IAsyncHandler (void) {}

	virtual void onAsyncHandled (T arg) = 0;

	bool isDeletable (void) {
		return mIsDeletable;
	}
	void deletable (void) {
		mIsDeletable = true;
	}

	bool isDone (void) {
		return mIsDone;
	}
	void done (void) {
		mIsDone = true;
	}

private:
	bool mIsDeletable;
	bool mIsDone;
};

template <typename T>
class CProxyThread : public CWorkerThread
{
public:
	CProxyThread (void);
	CProxyThread (CAsyncProcProxy<T> *pProxy);
	virtual ~CProxyThread (void);

	bool start (void); // async
	void stop (void); // async
	void syncStop (void);

	void setAsyncProcProxy (CAsyncProcProxy<T> *pAsyncProcProxy);

private:
	void onThreadMainRoutine (void);


	bool mIsStop;
	CAsyncProcProxy<T> *mpAsyncProcProxy;

	pthread_mutex_t mMutex;
};

template <typename T>
class CAsyncProcProxy
{
public:
	friend class CProxyThread<T>;

	explicit CAsyncProcProxy (int nThreadPoolNum=1);
	CAsyncProcProxy (IAsyncHandler<T> *pHandler, int nThreadPoolNum=1);
	virtual ~CAsyncProcProxy (void);

	bool start (void);
	void stop (void);
	void syncStop (void);

	void request (T arg);
	void request (T arg, IAsyncHandler<T> *pHandler);


private:
	void init (IAsyncHandler<T> *pHandler, int nThreadPoolNum);
	void finaliz (void);
	void enQueue (ST_ASYNC_QUEUE<T> *eq);
	ST_ASYNC_QUEUE<T> deQueue (bool isPeep=false); // friend access


	int mThreadPoolNum;

	CProxyThread<T> *mpThreadPool; // thread pool start address
	IAsyncHandler<T> *mpAsyncHandler; // friend access

	queue < ST_ASYNC_QUEUE<T> > mQueue;
	pthread_mutex_t mMutexQueue;

	pthread_mutex_t mMutexCond; // friend access
	pthread_cond_t mCondMulti;  // friend access
};

} // namespace ImmSocketService

#include "AsyncProcProxyImpl.h"

#endif
