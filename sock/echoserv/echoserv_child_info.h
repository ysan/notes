#ifndef _ECHOSERV_CHILD_INFO_H_
#define _ECHOSERV_CHILD_INFO_H_

#include "echoserv_child_proc.h"


/*
 * $BDj?tDj5A(B
 */
#define CHILD_MAX				(1024) /*$B%f!<%6Kh$N%U%!%$%k%G%#%9%/%j%W%??t$N>e8B(B
																[ulimit -n]$B$G3NG'$G$-$k(B */
#define CHILD_INFO_TBL_NUM		(256) /* $B6&M-%a%b%j>e$N%F!<%V%k?t(B */

/*
 * $B7?Dj5A(B
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
 * $B30It@k8@(B
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
