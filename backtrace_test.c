//
// [compile command]
// gcc backtrace_test.c -rdynamic -lpthread
//
// pthread_sigmaskについて以下のモデルの場合
// [mainthread]
//      |--[threadA]
//      |--[threadB]
// ・mainthreadでマスクするとプロセス全体がマスク
// ・子threadでのみマスクしてそのthreadにsignal送るとmainthreadが受ける
// ・マスクしてないハンドラも設定しないsignalを子threadに送るとプロセスは落ちる

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <execinfo.h>
#include <pthread.h>
#include <signal.h>


#define BACKTRACE_BUFF_SIZE	(100)


void execBackTrace (void)
{
	int i;
	int n;
	void *pBuff [BACKTRACE_BUFF_SIZE];
	char **pRtn;

	n = backtrace (pBuff, BACKTRACE_BUFF_SIZE);
	pRtn = backtrace_symbols (pBuff, n);
	if (!pRtn) {
		perror ("backtrace_symbols()");
		return;
	}

	for (i = 0; i < n; i ++) {
		printf ("%s\n", pRtn[i]);
	}

	free (pRtn);
}

static void sigHandler (int nSigno)
{
	execBackTrace ();
}

pthread_mutex_t gMutex;
pthread_cond_t gCond;
void *workerThreadA (void *pArg)
{
	int ret = 0;
	struct timeval now;
	struct timespec timeout;

	pthread_mutex_init (&gMutex, NULL);
	pthread_cond_init (&gCond, NULL);

	// sigmask debug
//	sigset_t nSigset;
//	sigemptyset (&nSigset);
//	sigaddset (&nSigset, SIGUSR2);
//	if (pthread_sigmask (SIG_BLOCK, &nSigset, NULL) < 0) {
//		perror ("pthread_sigmask()");
//		exit (EXIT_FAILURE);
//	}

	while (1) {
		gettimeofday (&now, NULL);
		time_t now_sec = now.tv_sec;
		long int now_nsec = now.tv_usec * 1000;
		timeout.tv_sec = now_sec + 5;
		timeout.tv_nsec = now_nsec + 0;

		ret = pthread_cond_timedwait (&gCond, &gMutex, &timeout);
		switch (ret) {
		case ETIMEDOUT:
			printf ("pthread_cond_timedwait() timeout\n");
			break;
		default:
			printf ("pthread_cond_timedwait() other error\n");
			break;
		}
	}

	pthread_mutex_destroy (&gMutex);
	pthread_cond_destroy (&gCond);

	return NULL;
}

static void *workerThreadB (void *pArg)
{
	// sigmask debug
//	sigset_t nSigset;
//	sigemptyset (&nSigset);
//	sigaddset (&nSigset, SIGUSR2);
//	if (pthread_sigmask (SIG_BLOCK, &nSigset, NULL) < 0) {
//		perror ("pthread_sigmask()");
//		exit (EXIT_FAILURE);
//	}

	while (1) {
		sleep (2);
		puts ("sleep");
	}

	return NULL;
}

int main (void)
{
	// sigmask debug
//	sigset_t nSigset;
//	sigemptyset (&nSigset);
//	sigaddset (&nSigset, SIGUSR2);
//	if (pthread_sigmask (SIG_BLOCK, &nSigset, NULL) < 0) {
//		perror ("pthread_sigmask()");
//		exit (EXIT_FAILURE);
//	}


	struct sigaction stSigact;
	memset (&stSigact, 0x00, sizeof(stSigact));
	stSigact.sa_handler = sigHandler;
	if (sigaction (SIGUSR2, &stSigact, NULL) < 0) {
		perror ("sigaction()");
		exit (EXIT_FAILURE);
	}

	pthread_t threadIdA;
	if (pthread_create (&threadIdA, NULL, workerThreadA, NULL) != 0) {
		perror ("pthread_create()");
		exit (EXIT_FAILURE);
    }
	pthread_t threadIdB;
	if (pthread_create (&threadIdB, NULL, workerThreadB, NULL) != 0) {
		perror ("pthread_create()");
		exit (EXIT_FAILURE);
    }

	pthread_join (threadIdA, NULL);
	pthread_join (threadIdB, NULL);


	exit (EXIT_SUCCESS);
}
