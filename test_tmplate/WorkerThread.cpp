#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "WorkerThread.h"
#include "Utils.h"


CWorkerThread::CWorkerThread (void) :
	mIsJoinable (false),
	mIsCreated (false)
{
	pthread_mutex_init (&mMutex, NULL);

	memset (mName, 0x00, sizeof (mName));
}

CWorkerThread::~CWorkerThread (void)
{
	pthread_mutex_destroy (&mMutex);
}


bool CWorkerThread::create (bool isJoinable)
{
	CUtils::CScopedMutex scopedMutex (&mMutex);


	int rtn = 0;

	if (!mIsCreated) {
		mIsCreated = true;
		mIsJoinable = isJoinable;

		// create thread

		if (!isJoinable) {

			pthread_attr_t threadAttr;
			rtn = pthread_attr_init (&threadAttr);
			if (rtn != 0) {
				_UTL_PERROR ("pthread_attr_init()");
				mIsCreated = false;
				return false;
			}

			rtn = pthread_attr_setdetachstate (&threadAttr, PTHREAD_CREATE_DETACHED);
			if (rtn != 0) {
				_UTL_PERROR ("pthread_attr_setdetachstate()");
				mIsCreated = false;
				return false;
			}

			rtn = pthread_create (&mThreadId, &threadAttr, threadHandler, this);
			if (rtn != 0) {
				_UTL_PERROR ("pthread_create()");
				mIsCreated = false;
				return false;
			}

		} else {

			// joinable
			rtn = pthread_create (&mThreadId, NULL, threadHandler, this);
			if (rtn != 0) {
				_UTL_PERROR ("pthread_create()");
				mIsCreated = false;
				return false;
			}
		}
	}

	return true;
}

void CWorkerThread::waitDestroy (void)
{
	if (mIsJoinable) {
		if (pthread_join (mThreadId, NULL) != 0) {
			_UTL_PERROR ("pthread_join()");
		}

	} else {
		while (mIsCreated) {
			sleep (1);
		}
	}
}

void *CWorkerThread::threadHandler (void *args)
{
	if (!args) {
		return NULL;
	}

	CWorkerThread *pInstance = static_cast <CWorkerThread*> (args);
	if (pInstance) {
		pInstance->run ();
	}

	return NULL;
}

void CWorkerThread::run (void)
{
	onThreadMainRoutine ();

	mIsCreated = false;


	// thread end
}

void CWorkerThread::onThreadMainRoutine (void)
{
	setName ((char*)"WorkerThread");
	_UTL_LOG_I ("%s %s\n", __FILE__, __func__);
}

pthread_t CWorkerThread::getId (void)
{
	return mThreadId;
}

void CWorkerThread::setName (char *p)
{
	if (!p) {
		return;
	}

	memset (mName, 0x00, sizeof(mName));
	strncpy (mName, p, sizeof(mName) -1);

	CUtils::setThreadName (p);
}

char *CWorkerThread::getName (void)
{
	return mName;
}

bool CWorkerThread::isAlive (void)
{
	return mIsCreated;
}

bool CWorkerThread::isJoinable (void)
{
	return mIsJoinable;
}

