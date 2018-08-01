#ifndef _ASYNC_PROC_PROXY_IMPL_H_
#define _ASYNC_PROC_PROXY_IMPL_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ImmSocketServiceCommon.h"
#include "AsyncProcProxy.h"


namespace ImmSocketService {

template <typename T>
CProxyThread<T>::CProxyThread (void)
	:mIsStop (false)
	,mpAsyncProcProxy (NULL)
{
	pthread_mutex_init (&mMutex, NULL);
}

template <typename T>
CProxyThread<T>::CProxyThread (CAsyncProcProxy<T> *pProxy)
	:mIsStop (false)
	,mpAsyncProcProxy (NULL)
{
	if (pProxy) {
		mpAsyncProcProxy = pProxy;
	}

	pthread_mutex_init (&mMutex, NULL);
}

template <typename T>
CProxyThread<T>::~CProxyThread (void)
{
	pthread_mutex_destroy (&mMutex);
}

template <typename T>
bool CProxyThread<T>::start (void)
{
	CUtils::CScopedMutex scopedMutex (&mMutex);

	if (isAlive()) {
		_ISS_LOG_W ("already started\n");
		return true;
	}

	mIsStop = false;
	return create ();
}

template <typename T>
void CProxyThread<T>::stop (void)
{
	CUtils::CScopedMutex scopedMutex (&mMutex);

	mIsStop = true;
}

template <typename T>
void CProxyThread<T>::syncStop (void)
{
	CUtils::CScopedMutex scopedMutex (&mMutex);

	mIsStop = true;

	waitDestroy ();
}

template <typename T>
void CProxyThread<T>::setAsyncProcProxy (CAsyncProcProxy<T> *pAsyncProcProxy)
{
	if (pAsyncProcProxy) {
		mpAsyncProcProxy = pAsyncProcProxy;
	}
}

template <typename T>
void CProxyThread<T>::onThreadMainRoutine (void)
{
	char szName [64] = {0};
	snprintf (szName, sizeof(szName), "%s(%ld)", "ProxyTh", syscall(SYS_gettid));
	setName (szName);
	_ISS_LOG_I ("%s %s\n", __FILE__, __func__);


	if (!mpAsyncProcProxy) {
		_ISS_LOG_E ("mpAsyncProcProxy is null\n");
		return;
	}

	pthread_mutex_t *pMutexCond = &(mpAsyncProcProxy->mMutexCond);
	pthread_cond_t *pCond = &(mpAsyncProcProxy->mCondMulti);

	int rtn = 0;
	struct timespec stTimeout = {0};
	struct timeval stNowTimeval = {0};

	while (1) {
		// lock
		pthread_mutex_lock (pMutexCond);

		ST_ASYNC_QUEUE<T> q = mpAsyncProcProxy->deQueue();
		if (!q.isUsed) {

			memset (&stTimeout, 0x00, sizeof(stTimeout));
			memset (&stNowTimeval, 0x00, sizeof(stNowTimeval));
			CUtils::getTimeOfDay (&stNowTimeval);
			stTimeout.tv_sec = stNowTimeval.tv_sec + 1;
			stTimeout.tv_nsec = stNowTimeval.tv_usec * 1000;

			rtn = pthread_cond_timedwait (pCond, pMutexCond, &stTimeout);

			// unlock
			pthread_mutex_unlock (pMutexCond);

			if (rtn == ETIMEDOUT) {
			// timeout
				if (mIsStop) {
					_ISS_LOG_W ("stop --> waitloop break\n");
					break;
				}
			}

		} else {

			// unlock
			pthread_mutex_unlock (pMutexCond);

			if (q.mpAsyncHandler) {

				q.mpAsyncHandler->onAsyncHandled (q.msg);

				q.mpAsyncHandler->done();

				if (q.mpAsyncHandler->isDeletable()) {
					delete q.mpAsyncHandler;
				}

			} else {
				if (mpAsyncProcProxy->mpAsyncHandler) {

					mpAsyncProcProxy->mpAsyncHandler->onAsyncHandled (q.msg);

					//TODO
					//mpAsyncProcProxy->mpAsyncHandler->done();

					//TODO
					//if (mpAsyncProcProxy->mpAsyncHandler->isDeletable()) {
					//	delete mpAsyncProcProxy->mpAsyncHandler;
					//}
				}
			}
		}
	}


	_ISS_LOG_I ("%s %s end...\n", __FILE__, __func__);

	// thread end
}

template <typename T>
CAsyncProcProxy<T> ::CAsyncProcProxy (int nThreadPoolNum)
	:mThreadPoolNum (1)
	,mpThreadPool (NULL)
	,mpAsyncHandler (NULL)
{
	init (NULL, nThreadPoolNum);
}

template <typename T>
CAsyncProcProxy<T> ::CAsyncProcProxy (IAsyncHandler<T> *pHandler, int nThreadPoolNum)
	:mThreadPoolNum (1)
	,mpThreadPool (NULL)
	,mpAsyncHandler (NULL)
{
	init (pHandler, nThreadPoolNum);
}

template <typename T>
CAsyncProcProxy<T> ::~CAsyncProcProxy (void)
{
	finaliz ();
}

template <typename T>
void CAsyncProcProxy<T> ::init (IAsyncHandler<T> *pHandler, int nThreadPoolNum)
{
	if (nThreadPoolNum < 1) {
		nThreadPoolNum = 1;
	}
	mThreadPoolNum = nThreadPoolNum;

	mpThreadPool = new CProxyThread<T> [mThreadPoolNum];

	if (pHandler) {
		mpAsyncHandler = pHandler;
	}

	pthread_mutex_init (&mMutexQueue, NULL);
	pthread_mutex_init (&mMutexCond, NULL);
	pthread_cond_init (&mCondMulti, NULL);
}

template <typename T>
void CAsyncProcProxy<T> ::finaliz (void)
{
	if (mpThreadPool) {
		delete [] mpThreadPool;
		mpThreadPool = NULL;
	}

	pthread_mutex_destroy (&mMutexQueue);
	pthread_mutex_destroy (&mMutexCond);
	pthread_cond_destroy (&mCondMulti);
}

template <typename T>
bool CAsyncProcProxy<T> ::start (void)
{
	bool rtn = true;

	for (int i = 0; i < mThreadPoolNum; ++ i) {

		(mpThreadPool + i)->setAsyncProcProxy (this);

		if (!((mpThreadPool + i)->start())) {
			_ISS_LOG_E ("mpThreadPool %d ->start() is failure\n", i);
		}
	}

	return rtn;
}

template <typename T>
void CAsyncProcProxy<T> ::stop (void)
{
	for (int i = 0; i < mThreadPoolNum; ++ i) {
		(mpThreadPool + i)->stop();
	}
}

template <typename T>
void CAsyncProcProxy<T> ::syncStop (void)
{
	for (int i = 0; i < mThreadPoolNum; ++ i) {
		(mpThreadPool + i)->syncStop();
	}
}

template <typename T>
void CAsyncProcProxy<T> ::request (T arg)
{
	CUtils::CScopedMutex scopedMutex (&mMutexCond);

	ST_ASYNC_QUEUE<T> eq (arg);
	enQueue (&eq);
	pthread_cond_broadcast (&mCondMulti); // thread pool start
}

template <typename T>
void CAsyncProcProxy<T> ::request (T arg, IAsyncHandler<T> *pHandler)
{
	CUtils::CScopedMutex scopedMutex (&mMutexCond);

	ST_ASYNC_QUEUE<T> eq (arg, pHandler);
	enQueue (&eq);
	pthread_cond_broadcast (&mCondMulti); // thread pool start
}

template <typename T>
void CAsyncProcProxy<T> ::enQueue (ST_ASYNC_QUEUE<T> *eq)
{
	if (!eq) {
		return ;
	}

	CUtils::CScopedMutex scopedMutex (&mMutexQueue);

	mQueue.push (*eq);
}

template <typename T>
ST_ASYNC_QUEUE<T> CAsyncProcProxy<T> ::deQueue (bool isPeep)
{
	CUtils::CScopedMutex scopedMutex (&mMutexQueue);


	ST_ASYNC_QUEUE<T> dq;

	if (mQueue.size() == 0) {
		return dq;

	} else {

		if (isPeep) {
			return mQueue.front();
		} else {
			dq = mQueue.front();
			mQueue.pop();
			return dq;
		}
	}
}

} // namespace ImmSocketService

#endif
