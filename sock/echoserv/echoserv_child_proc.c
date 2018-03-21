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
#include "echoserv_child_proc.h"
#include "echoserv_child_info.h"
#include "echoserv_signal.h"
#include "echoserv_shm.h"


/*
 * プロトタイプ宣言
 */
static int NotifyOtherChild( ST_CHILD_INFO_TBL *pstChildInfoTbl, void *pArg );
static int SendMsg( int nReadPos, void *pArg );
static int SendRecv( int nFdSockCl );


/*
 * 自分以外の子プロセスに通知
 * SelChildInfoTable()のコールバック関数
 */
static int NotifyOtherChild( ST_CHILD_INFO_TBL *pstChildInfoTbl, void *pArg )
{
	int nRtn = 0;
	ST_SEND_SIGQUE_ARGS *pstSendSigqueArgs = (ST_SEND_SIGQUE_ARGS*)pArg;

	nRtn = SendSigque (
				pstChildInfoTbl->nPid,
				pstSendSigqueArgs->nSigKind,
				pstSendSigqueArgs->nVal
			);
	if ( nRtn < 0 ) {
		fprintf( stderr, "Err: SendSigque( %d ) is failure.\n", pstChildInfoTbl->nPid );
		return -1;
	}

	fprintf( stdout, "Info: SendSigque( %d ) is success.\n", pstChildInfoTbl->nPid );

	return 0;
}

/*
 * 送信処理
 * PursueSigInfoVal()のコールバック関数
 */
static int SendMsg( int nReadPos, void *pArg )
{
	int nRtn = 0;
	int nFdSockCl = *((int*)pArg);
	int nWriteSize = 0;
	unsigned char *pszMsg = NULL;


	/* 共有メモリ上のメッセージをクライアントに送信 */
	pszMsg = gpstMsgInfo->gstMsgData[ nReadPos ].szMsg;
	nWriteSize = gpstMsgInfo->gstMsgData[ nReadPos ].nWriteSize;

	/* 送信 */
	if ( ( nRtn = SendData( nFdSockCl, pszMsg, nWriteSize ) ) < 0 ) {
		fprintf( stderr, "Err: SendData() is failure.\n" );
		return -1;
	}

	fprintf( stdout, "[%d]bytes sent.\n", nRtn );

	return 0;
}

/*
 * 送受信ループ
 */
static int SendRecv( int nFdSockCl )
{
	int nFlagConn = 0;
	int nRtnRecv = 0;
	unsigned char szBuff[ RECV_BUFF_SIZE ];
	ST_SEND_SIGQUE_ARGS stSendSigqueArgs;


	while (1) {

		/* 割り込み禁止 */
		if ( DisableInterrupt() < 0 ) {
			fprintf( stderr, "Err: DisableInterrupt() is failure.\n" );
			return -1;
		}

		if ( GetSigValChild() == SIGRTMIN ) {
			PursueSigInfoVal( SendMsg, (void*)&nFdSockCl );
			InitSigValChild();
		}

		/* 割り込み許可 */
		if ( EnableInterrupt() < 0 ) {
			fprintf( stderr, "Err: EnableInterrupt() is failure.\n" );
			return -1;
		}


		memset( szBuff, 0x00, sizeof(szBuff) );

		/* 受信 */
		nRtnRecv = RecvData( nFdSockCl, szBuff, sizeof(szBuff), &nFlagConn );
		if ( nRtnRecv < 0 ) {
			if ( errno == EINTR ) {
				/* シグナルによる割り込み */
				fprintf( stdout, "Interrupt occurred. [%s]\n", strsignal( GetSigValChild() ) );

				/* 割り込み禁止 */
				if ( DisableInterrupt() < 0 ) {
					fprintf( stderr, "Err: DisableInterrupt() is failure.\n" );
					return -1;
				}

				if ( GetSigValChild() == SIGRTMIN ) {
					PursueSigInfoVal( SendMsg, (void*)&nFdSockCl );
					InitSigValChild();
				}

				/* 割り込み許可 */
				if ( EnableInterrupt() < 0 ) {
					fprintf( stderr, "Err: EnableInterrupt() is failure.\n" );
					return -1;
				}

				/* 受信処理に戻る */
				continue;

			} else {
				fprintf( stderr,"Err: RecvData() is failure. nRtnRecv=[%d]\n", nRtnRecv );
				return -1;
			}

		}


		/* 以下正常受信後の処理 */

		fprintf( stdout, "%s", szBuff );
		fprintf( stdout, "[%d]bytes received.\n", nRtnRecv );

		if ( nFlagConn ) {
			/* 0 byte recieved */
			fprintf( stdout, "Connection with a partner went out...\n" );
			break;
		}

		/* 受信データを共有メモリに格納 */
		if ( SetSharedMemoryMsgInfo( getpid(), szBuff, nRtnRecv ) < 0 ) {
			fprintf( stderr, "Err: SetSharedMemoryMsgInfo() is failure.\n" );
			return -1;
		}


		/* 自分以外のクライアントに通知 */
		stSendSigqueArgs.nSigKind = SIGRTMIN;
		stSendSigqueArgs.nVal = gpstMsgInfo->nReadPos; /* 共有メモリ読み出し位置も通知 */

		SelChildInfoTable( getpid(), FALSE, NotifyOtherChild, (void*)&stSendSigqueArgs );

	}


	return 0;
}

/*
 * 子プロセスメイン処理
 * (親プロセスにてSIGRTMIN割り込みを禁止した状態で開始)
 */
int ChildProcMain( ST_CLIENT_INFO *pstClientInfo )
{
	int nRtn = 0;

	/* サーバソケットは使用しないので閉じる */
	close( pstClientInfo->nFdSockSv );

	fprintf( stdout, "client:[%s] is connect. --- Pid:[%d]\n",
								inet_ntoa(pstClientInfo->stAddrCl.sin_addr), getpid() );

	InitSigValChild();
	InitSigInfoVal();

	if ( SetSigHandleSiginfo( SIGRTMIN ) < 0 ) {
		fprintf( stderr, "Err: SetSigHandleSiginfo() is failure.\n" );
		exit( EXIT_FAILURE ); /* 子プロセスexit() */
	}


	/* 共有メモリアタッチ */
	if ( AttSharedMemory( pstClientInfo->nShmIdMsgInfo, (void**)&gpstMsgInfo ) < 0 ) {
		fprintf( stdout, "Err: AttSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE ); /* 子プロセスexit() */
	}

	if ( AttSharedMemory( gnShmIdChildInfoTbl, (void**)&gpstChildInfoTbl ) < 0 ) {
		fprintf( stderr, "Err: AttSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE ); /* 子プロセスexit() */
	}

	if ( AttSharedMemory( gnShmIdChildInfoTblPtr, (void**)&gpstChildInfoTblPtr ) < 0 ) {
		fprintf( stderr, "Err: AttSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE ); /* 子プロセスexit() */
	}


	/* 主処理 送受信 */
	nRtn = SendRecv( pstClientInfo->nFdSockCl );
	if ( nRtn < 0 ) {
		fprintf( stdout, "Err: SendRecv() is failure.\n" );
		close( pstClientInfo->nFdSockCl );
		exit( EXIT_FAILURE ); /* 子プロセスexit() */
	}

	close( pstClientInfo->nFdSockCl );


	fprintf( stdout, "client:[%s] is closed. --- Pid:[%d]\n",
								inet_ntoa(pstClientInfo->stAddrCl.sin_addr), getpid() );


	/* 共有メモリデタッチ */
	if ( DetSharedMemory( (void*)gpstMsgInfo ) < 0 ) {
		fprintf( stderr, "Err: DetSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE ); /* 子プロセスexit() */
	}

	if ( DetSharedMemory( (void*)gpstChildInfoTbl ) < 0 ) {
		fprintf( stderr, "Err: DetSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE ); /* 子プロセスexit() */
	}

	if ( DetSharedMemory( (void*)gpstChildInfoTblPtr ) < 0 ) {
		fprintf( stderr, "Err: DetSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE ); /* 子プロセスexit() */
	}


	exit( EXIT_SUCCESS ); /* 子プロセスexit() */
}
