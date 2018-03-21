#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mycommon.h"
#include "echoserv_accept.h"
#include "echoserv_signal.h"
#include "echoserv_child_proc.h"
#include "echoserv_child_info.h"
#include "echoserv_shm.h"


/*
 * プロトタイプ宣言
 */
static int DelZombieProc( void );


/*
 * acceptの監視(select)及び
 * 1クライアントごとの子プロセス生成
 */
int SelectAcceptFork( int nFdSockSv )
{
	int nRtn = 0;
	int nRtnDelCnt = 0;
	int nFdSockCl = 0;
	int nCntChild = 0;
	struct sockaddr_in stAddrCl;
	socklen_t nLen = (socklen_t)sizeof(struct sockaddr_in);
	pid_t nPid;
	fd_set stFds;
	struct timeval stTimeval;
	ST_CLIENT_INFO stClientInfo;


	while (1) {

		if ( GetSigVal() == SIGCHLD ) {

			/* ゾンビプロセス削除 */
			if( ( nRtnDelCnt = DelZombieProc() < 0 ) ) {
				fprintf( stderr, "DelZombieProc() is failure.\n" );
				return -1;
			}

			InitSigVal();

			/* 子プロセス数減算 */
			nCntChild -= nRtnDelCnt;

			/* 子プロセスID管理テーブルを参照 */
			RefChildInfoTable();
		}


		FD_ZERO( &stFds );
		FD_SET( nFdSockSv, &stFds );
		stTimeval.tv_sec = TIMEOUT_SEC_VAL;
		stTimeval.tv_usec = TIMEOUT_USEC_VAL;

		/* サーバソケット監視 */
		nRtn = select( nFdSockSv+1, &stFds, NULL, NULL, &stTimeval );
		if( nRtn < 0 ){
			if ( errno == EINTR ) {
				/* シグナルによる割り込み */
				fprintf( stdout, "Interrupt occurred. [%s]\n", strsignal( GetSigVal() ) );

				if ( GetSigVal() == SIGCHLD ) {

					/* ゾンビプロセス削除 */
					if( ( nRtnDelCnt = DelZombieProc() ) < 0 ) {
						fprintf( stderr, "DelZombieProc() is failure.\n" );
						return -1;
					}

					InitSigVal();

					/* 子プロセス数減算 */
					nCntChild -= nRtnDelCnt;

					/* 子プロセスID管理テーブルを参照 */
					RefChildInfoTable();
				}

				/* 監視に戻る */
				continue;

			} else {
				/* select()エラー */
				perror( "select()" );
				return -1;
			}

		} else if ( nRtn == 0 ) {
			/* タイムアウト */

			/* 監視に戻る */
			continue;
		}

		/* ここにきた場合 サーバソケットがレディである */

		if ( ( nFdSockCl = accept( nFdSockSv, (struct sockaddr*)&stAddrCl, &nLen ) ) < 0 ) {
			perror( "accept()" );
			return -1;
		}

		/* 子プロセスに渡す情報をセット */
		memset( &stClientInfo, 0x00, sizeof(ST_CLIENT_INFO) );
		stClientInfo.nFdSockSv = nFdSockSv;
		stClientInfo.nFdSockCl = nFdSockCl;
		memcpy( &(stClientInfo.stAddrCl), &stAddrCl, sizeof(struct sockaddr) );
		/* グローバル変数はそのままコピーされるが念のため */
		stClientInfo.nShmIdMsgInfo = gnShmIdMsgInfo;
		stClientInfo.nShmIdChildInfoTbl = gnShmIdChildInfoTbl;
		stClientInfo.nShmIdChildInfoTblPtr = gnShmIdChildInfoTblPtr;
		stClientInfo.nSemIdMsgInfo = gnSemIdMsgInfo;

		nPid = fork();
		if ( nPid < 0 ) {
			perror( "fork()" );
			close( nFdSockCl );
			return -1;

		} else if ( nPid == 0 ) {
			/* 子プロセス */

			/* 子プロセスメイン処理 */
			ChildProcMain( &stClientInfo );

		}


		/* クライアントソケットは使用しないので閉じる */
		close( nFdSockCl );

		/* 子プロセス数加算 */
		nCntChild ++;

		fprintf( stdout, "ChildPid:[%d] is generated. TotalChildNum:[%d]\n", nPid, nCntChild );

		/* 新規 子プロセス情報を管理テーブルに追加 */
		if ( AddChildInfoTable( nPid, &stClientInfo ) < 0 ) {
			fprintf( stderr, "Err: AddChildInfoTable() is failure.\n" );
			return -1;
		}
	}

	return 0;
}

/*
 * ゾンビプロセス削除
 */
static int DelZombieProc( void )
{
	pid_t nPidWait = 0;
	int nCnt = 0;

	while (1) {

		/*
		 * ゾンビプロセスが見つからなければ処理を返す
		 * WNOHANGでブロックしない
		 */
		nPidWait = waitpid( (pid_t)-1, NULL, WNOHANG );
		if ( nPidWait < 0 ) {
			if ( errno == ECHILD ) {
				/* ゾンビプロセスなし */
				/* 子プロセスがいない場合ここに来る */
				fprintf( stdout, "Zombie is none.(ECHILD)\n" );
				break;

			} else {
				/* エラー */
				perror( "waitpid()" );
				return -1;
			}

		} else if ( nPidWait == 0 ) {
			/* ゾンビプロセスなし */
			fprintf( stdout, "Zombie is none.\n" );
			break;

		} else {
			/* ゾンビプロセスを削除した */
			fprintf( stdout, "Zombie is reject. Pid:[%d]\n", nPidWait );

			/* 削除数をカウント */
			nCnt ++;

			/* 削除した子プロセスIDを管理テーブルから削除 */
			(void)DelChildInfoTable( nPidWait );
		}
	}

	return nCnt;
}
