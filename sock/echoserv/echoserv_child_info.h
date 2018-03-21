#ifndef _ECHOSERV_CHILD_INFO_H_
#define _ECHOSERV_CHILD_INFO_H_

#include "echoserv_child_proc.h"


/*
 * 定数定義
 */
#define CHILD_MAX				(1024) /*ユーザ毎のファイルディスクリプタ数の上限
																[ulimit -n]で確認できる */
#define CHILD_INFO_TBL_NUM		(256) /* 共有メモリ上のテーブル数 */

/*
 * 型定義
 */
typedef struct child_info_tbl {
	pid_t nPid;
	ST_CLIENT_INFO stClientInfo;
	struct child_info_tbl *pNext;
} ST_CHILD_INFO_TBL;

typedef struct child_info_tbl_ptr {
	ST_CHILD_INFO_TBL *pstTop;
	ST_CHILD_INFO_TBL *pstBottom;
} ST_CHILD_INFO_TBL_PTR;


/*
 * 外部宣言
 */
extern ST_CHILD_INFO_TBL *gpstChildInfoTbl;
extern ST_CHILD_INFO_TBL_PTR *gpstChildInfoTblPtr;

extern int AddChildInfoTable( pid_t nPid, ST_CLIENT_INFO *pstClientInfo );
extern void SelChildInfoTable (
	pid_t nPid,
	int nFlag,
	int (*pFuncCallback)( ST_CHILD_INFO_TBL*, void* ),
	void *pArg
);
extern int DelChildInfoTable( pid_t nPid );
extern void InitChildInfoTable( void );
extern void FreeChildInfoTable( void );
extern void RefChildInfoTable( void );

#endif
