#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "mycommon.h"
#include "pcap_signal.h"
#include "pcap_proc.h"


volatile sig_atomic_t gCatchSignal;

static void SigHandler( int );


/*
 * シグナルマスク設定
 */
int SetSignalMask( void )
{
	sigset_t nSigset;

	sigemptyset( &nSigset );
	sigaddset( &nSigset, SIGINT );
	sigaddset( &nSigset, SIGCHLD );

	/* シグナルマスクを設定 */
	if( sigprocmask( SIG_BLOCK, &nSigset, NULL) < 0 ){
		perror( "sigprocmask()" );
		return -1;
	}

	return 0;
}

/*
 * シグナルハンドラ関数
 */
static void SigHandler( int nSigno )
{
	gCatchSignal = nSigno;
	return;
}

/*
 * シグナルハンドラ設定
 */
int SetSigHandle( int nSigkind )
{
	struct sigaction strSigact;

	memset( &strSigact, 0x00, sizeof(strSigact) );
	strSigact.sa_handler = SigHandler;
	/*
	 * strSigact.sa_flags = SA_RESTART を使用すると
	 * msgrcv()はrestartされない、EINTRで判定できる。
	 * recv()はrestartする。
	 */

	if( sigaction( nSigkind, &strSigact, NULL ) < 0 ){
		perror( "sigaction()" );
		return -1;
	}

	return 0;
}

/*
 * シグナル受信待ち
 */
int WaitSignal( pid_t *pnPid )
{
	int nFlagAbnormal = 0;
	sigset_t nSigsetNow;
	siginfo_t strSiginfo;

	/* 現在のシグナルマスクを取得 */
	if( sigprocmask( SIG_BLOCK, NULL, &nSigsetNow ) < 0 ){
		perror( "sigprocmask()" );
		return -1;
	}

	while(1){
		memset( &strSiginfo, 0x00, sizeof(strSiginfo) );

		if( sigwaitinfo( &nSigsetNow, &strSiginfo ) ){

			fprintf( stdout, "main() Catch Signal! [%s]\n", sys_siglist[strSiginfo.si_signo] );

			if( strSiginfo.si_signo == SIGINT ){

				/* パケット受信プロセスに終了指示を送信 */
				fprintf( stdout, "Send SIGUSR1 to RecvPacketProc().\n" );
				if( kill( *pnPid, SIGUSR1 ) < 0 ){
					perror( "kill()" );
					return -1;
				}

			} else if( strSiginfo.si_signo == SIGCHLD ){

				/* ゾンビプロセス削除 */
				if( CheckWaitpid() < 0 ){
					fprintf( stderr, "Err: CheckWaitpid()\n" );
					return -1;
				}

				if( strSiginfo.si_status == EXIT_SUCCESS ){
					/* 子プロセス正常終了時 */

					if( !nFlagAbnormal ){
						if( strSiginfo.si_pid == *pnPid ){
							/*
							 * パケット受信プロセス正常終了後に 
							 * パケット解析プロセスに終了指示を送信
							 */
							fprintf( stdout, "Send SIGUSR1 to AnalyzePacketProc().\n" );
							if( kill( *(pnPid+1), SIGUSR1 ) < 0 ){
								perror( "kill()" );
								return -1;
							}

						} else {
							/* パケット解析プロセス終了 ループを抜けメイン終了へ */
							break;
						}
					} else {
						/* ループを抜けメイン終了へ */
						break;
					}

				} else {
					/* 子プロセス異常終了時 */
					nFlagAbnormal = 1;

					/* 生存確認し終了指示を送信 */
					if( !kill( *pnPid, 0 ) ){
						fprintf( stdout, "Send SIGUSR1 to RecvPacketProc(). (at Err)\n" );
						if( kill( *pnPid, SIGUSR1 ) < 0 ){
							perror( "kill()" );
							return -1;
						}
					} else {
						if( errno != ESRCH ){
							perror( "kill()" );
							return -1;
						}
					}

					if( !kill( *(pnPid+1), 0 ) ){
						fprintf( stdout, "Send SIGUSR1 to AnalyzePacketProc(). (at Err)\n" );
						if( kill( *(pnPid+1), SIGUSR1 ) < 0 ){
							perror( "kill()" );
							return -1;
						}
					} else {
						if( errno != ESRCH ){
							perror( "kill()" );
							return -1;
						}
					}

				}
			}

		} else {
			perror( "sigwaitinfo()" );
			return -1;
		}

	}

	return 0;
}
