#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "echoserv_main.h"
#include "echoserv_socket.h"
#include "echoserv_signal.h"
#include "echoserv_accept.h"
#include "echoserv_shm.h"
#include "echoserv_child_info.h"


/*
 * メイン
 */
int main( void )
{
	int nFdSockSv = 0;


	InitSigVal();

	/* 子プロセス開始からSIGRTMINを割り込み禁止にする為 */
	SetSigmask();
	if ( DisableInterrupt() < 0 ) {
		fprintf( stderr, "Err: DisableInterrupt() is failure.\n" );
		exit( EXIT_FAILURE );
	}


	if ( SetSigHandle( SIGCHLD ) < 0 ) {
		fprintf( stderr, "Err: SetSigHandle( SIGCHLD ) is failure.\n" );
		exit( EXIT_FAILURE );
	}

	if ( IgnoreSignal( SIGPIPE ) < 0 ) {
		fprintf( stderr, "Err: IgnoreSignal( SIGPIPE ) is failure.\n" );
		exit( EXIT_FAILURE );
	}

	/* サーバソケット作成 */
	if ( ( nFdSockSv = CreateServSock( SERVER_PORT ) ) < 0 ) {
		fprintf( stderr, "Err: CreateServSock() is failure.\n" );
		exit( EXIT_FAILURE );
	}


	/* 共有メモリ作成 */
	if ( ( gnShmIdMsgInfo = CreateSharedMemory( SHMKEY_MSG_INFO, sizeof(ST_MSG_INFO) ) ) < 0 ) {
		fprintf( stderr, "Err: CreateSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE );
	}

	if ( ( gnShmIdChildInfoTbl =
			CreateSharedMemory ( SHMKEY_CHILD_INFO_TBL,
									sizeof(ST_CHILD_INFO_TBL)*CHILD_INFO_TBL_NUM ) ) < 0 ) {
		fprintf( stderr, "Err: CreateSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE );
	}

	if ( ( gnShmIdChildInfoTblPtr =
			CreateSharedMemory( SHMKEY_CHILD_INFO_TBL_PTR, sizeof(ST_CHILD_INFO_TBL_PTR) ) ) < 0 ) {
		fprintf( stderr, "Err: CreateSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE );
	}


	/* 共有メモリアタッチ */
	if ( AttSharedMemory( gnShmIdMsgInfo, (void**)&gpstMsgInfo ) < 0 ) {
		fprintf( stderr, "Err: AttSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE );
	}

	if ( AttSharedMemory( gnShmIdChildInfoTbl, (void**)&gpstChildInfoTbl ) < 0 ) {
		fprintf( stderr, "Err: AttSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE );
	}

	if ( AttSharedMemory( gnShmIdChildInfoTblPtr, (void**)&gpstChildInfoTblPtr ) < 0 ) {
		fprintf( stderr, "Err: AttSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE );
	}

	InitSharedMemory( gpstMsgInfo, sizeof(ST_MSG_INFO) );
	InitSharedMemory( gpstChildInfoTbl, sizeof(ST_CHILD_INFO_TBL) );
	InitSharedMemory( gpstChildInfoTblPtr, sizeof(ST_CHILD_INFO_TBL_PTR) );

	InitChildInfoTable();


	/* セマフォ作成 */
	if ( ( gnSemIdMsgInfo = CreateSemaphore( SEMKEY_MSG_INFO ) ) < 0 ) {
		fprintf( stderr, "Err: CreateSemaphore() is failure.\n" );
		exit( EXIT_FAILURE );
	}

	/* セマフォ初期化 */
	if ( InitSemaphore( gnSemIdMsgInfo, SEM_IDX_SHM_MSG_INFO ) < 0 ) {
		fprintf( stderr, "Err: InitSemaphore() is failure.\n" );
		exit( EXIT_FAILURE );
	}


	/* 主処理 */
	if ( SelectAcceptFork( nFdSockSv ) < 0 ) {
		fprintf( stderr, "Err: SelectAcceptFork() is failure.\n" );
		close( nFdSockSv );
		exit( EXIT_FAILURE );
	}


	/* 共有メモリデタッチ */
	if ( DetSharedMemory( (void*)gpstMsgInfo ) < 0 ) {
		fprintf( stderr, "Err: DetSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE );
	}

	if ( DetSharedMemory( (void*)gpstChildInfoTbl ) < 0 ) {
		fprintf( stderr, "Err: DetSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE );
	}

	if ( DetSharedMemory( (void*)gpstChildInfoTblPtr ) < 0 ) {
		fprintf( stderr, "Err: DetSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE );
	}


	/* 共有メモリ削除 */
	if ( DelSharedMemory( gnShmIdMsgInfo ) < 0 ) {
		fprintf( stderr, "Err: DelSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE );
	}

	if ( DelSharedMemory( gnShmIdChildInfoTbl ) < 0 ) {
		fprintf( stderr, "Err: DelSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE );
	}

	if ( DelSharedMemory( gnShmIdChildInfoTblPtr ) < 0 ) {
		fprintf( stderr, "Err: DelSharedMemory() is failure.\n" );
		exit( EXIT_FAILURE );
	}



	exit( EXIT_SUCCESS );
}
