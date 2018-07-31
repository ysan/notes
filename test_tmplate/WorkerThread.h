#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "Utils.h"


#define THREAD_NAME_SIZE		(16)

typedef void* (*P_THREAD_HANDLER) (void* args);

class CWorkerThread {
public:
	CWorkerThread (void);
	virtual ~CWorkerThread (void);

	bool create (bool isJoinable=false);
	void waitDestroy (void); // join
	pthread_t getId (void);
	bool isAlive (void);
	bool isJoinable (void);

protected:
	virtual void onThreadMainRoutine (void);

	void setName (char *p);
	char *getName (void);

private:
	static void *threadHandler (void *args);
	void run (void);

	pthread_t mThreadId;
	pthread_mutex_t mMutex;

	bool mIsJoinable;
	bool mIsCreated;

	char mName [THREAD_NAME_SIZE];
};

#endif
