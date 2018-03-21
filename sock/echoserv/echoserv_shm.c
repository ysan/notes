#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>  

#include "mycommon.h"
#include "echoserv_shm.h"


/*
 * 変数宣言
 */
int gnShmIdMsgInfo;
int gnShmIdChildInfoTbl;
int gnShmIdChildInfoTblPtr;
ST_MSG_INFO *gpstMsgInfo;
int gnSemIdMsgInfo;


/*
 * 共有メモリ作成
 */
int CreateSharedMemory( key_t nKey, size_t nSize )
{
	int nShmId = 0;

	/* 毎回新しいキーで作成する */
//	if ( ( nShmId = shmget( IPC_PRIVATE, sizeof(ST_MSG_INFO), S_IRUSR|S_IWUSR ) ) < 0 ) {

	/*
	 * 第1引数で指定したキーで作成する
	 * 既に存在している場合はそのまま
	 */
	if ( ( nShmId = shmget( nKey, nSize, IPC_CREAT|S_IRUSR|S_IWUSR ) ) < 0 ) {
		perror( "shmget()" );
		return -1;
	}

	return nShmId;
}

/*
 * 共有メモリアタッチ
 */
int AttSharedMemory( int nShmId, void **pRtn )
{
	void *pShmAddr = shmat( nShmId, NULL, 0 );
	if ( pShmAddr == (void*)-1 ) {
		perror( "shmat()" );
		return -1;
	}

	*pRtn = pShmAddr;

	return 0;
}

/*
 * 共有メモリデタッチ
 */
int DetSharedMemory( void *pShmAddr )
{
	if ( shmdt( pShmAddr ) < 0 ) {
		perror( "shmdt()" );
		return -1;
	}

	return 0;
}

/*
 * 共有メモリ削除
 */
int DelSharedMemory( int nShmId )
{
	if ( shmctl( nShmId, IPC_RMID, NULL ) < 0 ) {
		perror( "shmctl( IPC_RMID )" );
		return -1;
	}

	return 0;
}

/*
 * 共有メモリ データ初期化
 */
void InitSharedMemory( void *pArg, size_t nSize )
{
	memset( pArg, 0x00, nSize );
	return;
}

/*
 * 共有メモリ データ格納
 * ST_MSG_INFO型
 */
int SetSharedMemoryMsgInfo (
	pid_t nPid,
	unsigned char *pszBuff,
	size_t nWriteSize
)
{
	/* セマフォロック */
	if ( LockSemaphore( gnSemIdMsgInfo, SEM_IDX_SHM_MSG_INFO ) ) {
		fprintf( stderr, "Err: LockSemaphore() is failure.\n" );
		return -1;
	}


	gpstMsgInfo->gstMsgData[ gpstMsgInfo->nWritePos ].nPid = nPid;
	gpstMsgInfo->gstMsgData[ gpstMsgInfo->nWritePos ].nWriteSize =
							MSG_SIZE > nWriteSize ? nWriteSize : MSG_SIZE;

	/* 一度クリアしてコピー */
	memset( gpstMsgInfo->gstMsgData[ gpstMsgInfo->nWritePos ].szMsg, 0x00, MSG_SIZE );
	memcpy (
		gpstMsgInfo->gstMsgData[ gpstMsgInfo->nWritePos ].szMsg,
		pszBuff,
		MSG_SIZE > nWriteSize ? nWriteSize : MSG_SIZE
	);

	gpstMsgInfo->nReadPos = gpstMsgInfo->nWritePos;

	/* 書き込み位置を進める */
	gpstMsgInfo->nWritePos ++;
	gpstMsgInfo->nWritePos &= MSG_DATA_IDX -1;


	/* セマフォ開放 */
	if ( UnlockSemaphore( gnSemIdMsgInfo, SEM_IDX_SHM_MSG_INFO ) ) {
		fprintf( stderr, "Err: UnlockSemaphore() is failure.\n" );
		return -1;
	}


	return 0;
}

/*
 * セマフォ作成
 */
int CreateSemaphore( key_t nKey )
{
	int nSemId;

	/*
	 * 第1引数で指定したキーで作成する
	 * 既に存在している場合はそのまま
	 */
	if ( ( nSemId = semget( nKey, SEM_IDX_MAX, IPC_CREAT|S_IRUSR|S_IWUSR ) ) < 0 ) {
		perror( "semget()" );
		return -1;
	}

	return nSemId;
}

/*
 * セマフォ初期化
 */
int InitSemaphore( int nSemId, int nIdx )
{
	UN_SEM unSem;

	memset( &unSem, 0x00, sizeof(unSem) );

	/* 初期値1にする */
	unSem.val = SEM_INIT_VAL;

	if ( semctl( nSemId, nIdx, SETVAL, unSem ) < 0 ) {
		perror( "semctl( SETVAL )" );
		return -1;
	}

	return 0;
}

/*
 * セマフォロック
 */
int LockSemaphore( int nSemId, int nIdx )
{
	struct sembuf stSembuf[1];

	/*
	 * セマフォが1の場合 -1して戻る
	 * セマフォが0の場合 1(SEM_INIT_VAL) になるまで待つ
	 */
	stSembuf[0].sem_num = nIdx;
	stSembuf[0].sem_op = -1;
	stSembuf[0].sem_flg = SEM_UNDO;

	if ( semop( nSemId, stSembuf, sizeof(stSembuf)/sizeof(struct sembuf) ) < 0 ) {
		perror( "semop(): lock" );
		return -1;
	}

	return 0;
}

/*
 * セマフォ開放
 */
int UnlockSemaphore( int nSemId, int nIdx )
{
	struct sembuf stSembuf[1];

	/* セマフォを 1(SEM_INIT_VAL) にする */
	stSembuf[0].sem_num = nIdx;
	stSembuf[0].sem_op = SEM_INIT_VAL;
	stSembuf[0].sem_flg = SEM_UNDO;

	if ( semop( nSemId, stSembuf, sizeof(stSembuf)/sizeof(struct sembuf) ) < 0 ) {
		perror( "semop(): unlock" );
		return -1;
	}

	return 0;
}
