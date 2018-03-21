#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "mycommon.h"
#include "echoserv_child_info.h"
#include "echoserv_child_proc.h"
#include "echoserv_shm.h"


/*
 * 変数宣言
 */
ST_CHILD_INFO_TBL *gpstChildInfoTbl; /* 共有メモリアタッチ用 */
ST_CHILD_INFO_TBL_PTR *gpstChildInfoTblPtr; /* 共有メモリアタッチ用 */

/*
 * プロトタイプ宣言
 */
static ST_CHILD_INFO_TBL *GetAllocShmAddr( void );


/*
 * 共有メモリ割り当て
 */
static ST_CHILD_INFO_TBL *GetAllocShmAddr( void )
{
	int i = 0;
	ST_CHILD_INFO_TBL *pstShmAddr = gpstChildInfoTbl;

	while ( i < CHILD_INFO_TBL_NUM ) {

		if ( pstShmAddr->nPid == 0 ) {
			return pstShmAddr;
		}

		i ++;
		pstShmAddr ++;
	}

	/* 空きが無い場合NULLを返す */
	return NULL;
}

/*
 * 子プロセス管理テーブルに追加
 * FIFO
 */
int AddChildInfoTable( pid_t nPid, ST_CLIENT_INFO *pstClientInfo )
{
	ST_CHILD_INFO_TBL *pstNow = NULL;


	/* pidの重複チェック入れた方がいい DDDDDDDDDDDDDDDDDDDDDDDDDDDD */


	/* 共有メモリ割り当て */
	pstNow = GetAllocShmAddr();
	if ( !pstNow ) {
		perror( "malloc()" );
		return -1;
	}

	/* 値をセット */
	pstNow->nPid = nPid;
	memcpy( &(pstNow->stClientInfo), pstClientInfo, sizeof(ST_CLIENT_INFO) );

	/* NULLでリスト終端 */
	pstNow->pNext = NULL;

	if ( !(gpstChildInfoTblPtr->pstTop) ) {

		/* リストが空の場合 TopとBottomにセット */
		gpstChildInfoTblPtr->pstTop = pstNow;
		gpstChildInfoTblPtr->pstBottom = pstNow;

	} else {

		/* リストが空でない場合 Bottomの後ろに繋ぎ 自分がBottomになる */
		gpstChildInfoTblPtr->pstBottom->pNext = pstNow;
		gpstChildInfoTblPtr->pstBottom = pstNow;

	}

	return 0;
}

/*
 * 子プロセス管理テーブル検索しコールバック関数実行
 * 第1引数キーと一致した場合(一致しない場合)コールバック関数を実行する
 * 第2引数で一致した場合、一致しない場合の切り替え
 */
void SelChildInfoTable (
	pid_t nPid,
	int nFlag,
	int (*pFuncCallback)( ST_CHILD_INFO_TBL*, void* ),
	void *pArg
)
{
	ST_CHILD_INFO_TBL *pstNow = gpstChildInfoTblPtr->pstTop;

	while ( pstNow ) {

		if ( nFlag ) {
			if ( pstNow->nPid == nPid ) {

				if (pFuncCallback) {
					pFuncCallback( pstNow, pArg );
				}

				/* ここには一回しか来ないのでbreakする */
				break;
			}

		} else {
			if ( pstNow->nPid != nPid ) {

				if (pFuncCallback) {
					pFuncCallback( pstNow, pArg );
				}

			}
		}

		pstNow = pstNow->pNext;
	}
}

/*
 * 子プロセス管理テーブル検索し該当部を削除
 * (領域開放とポインタ繋ぎかえ)
 * 検索一致し削除した数を戻す
 */
int DelChildInfoTable( pid_t nPid )
{
	int i = 0;
	ST_CHILD_INFO_TBL *pstNow = gpstChildInfoTblPtr->pstTop;
	ST_CHILD_INFO_TBL *pstBefore = NULL;

	while ( pstNow ) {

		if ( pstNow->nPid == nPid ) {
			i ++;

			if ( gpstChildInfoTblPtr->pstTop == gpstChildInfoTblPtr->pstBottom ) {
				/* リスト要素が1つの場合 */

				gpstChildInfoTblPtr->pstTop = NULL;
				gpstChildInfoTblPtr->pstBottom = NULL;

			} else {
				/* リスト要素が複数の場合 */

				if( pstNow == gpstChildInfoTblPtr->pstTop ){

					/* Topが検索された場合Nowの後ろのポインタをTopに */
					gpstChildInfoTblPtr->pstTop = pstNow->pNext;

				} else if( pstNow == gpstChildInfoTblPtr->pstBottom ){

					/* Bottomが検索された場合BeforeをBottomに */
					pstBefore->pNext = NULL;
					gpstChildInfoTblPtr->pstBottom = pstBefore;

				} else {

					/* 上記以外の場合 現在のポインタをとばして繋ぐ */
					pstBefore->pNext = pstNow->pNext;
				}
			}

			InitSharedMemory( pstNow, sizeof(ST_CHILD_INFO_TBL) );

		} else {
			/* 検索に該当しない場合のみ ポインタ保存 */
			/* (連続で検索された場合これがBeforeのままとなる) */
			pstBefore = pstNow;
		}

		pstNow = pstNow->pNext;
	}

	return i;
}

/*
 * 子プロセス管理テーブル初期化
 * 共有メモリアタッチ後に実行すること
 */
void InitChildInfoTable( void )
{
	gpstChildInfoTblPtr->pstTop = NULL;
	gpstChildInfoTblPtr->pstBottom = NULL;
	return;
}

/*
 * 子プロセス管理テーブル全削除
 * (リスト全体の領域開放)
 */
void FreeChildInfoTable( void )
{
	ST_CHILD_INFO_TBL *pstNow = gpstChildInfoTblPtr->pstTop;
	ST_CHILD_INFO_TBL *pstNow2 = NULL;

	while ( pstNow ) {
		pstNow2 = pstNow;
		pstNow = pstNow->pNext;
		InitSharedMemory( pstNow2, sizeof(ST_CHILD_INFO_TBL) );
	}

	gpstChildInfoTblPtr->pstTop = NULL;
	gpstChildInfoTblPtr->pstBottom = NULL;

	return;
}

/*
 * 子プロセス管理テーブル参照
 */
void RefChildInfoTable( void )
{
	ST_CHILD_INFO_TBL *pstNow = gpstChildInfoTblPtr->pstTop;

	while ( pstNow ) {
		fprintf( stdout, "nPid:[%d] addr:[%p -> %p]\n", pstNow->nPid, pstNow, pstNow->pNext );
		pstNow = pstNow->pNext;
	}

	return;
}
