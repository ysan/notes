#ifndef _ECHOSERV_SHM_H_
#define _ECHOSERV_SHM_H_


/*
 * 定数定義
 */
#define SHMKEY_MSG_INFO				((key_t)0x10000000)
#define SHMKEY_CHILD_INFO_TBL		((key_t)0x20000000)
#define SHMKEY_CHILD_INFO_TBL_PTR	((key_t)0x30000000)
#define SEMKEY_MSG_INFO				((key_t)0x40000000)

#define MSG_SIZE					(4096)
#define MSG_DATA_IDX				(512)	/* 2のべき乗の値にすること */

#define SEM_INIT_VAL				(1)

enum {
	SEM_IDX_SHM_MSG_INFO = 0,
	SEM_IDX_MAX
};

/*
 * 型定義
 */
typedef struct msg_data {
	pid_t nPid;
	int nWriteSize;
	unsigned char szMsg[ MSG_SIZE ];
} ST_MSG_DATA;

typedef struct msg_info {
	int nReadPos;
	int nWritePos;
	ST_MSG_DATA gstMsgData[ MSG_DATA_IDX ];
} ST_MSG_INFO;

typedef union semun {		/* linux/sem.hより抜粋 */
	int val;				/* value for SETVAL */
	struct semid_ds *buf;	/* buffer for IPC_STAT & IPC_SET */
	unsigned short *array;	/* array for GETALL & SETALL */
} UN_SEM;


/*
 * 外部宣言
 */
extern int gnShmIdMsgInfo;
extern int gnShmIdChildInfoTbl;
extern int gnShmIdChildInfoTblPtr;
extern ST_MSG_INFO *gpstMsgInfo;
extern int gnSemIdMsgInfo;

extern int CreateSharedMemory( key_t nKey, size_t nSize );
extern int AttSharedMemory( int nShmId, void **pRtn );
extern int DetSharedMemory( void *pShmAddr );
extern int DelSharedMemory( int nShmId );
extern void InitSharedMemory( void *pArg, size_t nSize );
extern int SetSharedMemoryMsgInfo (
	pid_t nPid,
	unsigned char *pszBuff,
	size_t nBuffSize
);
extern int CreateSemaphore( key_t nKey );
extern int InitSemaphore( int nSemId, int nIdx );
extern int LockSemaphore( int nSemId, int nIdx );
extern int UnlockSemaphore( int nSemId, int nIdx );

#endif
