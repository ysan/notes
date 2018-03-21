#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "mycommon.h"
#include "ntpmoni.h"
#include "util.h"


/**
 * double小数値->32bit小数のみ固定小数点変換
 * 正の数のみ対応
 */
uint32_t Double2FixedOnlyFraction( double arg )
{
	if ( (arg >= 1) || (arg < 0) ) {
		return 0;
	}

	uint32_t nRtn = 0;
	double nLocal = 0;
	UN_DOUBLE *punDouble = NULL;

	nLocal = arg + 1.0; /* 1.xxx  の形に */
	punDouble = (UN_DOUBLE*)&nLocal;
	nRtn = punDouble->ST_DOUBLE.fraction >> 20; /* 上位32bit分取得 */

	return nRtn;
}

/**
 * 32bit小数のみ固定小数点->double小数値変換
 * 正の数のみ対応
 */
double Fixed2DoubleOnlyFraction( uint32_t arg )
{
	if ( arg == 0 ) {
		return 0;
	}

	double nRtn = 1.0; /* 予め 1.xxx  の形に */
	UN_DOUBLE *punDouble = NULL;

	punDouble = (UN_DOUBLE*)&nRtn;
	punDouble->ST_DOUBLE.fraction = ((unsigned long long)arg) << 20; /* 上位32bit分に入れる */

	return nRtn -1.0; /* 1.0を引く */
}

/**
 * サーバリスト取得
 * 使用後はDestroyServerList()を呼ぶこと
 */
struct serv_info *GetServerList( void )
{
	int i = 0;
	int nTotal = 0;
	char szTmp[ NTP_SERVER_NAME_LEN ];
	char szTmp2[ IPV4_ADDR_STR_LEN ];
	FILE *pFp = NULL;
	struct serv_info *pstServInfoSta = NULL;


	if ( !( pFp = fopen( CONF_FILE_PATH, "r" ) ) ) {
	    PERROR( "fopen()" );
		return NULL;
	}

	/* 総レコード数を取得 */
	while ( fgetc( pFp ) != EOF ) {

		/* fgetc()の分をもどす */
		if ( fseek( pFp, -1, SEEK_CUR ) ) {
			PERROR( "fseek()" );
			return NULL;
		}

		memset( szTmp, 0x00, sizeof(szTmp) );
		fgets( szTmp, NTP_SERVER_NAME_LEN -1, pFp );
		if ( ferror( pFp ) ) {
			LOG_E( "fgets() is failure.\n" );
			return NULL;
		}

		nTotal ++;
	}

	LOG_I( "TotalRecords=[%d]\n", nTotal );

	/* FP位置を初期化 */
	if ( fseek( pFp, 0, SEEK_SET ) ) {
		PERROR( "fseek()" );
		return NULL;
	}

	/* malloc */
	pstServInfoSta = (struct serv_info*)malloc( sizeof(struct serv_info)*nTotal );
	if ( !pstServInfoSta ) {
		PERROR( "malloc()" );
		return NULL;
	}

	memset( pstServInfoSta, 0x00, sizeof(struct serv_info)*nTotal );
 
	/* get server list */
	while ( fgetc( pFp ) != EOF ) {

		/* fgetc()の分をもどす */
		if ( fseek( pFp, -1, SEEK_CUR ) ) {
			PERROR( "fseek()" );
			return NULL;
		}

		memset( szTmp, 0x00, sizeof(szTmp) );
		fgets( szTmp, NTP_SERVER_NAME_LEN, pFp );
		if ( ferror( pFp ) ) {
			LOG_E( "fgets() is failure.\n" );
			return NULL;
		}

		/* 改行削除 */
		DeleteLF( szTmp );

		/* 行頭,行末スペース削除 */
		DeleteHeadSp( szTmp );
		DeleteTailSp( szTmp );


		/* 空行は無視 */
		if ( szTmp[0] == 0x00 ) {
			nTotal --;
			LOG_W( "null line. TotalRecords=[%d]\n", nTotal );
			if ( nTotal == i ) {
				(pstServInfoSta+i)->isInitTerm = TRUE;
			}
			continue;
		}

		/* 行頭'#'はコメント */
		if ( szTmp[0] == '#' ) {
			nTotal --;
			LOG_W( "comment line. TotalRecords=[%d]\n", nTotal );
			if ( nTotal == i ) {
				(pstServInfoSta+i)->isInitTerm = TRUE;
			}
			continue;
		}


		/* impliments */

		(pstServInfoSta+i)->enState = EN_STATE_INIT;
		(pstServInfoSta+i)->isInitValid = TRUE;
		(pstServInfoSta+i)->isInitTerm = FALSE;

		memcpy( (pstServInfoSta+i)->szName, szTmp, NTP_SERVER_NAME_LEN -1 );

		if ( GetIpAddr( (const char*)szTmp, &((pstServInfoSta+i)->nAddr) ) < 0 ) {
			LOG_E( "GetIpAddr() is failure.\n" );
			LOG_W( "continue...\n" );
			(pstServInfoSta+i)->isInitValid = FALSE;

		} else {
//			/* DNS引けた時だけソケット生成する */
//
//			if ( ( (pstServInfoSta+i)->nFdSock = CreateSocket() ) < 0 ) {
//				LOG_E( "CreateSocket() is failure.\n" );
//				LOG_W( "continue...\n" );
//				(pstServInfoSta+i)->isInitValid = FALSE;
//			}
		}

		if ( i >= nTotal -1 ) {
			(pstServInfoSta+i)->isInitTerm = TRUE;
		}


		/* for log */
		memset( szTmp2, 0x00, sizeof(szTmp2) );
		if ( !inet_ntop( AF_INET, &((pstServInfoSta+i)->nAddr), szTmp2, IPV4_ADDR_STR_LEN -1 ) ) {
			PERROR( "inet_ntop()" );
		}

		LOG_I (
			"idx:[%d] isInitValid:[%d] isInitTerm:[%d] fd:[%d] ip:[%s(%s)]\n",
			i,
			(pstServInfoSta+i)->isInitValid,
			(pstServInfoSta+i)->isInitTerm,
			(pstServInfoSta+i)->nFdSock,
			szTmp2,
			(pstServInfoSta+i)->szName
		);

		i ++;
	}

	fclose( pFp );

	return pstServInfoSta;
}

/**
 * サーバリスト解放
 */
void DestroyServerList( struct serv_info *p )
{
	if ( !p ) {
		LOG_E( "Argument is null.\n" );
		return;
	}

	int i = 0;

	while (1) {

		/* socket close */
		if ( (p+i)->isInitValid ) {
			close( (p+i)->nFdSock );
		}

		if ( (p+i)->isInitTerm ) {
			break;
		}

		i ++;
	}

	free( p );
	p = NULL;
}
