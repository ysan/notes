#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


typedef struct info {
	int nKey;
	unsigned int nData;
	struct info *pPrev;
	struct info *pNext;
} ST_INFO;


ST_INFO *gpstInfoTop;
ST_INFO *gpstInfoBottom;
ST_INFO *gpstInfoCurrent;


void FreeAllList( void );
void InitList( void );
int AddList( int, unsigned int );
void RefList( void );
void RefListR( void );
int DelList( void );
int InsList( int, unsigned int );
int SearchListLRU( int );


/*
 * リスト全体の領域開放
 */
void FreeAllList( void )
{
	ST_INFO *pstTmp = gpstInfoTop;
	ST_INFO *pstTmp2 = NULL;

	while ( pstTmp ) {
		pstTmp2 = pstTmp;
		pstTmp = pstTmp->pNext;
		free( pstTmp2 );
	}

	gpstInfoTop = NULL;
	gpstInfoBottom = NULL;
	gpstInfoCurrent = NULL;

	return;
}

/*
 * リスト初期化
 */
void InitList( void )
{
	if ( gpstInfoTop ) {
		FreeAllList();
	}

	gpstInfoTop = NULL;
	gpstInfoBottom = NULL;
	gpstInfoCurrent = NULL;

	return;
}

/*
 * リスト追加
 * Bottomの後ろに1要素を追加する
 */
int AddList( int nKey, unsigned int nData )
{
	ST_INFO *pstTmp = NULL;
	
	pstTmp = (ST_INFO*)malloc( sizeof(ST_INFO) );
	if ( !pstTmp ) {
		perror( "malloc()" );
		return -1;
	}

	/* データをセット */
	pstTmp->nKey = nKey;
	pstTmp->nData = nData;

	if ( !gpstInfoTop ) {
		/* リストが空の場合 */

		/* 前後はNULLとなる */
		pstTmp->pPrev = NULL;
		pstTmp->pNext = NULL;

		/* Top,Bottom,Currentにセット */
		gpstInfoTop = pstTmp;
		gpstInfoBottom = pstTmp;
		gpstInfoCurrent = pstTmp;

	} else {
		/* リストが空でない場合 */

		/* 前はBottom、後はNULLとなる */
		pstTmp->pPrev = gpstInfoBottom;
		pstTmp->pNext = NULL;

		/* Bottomの後ろに繋ぎ 自分がBottom及びCurrentになる */
		gpstInfoBottom->pNext = pstTmp;
		gpstInfoBottom = pstTmp;
		gpstInfoCurrent = pstTmp;

	}

	return 0;
} 

/*
 * リスト参照
 */
void RefList( void )
{
	ST_INFO *pstTmp = gpstInfoTop;

	while ( pstTmp ) {
		printf(
			"%snKey=[%d] nData=[0x%08x] Addr:[%p] ( pPrev:[%p] pNext:[%p] )\n",
			pstTmp == gpstInfoCurrent ? "*" : " ",
			pstTmp->nKey,
			pstTmp->nData,
			pstTmp,
			pstTmp->pPrev,
			pstTmp->pNext
		);
		pstTmp = pstTmp->pNext;
	}

	return;
}

/*
 * リスト参照 逆順
 */
void RefListR( void )
{
	ST_INFO *pstTmp = gpstInfoBottom;

	while ( pstTmp ) {
		printf(
			"%snKey=[%d] nData=[0x%08x] Addr:[%p] ( pPrev:[%p] pNext:[%p] )\n",
			pstTmp == gpstInfoCurrent ? "*" : " ",
			pstTmp->nKey,
			pstTmp->nData,
			pstTmp,
			pstTmp->pPrev,
			pstTmp->pNext
		);
		pstTmp = pstTmp->pPrev;
	}

	return;
}

/*
 * リスト削除 (領域開放と繋ぎ変え)
 * Currentが指す1要素を削除する
 */
int DelList( void )
{
	ST_INFO *pstTmp = gpstInfoCurrent;

	if ( !pstTmp ) {
		return -1;
	}

	if ( gpstInfoTop == gpstInfoBottom ) {
		/* リスト要素が1つの場合 */

		gpstInfoTop = NULL;
		gpstInfoBottom = NULL;
		gpstInfoCurrent = NULL;

	} else {
		/* リスト要素が複数の場合 */

		if ( pstTmp == gpstInfoTop ) {
			/* Topである場合 */

			/* 自分の後ろをTop及びCurrentとする */
			gpstInfoTop = pstTmp->pNext;
			gpstInfoCurrent = pstTmp->pNext;
			pstTmp->pNext->pPrev = NULL;

		} else if ( pstTmp == gpstInfoBottom ) {
			/* Bottomである場合 */

			/* 自分の前をBottom及びCurrentとする */
			gpstInfoBottom = pstTmp->pPrev;
			gpstInfoCurrent = pstTmp->pPrev;
			pstTmp->pPrev->pNext = NULL;

		} else {

			/*
			 * TopでもBottomでない場合
			 * 自分を飛び越してポインタをつなぐ
			 * Currentは後ろ側とする
			 */
			pstTmp->pNext->pPrev = pstTmp->pPrev;
			pstTmp->pPrev->pNext = pstTmp->pNext;
			gpstInfoCurrent = pstTmp->pNext;
		}
	}

	free( pstTmp );

	return 0;
}

/*
 * リスト挿入
 * Currentが指すリスト要素の後ろに追加する
 */
int InsList( int nKey, unsigned int nData )
{
	ST_INFO *pstTmp = NULL;
	
	pstTmp = (ST_INFO*)malloc( sizeof(ST_INFO) );
	if ( !pstTmp ) {
		perror( "malloc()" );
		return -1;
	}

	/* データをセット */
	pstTmp->nKey = nKey;
	pstTmp->nData = nData;

	if ( !gpstInfoTop ) {
		/* リストが空の場合 */

		/* 前後はNULLとなる */
		pstTmp->pPrev = NULL;
		pstTmp->pNext = NULL;

		/* Top,Bottom,Currentにセット */
		gpstInfoTop = pstTmp;
		gpstInfoBottom = pstTmp;
		gpstInfoCurrent = pstTmp;

	} else {
		/* リストが空でない場合 */

		if ( gpstInfoCurrent == gpstInfoBottom ) {
			/* CurrentがBottomの場合 */

			/* 前は元々のBottom、後はNULLとなる */
			pstTmp->pPrev = gpstInfoBottom;
			pstTmp->pNext = NULL;

			/* 元々のBottomの後ろに繋ぎ 自分がBottom及びCurrentになる */
			gpstInfoBottom->pNext = pstTmp;
			gpstInfoBottom = pstTmp;
			gpstInfoCurrent = pstTmp;
	
		} else {

			/* 元々のCurrentの後に追加する */
			pstTmp->pPrev = gpstInfoCurrent;
			pstTmp->pNext = gpstInfoCurrent->pNext;

			gpstInfoCurrent->pNext = pstTmp;
			/* 上で書き換えたので 次の次の前 */
			gpstInfoCurrent->pNext->pNext->pPrev = pstTmp;

			gpstInfoCurrent = pstTmp;
		}
	}

	return 0;
}

/*
 * リストを検索
 * 検索された要素をTopに移動する
 */
int SearchListLRU( int nKey )
{
	ST_INFO *pstTmp = gpstInfoTop;

	if ( !gpstInfoTop ) {
		return -1;
	}

	while ( pstTmp ) {

		if ( pstTmp->nKey == nKey ) {

			if( gpstInfoTop == gpstInfoBottom ){
				/* リスト要素が1つの場合 */
				/* 何もしない */

			} else {
				/* リスト要素が複数の場合 */

				if ( pstTmp == gpstInfoTop ) {
					/* Topが検索された場合 何もしない */

				} else if ( pstTmp == gpstInfoBottom ) {
					/* Bottomが検索された場合 */

					/* Bottomを自分の一つ前に */
					gpstInfoBottom = pstTmp->pPrev;
					gpstInfoBottom->pNext = NULL;

					/* 自分がTopの前に入る */
					pstTmp->pNext = gpstInfoTop;
					gpstInfoTop->pPrev = pstTmp;
					pstTmp->pPrev = NULL;

					/* 自分がTopに */
					gpstInfoTop = pstTmp;

				} else {
					/* TopでもBottomでもない場合 */

					/* 自分を飛び越してポインタをつなぐ */
					pstTmp->pNext->pPrev = pstTmp->pPrev;
					pstTmp->pPrev->pNext = pstTmp->pNext;

					/* 自分がTopの前に入る */
					pstTmp->pNext = gpstInfoTop;
					gpstInfoTop->pPrev = pstTmp;
					pstTmp->pPrev = NULL;

					/* 自分がTopに */
					gpstInfoTop = pstTmp;
				}
			}

			break;
		}

		pstTmp = pstTmp->pNext;
	}

	return 0;
}

int main( void )
{
	InitList();

	if ( AddList( 1, 0x00000001 ) ) {
		exit( EXIT_FAILURE );
	}

	AddList( 2, 0x00000002 );
	AddList( 3, 0x00000003 );
	AddList( 4, 0x00000004 );

	RefList();
puts("");

gpstInfoCurrent = gpstInfoCurrent->pPrev->pPrev;

	DelList();
	RefList();
puts("");

	InsList( 10, 0x11110001 );
	InsList( 11, 0x11110002 );
	RefList();
puts("");

	SearchListLRU(11);
	SearchListLRU(1);
	SearchListLRU(3);
	RefList();
puts("");

	FreeAllList();
	RefList();
puts("");



	exit( EXIT_SUCCESS );
}
