#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "mycommon.h"
#include "pcap_addr2name.h"


/* 全ての外部関数,変数はパケット解析プロセス内で使用する */


static ST_NAMELIST *gpstNameList[NAMELIST_HASHSIZE];
static ST_NAMELIST *gpstNameListTop;
static ST_NAMELIST *gpstNameListBottom;


static ST_NAMELIST *MallocNewNameList( void );
static void AddMallocList( ST_NAMELIST* );


/*
 * アドレス変換リスト初期化
 */
void InitNameList( void )
{
	int i = 0;
	for ( i = 0; i < NAMELIST_HASHSIZE; i ++ ) {
		gpstNameList[i] = NULL;
	}

	gpstNameListTop = NULL;
	gpstNameListBottom = NULL;

	return;
}

/*
 * 逆引き アドレス->名前変換
 * ポインタリストで結果を保持する
 * 引数nAddrはネットワークバイトオーダーで指定する
 */
int GetName( uint32_t nAddr, char *pszName, size_t nBuffSize )
{
	int nFlgTop = 1;
	char szRes[ NAME_SIZE ];
	ST_NAMELIST *pstTmp = NULL;
	ST_NAMELIST *pstBefore = NULL;

	/* 格納場所を得る */
	pstTmp = gpstNameList[ CALC_HASH( nAddr ) ];
//printf("%p ",pstTmp);

	/* リストを検索 */
	while ( pstTmp ) {
		if ( nAddr == pstTmp->nAddr ) {
			memcpy( pszName, pstTmp->szName, nBuffSize > NAME_SIZE ? NAME_SIZE : nBuffSize );
//printf("list ");
			return 0;
		}

		pstBefore = pstTmp;
		pstTmp = pstTmp->pNext;
		nFlgTop = 0;
	}

	/* 以下リストに存在しない場合 */

	if ( nFlgTop == 1 ) {
		/* 先頭で且つNULLである */
		if ( !( gpstNameList[ CALC_HASH( nAddr ) ] = MallocNewNameList() ) ) {
			fprintf( stderr, "Err: MallocNewNameList()\n" );	
			return -1;
		}

		pstTmp = gpstNameList[ CALC_HASH( nAddr ) ];
//printf("top ");

	} else {
		/*
		 * 空の領域を一つ割り当て及びポインタの繋ぎ替え
		 */
		if ( !( pstBefore->pNext = MallocNewNameList() ) ) {
			fprintf( stderr, "Err: MallocNewNameList()\n" );	
			return -1;
		}
		pstTmp = pstBefore->pNext;
//printf("alloc ");
	}

	/* リストにIPアドレスを格納 */
	pstTmp->nAddr = nAddr;


	memset( szRes, 0x00, sizeof(szRes) );

	/* 逆引き実行 */
	if ( GetReverseIp( nAddr, szRes, NAME_SIZE -1 ) < 0 ) {
		fprintf( stderr, "Err: GetReverseIp()\n" );	
		return -1;
	}

	/* リストに名前変換結果を格納 */
	memcpy( pstTmp->szName, szRes, NAME_SIZE );

	/* 引数pszNameに名前変換結果を返却 */
	memcpy( pszName, szRes, nBuffSize > NAME_SIZE ? NAME_SIZE : nBuffSize );

	return 0;
}

/*
 * NAMELIST_MALLOC_SIZE分 領域確保しアドレスを返す
 * 領域が無くなったら再度確保を行う
 */
static ST_NAMELIST *MallocNewNameList( void )
{
	int i = 0;
	static int nCnt = 0;
	static ST_NAMELIST *pstTmp = NULL;
	ST_NAMELIST *pstRtn = NULL;

	if ( !nCnt ) {
		pstTmp = (ST_NAMELIST*)malloc( sizeof(ST_NAMELIST) * NAMELIST_MALLOC_SIZE );
		if ( !pstTmp ) {
			perror( "malloc()" );
			return NULL;
		}

		for ( i = 0; i < NAMELIST_MALLOC_SIZE; i++ ) {
			(pstTmp+i)->nAddr = 0x00;
			memset( (pstTmp+i)->szName, 0x00, NAME_SIZE );
			(pstTmp+i)->pNext = NULL;
			(pstTmp+i)->pNextMalloc = NULL;
		}

		AddMallocList( pstTmp );

//printf("malloc ");
    }

	nCnt ++;

	if ( nCnt == NAMELIST_MALLOC_SIZE ) {
		nCnt = 0;
	}

	pstRtn = pstTmp;

	if ( nCnt != NAMELIST_MALLOC_SIZE ) {
		pstTmp ++;
	}

	return pstRtn;
}

/*
 * アドレス変換リスト参照
 */
void RefNameList( void )
{
	int i = 0;
	char szBuff[20];
	ST_NAMELIST *pstTmp = NULL;

	fprintf( stdout, "--------- RefNameList() ---------\n" );

	for ( i = 0; i < NAMELIST_HASHSIZE; i ++ ) {

		if ( gpstNameList[i] ) {
			fprintf( stdout, "0x%06x:\n", i );
			pstTmp = gpstNameList[i];

			while ( pstTmp ) {
				memset( szBuff, 0x00, sizeof(szBuff) );
				inet_ntop( AF_INET, &(pstTmp->nAddr), szBuff, sizeof(szBuff) );

				fprintf( stdout, "  [%p:%s:%s]\n", pstTmp, szBuff, pstTmp->szName );

				pstTmp = pstTmp->pNext;

			}
		}
	}

	fprintf( stdout, "---------------------------------\n" );

	return;
}

/*
 * Malllocリスト追加
 * リストの末尾に順次追加
 */
static void AddMallocList( ST_NAMELIST *pstAdd )
{
	pstAdd->pNextMalloc = NULL;

	if ( !gpstNameListTop ) {
		gpstNameListTop = pstAdd;
		gpstNameListBottom = pstAdd;

	} else {
		gpstNameListBottom->pNextMalloc = pstAdd;
		gpstNameListBottom = pstAdd;

	}

	return;
}

/*
 * Malllocリスト全体の領域開放
 */
void FreeAllMallocList( void )
{
	ST_NAMELIST *pstTmp = gpstNameListTop;
	ST_NAMELIST *pstTmp2 = NULL;

	while ( pstTmp ) {
		pstTmp2 = pstTmp;
		pstTmp = pstTmp->pNextMalloc;
		free( pstTmp2 );
	}

	return;
}
