/*
 * compile command:
 *      cc tree.c -o tree -Wall -Imycommon -Lmycommon -lmycommon
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "mycommon.h"


typedef struct info {
	char szData[ 128 ];
} ST_INFO;

typedef struct tree {
	uint16_t nIdx;
	struct tree *pLeft;
	struct tree *pRight;
	ST_INFO stInfo; // tree構造で保持する主データ
} ST_TREE;


static uint32_t gnCntTree;
static ST_TREE *gpstTreeRoot;
static ST_TREE *gpstSearchBefore; // サーチ中 保持データ
static BOOL g_isLeftSearchBefore; // サーチ中 保持データ 左か右か



void InitTree( void );											// extern
BOOL AddTree( ST_INFO *pstInfo, size_t nArgSize );				// extern
static BOOL AddTreeFromIdx( uint16_t nIdx, ST_INFO *pstInfo );
static BOOL AddTreeImp( ST_TREE *pstAdd );
BOOL DelTree( uint16_t nIdx );									// extern
ST_TREE *SearchTree( uint16_t nIdx );							// extern
static void RefTree( ST_TREE *pstArg );
void RefAllTree();												// extern
static void DestroyTree( ST_TREE *pstArg );
void DestroyAllTree();											// extern
static uint16_t GenIdx( const uint16_t *pArg, size_t nArgSize );


/**
 * 初期化
 */
void InitTree( void )
{
	DestroyAllTree();

	gnCntTree = 0;
	gpstTreeRoot = NULL;
	gpstSearchBefore = NULL;
	g_isLeftSearchBefore = FALSE;
}

/**
 * 要素追加
 */
BOOL AddTree( ST_INFO *pstInfo, size_t nArgSize )
{
	if (gnCntTree >= 0xffff) {
		return FALSE;
	}

	uint32_t nIdx = GenIdx( (const uint16_t*)pstInfo, nArgSize );

	/* index重複確認する */
	while (1) {
		if (!SearchTree( (uint16_t)nIdx )) {
			break;
		}
		nIdx ++;
		nIdx &= 0xffff;
	}

	if (!AddTreeFromIdx( (uint16_t)nIdx, pstInfo )) {
		return FALSE;
	}

	gnCntTree ++;

	return TRUE;
}

/**
 * 要素追加
 * indexより
 */
static BOOL AddTreeFromIdx( uint16_t nIdx, ST_INFO *pstInfo )
{
	ST_TREE *pstAdd = NULL;

	pstAdd = (ST_TREE*)malloc( sizeof(ST_TREE) );
	if (!pstAdd) {
		PERROR( "malloc()" );
		return FALSE;
	}

	pstAdd->nIdx = nIdx;
	memcpy( &(pstAdd->stInfo), pstInfo, sizeof(ST_INFO) );
	pstAdd->pLeft = NULL;
	pstAdd->pRight = NULL;


	/* 初回 */
	if (!gpstTreeRoot) {
		gpstTreeRoot = pstAdd;
		return TRUE;
	}

	/* 初回以降の場合はここに */
	if (!AddTreeImp( pstAdd )) {
		return FALSE;
	}

	return TRUE;
}

/**
 * 要素追加
 * 検索して追加
 */
static BOOL AddTreeImp( ST_TREE *pstAdd )
{
	BOOL bRtn = TRUE;
	ST_TREE *pstWork = gpstTreeRoot;

	if (!pstAdd) {
		return FALSE;
	}

	/* rootが空 */
	if (!pstWork) {
		return FALSE;
	}

	while (1) {
		if (pstWork->nIdx > pstAdd->nIdx) {
			if (!(pstWork->pLeft)) {
				pstWork->pLeft = pstAdd;
				break;
			} else {
				/* 続行 */
				pstWork = pstWork->pLeft;
			}
		} else if (pstWork->nIdx < pstAdd->nIdx) {
			if (!(pstWork->pRight)) {
				pstWork->pRight = pstAdd;
				break;
			} else {
				/* 続行 */
				pstWork = pstWork->pRight;
			}
		} else {
			/*
			 * indexが重複した場合
			 * ここにはこないはず
			 */
			bRtn = FALSE;
			LOG_E( "index duplicate.\n" );
			break;
		}
	}

	return bRtn;
}

/**
 * 削除
 * 要素を1つ削除
 */
BOOL DelTree( uint16_t nIdx )
{
	ST_TREE *pstWork = NULL;
	ST_TREE *pstWorkLeft = NULL;
	ST_TREE *pstWorkRight = NULL;


	pstWork = SearchTree( nIdx );
	if (!pstWork) {
		LOG_W( "index not found.\n" );
		return FALSE;
	}

	/* 	親のポインタを切り離す */
	if (g_isLeftSearchBefore) {
		gpstSearchBefore->pLeft = NULL;
	} else {
		gpstSearchBefore->pRight = NULL;
	}

	/* マルチスレッド時は サーチ開始からここまで排他すべき */


	pstWorkLeft = pstWork->pLeft;
	pstWorkRight = pstWork->pRight;

	free( pstWork );
	pstWork = NULL;

	gnCntTree --;

//puts("ref in del");
//RefAllTree();

	/* 以下 子の要素を再構成 */

	if (pstWorkLeft) {
		if (!AddTreeImp( pstWorkLeft )) {
			return FALSE;
		}
	}

	if (pstWorkRight) {
		if (!AddTreeImp( pstWorkRight )) {
			return FALSE;
		}
	}

	return TRUE;
}

/**
 * 検索
 */
ST_TREE *SearchTree( uint16_t nIdx )
{
	ST_TREE *pstWork = gpstTreeRoot;
	ST_TREE *pstRtn = NULL;

	/* rootが空 */
	if (!pstWork) {
		return NULL;
	}

	while (1) {
		if (pstWork->nIdx > nIdx) {
			if (!(pstWork->pLeft)) {
				pstRtn = NULL;
				break;
			} else {
				/* 続行 */

				/* 1つ前を保存 */
				gpstSearchBefore = pstWork;
				g_isLeftSearchBefore = TRUE;

				pstWork = pstWork->pLeft;
			}
		} else if (pstWork->nIdx < nIdx) {
			if (!(pstWork->pRight)) {
				pstRtn = NULL;
				break;
			} else {
				/* 続行 */

				/* 1つ前を保存 */
				gpstSearchBefore = pstWork;
				g_isLeftSearchBefore = FALSE;

				pstWork = pstWork->pRight;
			}
		} else {
			/* indexが一致した */
			pstRtn = pstWork;
			break;
		}
	}

	return pstRtn;
}

/**
 * 参照
 * 引数で指定した要素とその子以下を参照する
 */
static void RefTree( ST_TREE *pstArg )
{
	if (!pstArg) {
		return;
	}

	LOG_I (
		"addr:[%p] nIdx:[%d] pLeft:[%p] pRight:[%p] data:[%s]\n",
		pstArg,
		pstArg->nIdx,
		pstArg->pLeft,
		pstArg->pRight,
		pstArg->stInfo.szData
	);

	if (pstArg->pLeft) {
		RefTree( pstArg->pLeft );
	}

	if (pstArg->pRight) {
		RefTree( pstArg->pRight );
	}
}

/**
 * 全データ参照
 */
void RefAllTree( void )
{
	RefTree( gpstTreeRoot );
}

/**
 * データ破棄
 * 引数で指定した要素とその子以下を破棄する
 */
static void DestroyTree( ST_TREE *pstArg )
{
	if (!pstArg) {
		return;
	}

	LOG_I( "free(%p)\n", pstArg );

	if (pstArg->pLeft) {
		DestroyTree( pstArg->pLeft );
	}

	if (pstArg->pRight) {
		DestroyTree( pstArg->pRight );
	}

	free( pstArg );
	pstArg = NULL;
}

/**
 * 全データ破棄
 */
void DestroyAllTree( void )
{
	DestroyTree( gpstTreeRoot );
	gpstTreeRoot = NULL;
	gnCntTree = 0;
}

/**
 * Index生成
 * 0〜65535 のランダム?な値を返す
 * (ということは このtree構造のデータは最大65536個)
 * IPチェックサムを参考
 */
static uint16_t GenIdx( const uint16_t *pArg, size_t nArgSize )
{
	uint32_t nIdx = 0;

	while (nArgSize > 1) {
		nIdx += *pArg;

		if (nIdx & 0x10000) {
			/* 16bit目が立っていたらオーバーフロー分を足す */
			nIdx = ( nIdx >> 16 ) + ( nIdx & 0xffff );
		}

		nArgSize -= 2;
		pArg ++;
	}

	/* 余りの1byte分 */
	if (nArgSize == 1) {
		nIdx += *pArg;

		if (nIdx & 0x10000) {
			/* 16bit目が立っていたらオーバーフロー分を足す */
			nIdx = ( nIdx >> 16 ) + ( nIdx & 0xffff );
		}
	}

	return (uint16_t)nIdx;
}

int main( void )
{
	ST_INFO stInfo;

	InitTree();

	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "aaa" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "bbb" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "ccc" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "aaa" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "egehhryjyrj" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "234346576uhy5g" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "dfergk4k-5v,.aa" );
	AddTree( &stInfo, sizeof(stInfo) );

	RefAllTree();

	DelTree( 25284 );

puts("del idx:25284 after");

	RefAllTree();

	DestroyAllTree();

puts("destroy after");

	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "eee" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "ggg" );
	AddTree( &stInfo, sizeof(stInfo) );

	RefAllTree();



	exit( EXIT_SUCCESS );
}
