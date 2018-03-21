#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


struct info {
	int m_nKey;
	struct info *pNext;
};

struct info *g_pstInfoTop;
struct info *g_pstInfoBottom;


void FreeAllInfoList( void );
void InitInfoList( void );
int AddInfoList( int );
struct info *GetInfoList( void );
void RefInfoList( void );
int DelInfoList( int );


/*
 * リスト全体の領域開放
 */
void FreeAllInfoList( void )
{
	struct info *pstTmp = g_pstInfoTop;
	struct info *pstTmp2 = NULL;

	while ( pstTmp ) {
		pstTmp2 = pstTmp;
		pstTmp = pstTmp->pNext;
		free( pstTmp2 );
	}

	g_pstInfoTop = NULL;
	g_pstInfoBottom = NULL;

	return;
}

/*
 * リスト初期化
 */
void InitInfoList( void )
{
	if ( g_pstInfoTop ) {
		FreeAllInfoList();
	}

	g_pstInfoTop = NULL;
	g_pstInfoBottom = NULL;

	return;
}

/*
 * リストに追加
 * キーを指定する
 */
int AddInfoList( int nKey )
{
	struct info *pstTmp = NULL;
	
	pstTmp = (struct info*)malloc( sizeof(struct info) );
	if ( !pstTmp ) {
		perror( "malloc()" );
		return -1;
	}

	pstTmp->m_nKey = nKey;
	pstTmp->pNext = NULL;

	if ( !g_pstInfoTop ) {

		/* リストが空の場合 TopとBottomにセット */
		g_pstInfoTop = pstTmp;
		g_pstInfoBottom = pstTmp;

	} else {

		/* リストが空でない場合 Bottomの後ろに繋ぎ 自分がBottomになる */
		g_pstInfoBottom->pNext = pstTmp;
		g_pstInfoBottom = pstTmp;

	}

	return 0;
} 

/*
 * リストから取得(FIFO)
 * 取得後にfreeする必要がある
 */
struct info *GetInfoList( void )
{
	struct info *pstRtn = NULL;

	if ( !g_pstInfoTop ) {

		/* リストが空の場合はNULLを返す */

	} else {

		/* リストが空でなければTopを返す */
		pstRtn = g_pstInfoTop;
		g_pstInfoTop = g_pstInfoTop->pNext;

	}

	return pstRtn;
}

/*
 * リスト参照
 */
void RefInfoList( void )
{
	struct info *pstTmp = g_pstInfoTop;

	while ( pstTmp ) {
		printf(
			"m_nKey=[%d] Addr:[%p] pNext:[%p]\n",
			pstTmp->m_nKey,
			pstTmp,
			pstTmp->pNext
		);
		pstTmp = pstTmp->pNext;
	}

	return;
}

/*
 * リストを検索し該当部を削除 (領域開放と繋ぎ変え)
 * 検索一致し削除した数を戻す
 */
int DelInfoList( int nKey )
{
	int i = 0;
	struct info *pstTmp = g_pstInfoTop;
	struct info *pstBefore = NULL;

	while ( pstTmp ) {

		if ( pstTmp->m_nKey == nKey ) {
			i ++;

			if ( g_pstInfoTop == g_pstInfoBottom ) {
				/* リスト要素が1つの場合 */

				g_pstInfoTop = NULL;
				g_pstInfoBottom = NULL;

			} else {
				/* リスト要素が複数の場合 */

				if ( pstTmp == g_pstInfoTop ) {

					/* Topで検索された場合 自分の後ろをTopとする */
					g_pstInfoTop = pstTmp->pNext;

				} else if ( pstTmp == g_pstInfoBottom ) {

					/* Bottomで検索された場合 BeforeをBottomとする */
					pstBefore->pNext = NULL;
					g_pstInfoBottom = pstBefore;

				} else {

					/* TopでもBottomでなければ 自分を飛び越してポインタをつなぐ */
					pstBefore->pNext = pstTmp->pNext;

				}
			}

			free( pstTmp );

		} else {
			/* キーに該当しなければ自分をBeforeに一時保存 */
			pstBefore = pstTmp;
		}

		pstTmp = pstTmp->pNext;
	}

	return i ;
}

int main( void )
{
	int nRtn = 0;
	struct info *pstTmp = NULL;

	InitInfoList();

	puts("add 1");
	if ( ( nRtn = AddInfoList( 1 ) ) < 0 ) {
		exit( EXIT_FAILURE );
	}

	puts("add 2"); AddInfoList( 2 );
	puts("add 3"); AddInfoList( 3 );
	puts("add 100"); AddInfoList( 100 );
	puts("add 1000"); AddInfoList( 1000 );
	puts("add 255"); AddInfoList( 255 );
	puts("add 255"); AddInfoList( 255 );
	puts("add 777"); AddInfoList( 777 );

	RefInfoList();
puts("");

	printf( "del 255 %d\n", DelInfoList( 255 ) );
	printf( "del 777 %d\n", DelInfoList( 777 ) );

	RefInfoList();
puts("");

	printf( "del 3 %d\n", DelInfoList( 3 ) );

	puts("add 555"); AddInfoList( 555 );
	puts("add 666"); AddInfoList( 666 );

	RefInfoList();
puts("");

	if ( (pstTmp = GetInfoList()) ) {
		printf( "get %d\n", pstTmp->m_nKey );
		free( pstTmp );
	}
	if ( (pstTmp = GetInfoList()) ) {
		printf( "get %d\n", pstTmp->m_nKey );
		free( pstTmp );
	}
	if ( (pstTmp = GetInfoList()) ) {
		printf( "get %d\n", pstTmp->m_nKey );
		free( pstTmp );
	}
	if ( (pstTmp = GetInfoList()) ) {
		printf( "get %d\n", pstTmp->m_nKey );
		free( pstTmp );
	}

	RefInfoList();
puts("");

	FreeAllInfoList();

	RefInfoList();
puts("");


	exit( EXIT_SUCCESS );
}
