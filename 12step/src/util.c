#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"

#include "defines.h"
#include "util.h"
#include "sci.h"


#define UINT_MAX_DEC_STR_SIZE	(10+1)
#define INT_MAX_DEC_STR_SIZE	(1+10+1)

#define INT_MAX_HEX_STR_SIZE	(8+1)

#define BIT_STR_SIZE			(32+1)

#define LOG_TOTAL_BUFF_SIZE		(64+1)
#define LOG_WORK_BUFF_SIZE		(32+1)
#define LOG_PARAM_BUFF_SIZE		(10+1)


typedef struct {
	BYTE bPadNum;	// 255まで注意
	BOOL isZeroPad;
} ST_CHKPAD_RSLT;


/*
 * 変数宣言
 */


/*
 * プロトタイプ宣言
 */
void logString( char *p ); // extern
void logDecVal( char *pszPrefix, DWORD dwVal ); // extern
void logHexVal( char *pszPrefix, DWORD dwVal ); // extern
void logBitVal( char *pszPrefix, DWORD dwVal, BYTE bPadding ); // extern
static DWORD ex10( BYTE i );
static BOOL uint2decStr( DWORD dwVal, char *pDst, BYTE bSize, BYTE bPadding );
static BOOL int2hexStr( DWORD dwVal, char *pDst, BYTE bSize, BYTE bPadding );
static BOOL int2bitStr( DWORD dwVal, char *pDst, BYTE bSize, BYTE bPadding );
static void dump( BYTE *p, DWORD nBuffLen );
static ST_CHKPAD_RSLT checkPad( char *p );
static void padString( char *pDst, WORD wSize, BYTE bPad, char c );
static void cashChar( char *pDst, WORD wSize, char c );
static void cashString( char *pDst, WORD wSize, char *pSrc );
void logPrint( char *fmt, ... ); // extern
DWORD bit2val( char *p ); //extern


/**
 * シリアルログ出力
 * 文字列
 */
void logString( char *p )
{
	if (p) {
		sendSequenceSci1( (BYTE*)p, strlen(p) );
	}
}

/**
 * シリアルログ出力
 * unsigned int -> 10進数文字列 
 */
void logDecVal( char *pszPrefix, DWORD dwVal )
{
	if (pszPrefix) {
		sendSequenceSci1( (BYTE*)pszPrefix, strlen(pszPrefix) );
	}

	char szWork[ UINT_MAX_DEC_STR_SIZE ];
	memset( szWork, 0x00, UINT_MAX_DEC_STR_SIZE );
	uint2decStr( dwVal, szWork, UINT_MAX_DEC_STR_SIZE, 0 );
	sendSequenceSci1( (BYTE*)szWork, strlen(szWork) );

	sendByteSci1( (BYTE)'\n' );
}

/**
 * シリアルログ出力
 * unsigned int -> 16進数文字列
 */
void logHexVal( char *pszPrefix, DWORD dwVal )
{
	if (pszPrefix) {
		sendSequenceSci1( (BYTE*)pszPrefix, strlen(pszPrefix) );
	}

	char szWork[ INT_MAX_HEX_STR_SIZE ];
	memset( szWork, 0x00, INT_MAX_HEX_STR_SIZE );
	int2hexStr( dwVal, szWork, INT_MAX_HEX_STR_SIZE, 0 );
	sendSequenceSci1( (BYTE*)szWork, strlen(szWork) );

	sendByteSci1( (BYTE)'\n' );
}

/**
 * シリアルログ出力
 * unsigned int -> 2進数文字列
 */
void logBitVal( char *pszPrefix, DWORD dwVal, BYTE bPadding )
{
	if (pszPrefix) {
		sendSequenceSci1( (BYTE*)pszPrefix, strlen(pszPrefix) );
	}

	char szWork[ BIT_STR_SIZE ];
	memset( szWork, 0x00, BIT_STR_SIZE );
	int2bitStr( dwVal, szWork, BIT_STR_SIZE, bPadding );
	sendSequenceSci1( (BYTE*)szWork, strlen(szWork) );

	sendByteSci1( (BYTE)'\n' );
}

/**
 * 10のべき乗を返す (正の数のみ)
 * UINT_MAX 4294967295 以内で 9乗までです
 * 引数が 9 より大きく指定されたら 9でまるめます
 */
static DWORD ex10( BYTE i )
{
	if (i == 0) {
		return 1;
	} else if (i > 9) {
		i = 9;
	}

	DWORD rtn = 10;
	BYTE n;

	for (n = 1; n < i; n ++) {
		rtn *= 10;
	}

	return rtn;
}

/**
 * unsigned int(4byte) -> 10進数文字列変換
 * 0パディングしない場合は bPadding=0で指定する
 */
static BOOL uint2decStr( DWORD dwVal, char *pDst, BYTE bSize, BYTE bPadding )
{
	if ((bSize < UINT_MAX_DEC_STR_SIZE) || (!pDst)) {
		return FALSE;
	}

	BYTE n = 9;
	BYTE m = 0;
	BYTE a = 0;
	BYTE nOffset = 0;
	BOOL isFound = FALSE;

//TODO signed / unsigned共通化
	/* for signed int */
//	if (dwVal < 0) {
//		dwVal *= -1;
//		*pDst = '-';
//		nOffset = 1;
//	}

	while ((n >= 0) && (n <= 9)) {
		a = dwVal / ex10(n);

		if ((a > 0) && !isFound) {
			isFound = TRUE;
			m = n;
		}

		if (isFound) {
			*(pDst + m-n + nOffset) = '0' + a;
		}

		dwVal = dwVal % ex10(n);
		n --;
	}

	if (!isFound) {
		*pDst = '0';
	}

	return TRUE;
}

/**
 * int(4byte) -> 16進数文字列変換
 * signed/unsigned 共通
 * 0パディングしない場合は bPadding=0で指定する
 */
static BOOL int2hexStr( DWORD dwVal, char *pDst, BYTE bSize, BYTE bPadding )
{
	if ((bSize < INT_MAX_HEX_STR_SIZE) || (!pDst)) {
		return FALSE;
	}

	if (bPadding > 8) {
		bPadding = 8;
	} else if (bPadding < 0) {
		bPadding = 0;
	}

	BYTE i = 0;
	BYTE j = 0;
	BYTE n = 0;
	BYTE bShift = 32; // 32bit
	BOOL isFound = FALSE;
	char szWork[ INT_MAX_HEX_STR_SIZE ] = { 0x00 };

	while (i < 8) {
		bShift -= 4;
		n = (dwVal >> bShift) & 0xf;

		if ((n > 0) && !isFound) {
			isFound = TRUE;
		}

		if (isFound) {
			szWork[ j ] = n < 10 ? '0'+n : 'a'+n-10;
			j ++;
		}

		i ++;
	}

	if (isFound) {
		if (j < bPadding) {
			memset( pDst, '0', bPadding - j );
			pDst += bPadding - j;
			strncpy( pDst, szWork, INT_MAX_HEX_STR_SIZE - (bPadding - j) -1 );

		} else {
			strncpy( pDst, szWork, INT_MAX_HEX_STR_SIZE -1 );
		}

	} else {
		*pDst = '0';
	}

	return TRUE;
}

/**
 * int(4byte) -> 2進数文字列変換
 * signed/unsigned 共通
 * 0パディングしない場合は bPadding=0で指定する
 */
static BOOL int2bitStr( DWORD dwVal, char *pDst, BYTE bSize, BYTE bPadding )
{
	if ((bSize < BIT_STR_SIZE) || (!pDst)) {
		return FALSE;
	}

	if (bPadding > 32) {
		bPadding = 32;
	} else if (bPadding < 0) {
		bPadding = 0;
	}

	BYTE i = 32;
	BOOL isFound = FALSE;
	char c;

	while ((i > 0) && (i <= 32)) {

		c = ((dwVal >> (i-1)) & 0x01) + '0';

		if ((c == '1') && (!isFound)) {
			isFound = TRUE;
		}

		if (bPadding >= i) {
			*pDst = c;
			pDst ++;

		} else {
			if (isFound) {
				*pDst = c;
				pDst ++;
			}
		}

		i --;
	}

	return TRUE;
}

struct dump16 {
	BYTE n1;
	BYTE n2;
	BYTE n3;
	BYTE n4;
	BYTE n5;
	BYTE n6;
	BYTE n7;
	BYTE n8;
	BYTE n9;
	BYTE n10;
	BYTE n11;
	BYTE n12;
	BYTE n13;
	BYTE n14;
	BYTE n15;
	BYTE n16;
};

/**
 * 16進ダンプ
 */
static void dump( BYTE *p, DWORD dwLen )
{
	if ((!p) || (dwLen <= 0)) {
		return;
	}

	DWORD i = 0;
	DWORD j = 0;
	DWORD k = 0;
	struct dump16 *pstDump16 = NULL;



}

/**
 * paddingの数チェック
 * 引数p には0~9の文字しかこない前提
 */
static ST_CHKPAD_RSLT checkPad( char *p )
{
	ST_CHKPAD_RSLT stRslt = { 0, FALSE };

	if (!p) {
		return stRslt;
	}

	BOOL isFirst = TRUE;
	BYTE t = 0;

	while (strlen(p) > 0) {

		if (isFirst) {
			isFirst = FALSE;
			if (*p == '0') {
				stRslt.isZeroPad = TRUE;
				t = strlen(p+1);
				p ++;
				continue;
			} else {
				t = strlen(p);
			}
		}

		if (*p == '0') {
			// 捨てる
		} else {
			// t に桁数がはいってる (1始まりに注意)
			// 各桁の数を加算する
			stRslt.bPadNum += (*p -'0') * ex10( t -1 );
		}

		t --;
		p ++;
	}

	return stRslt;
}

/**
 * パディンク処理
 * サイズを超えた時は切り詰める
 * 正負対応 (%10s / %-10s)
 */
static void padString( char *pDst, WORD wSize, BYTE bPad, char c )
{
	BOOL isPosi = TRUE;
	if (bPad < 0) {
		isPosi = FALSE;
		bPad *= -1;
	}

	if (strlen(pDst) >= bPad) {
		return;
	}

	if (wSize <= bPad) {
		bPad = wSize -1;
	}

	int n = bPad - strlen(pDst);

	if (isPosi) {
		memcpy( pDst+n, pDst, strlen(pDst) );
		memset( pDst, c, n );
		*(pDst + strlen(pDst)) = 0x00;
	} else {
		memset( pDst + strlen(pDst), c, n );
		*(pDst + strlen(pDst)) = 0x00;
	}
}

/**
 * dstの後ろにキャッシュ
 */
static void cashChar( char *pDst, WORD wSize, char c )
{
	if (!pDst) {
		return;
	}
	if (strlen(pDst)+1 >= wSize-1) {
		// buffer over
		return;
	}
	*(pDst + strlen(pDst)) = c;
}

/**
 * dstの後ろにキャッシュ
 */
static void cashString( char *pDst, WORD wSize, char *pSrc )
{
	if ((!pDst) || (!pSrc)) {
		return;
	}
	if (strlen(pDst)+strlen(pSrc) >= wSize-1) {
		// buffer over
		return;
	}
	strcpy(pDst + strlen(pDst), pSrc);
}

/**
 * 簡易printf
 * 対応形式
 *   %s, %d, %x, %b (拡張:bit表記)
 *
 * padding対応
 *   %5s %03d %4x %010b などに対応
 *   %-5s は%5s同じとなる マイナスは未対応(無視する)
 *   %05s は%5s同じとなる 文字列表示のときは 0padding無視する
 *
 *   %ngs となった時 ngは無視して %s を適用する
 *   %1n3gs となった時 ngは無視して %13s を適用する
 *
 * 内部バッファ
 *   1引数で32byteまで表示
 *   paddingは10byteまで読み込む
 *   全体で64byteまで表示
 *   これら越えた分は捨てられる
 */
void logPrint( char *fmt, ... )
{
	va_list args;
	DWORD d;
	char c;
	char *s;
	char szBuff[ LOG_TOTAL_BUFF_SIZE ] = { 0x00 };
	char szWork[ LOG_WORK_BUFF_SIZE ] = { 0x00 };
	char szPad[ LOG_PARAM_BUFF_SIZE ] = { 0x00 };
	BOOL isParam = FALSE;
	ST_CHKPAD_RSLT stRslt;

	va_start( args, fmt );

	while (strlen(fmt) > 0) {

		switch (*fmt) {
		case '%':
			if (isParam) {
				cashChar( szBuff, sizeof(szBuff), *fmt );
				isParam = FALSE;
			} else {
				isParam = TRUE;
			}
			break;

		case 's':
			if (!isParam) {
				cashChar( szBuff, sizeof(szBuff), *fmt );
			} else {
				s = va_arg( args, char* );
//printf( "string %s¥n", s );
				strncpy( szWork, s, sizeof(szWork)-1 );

				// padding処理
				stRslt = checkPad( szPad );
				padString( szWork, sizeof(szWork), stRslt.bPadNum, ' ' ); // 文字列はisZeroPadは無視

				cashString( szBuff, sizeof(szBuff), szWork );

				// clear
				memset( szWork, 0x00, sizeof(szWork) );
				memset( szPad, 0x00, sizeof(szPad) );
				isParam = FALSE;
			}
			break;

		case 'd':
		case 'x':
		case 'b':
			if (!isParam) {
				cashChar( szBuff, sizeof(szBuff), *fmt );
			} else {
				d = va_arg( args, int ); // intじゃないとだめだった...?
//printf( "int %d¥n", d );
				switch (*fmt) {
				case 'd':
					uint2decStr( d, szWork, sizeof(szWork), 0 ); // ここはpadding指定しない
					break;
				case 'x':
					int2hexStr( d, szWork, sizeof(szWork), 0 ); // ここはpadding指定しない
					break;
				case 'b':
					int2bitStr( d, szWork, sizeof(szWork), 0 ); // ここはpadding指定しない
					break;
				default:
					break;
				}

				// padding処理
				stRslt = checkPad( szPad );
				padString( szWork, sizeof(szWork), stRslt.bPadNum, stRslt.isZeroPad ? '0' : ' ' );

				cashString( szBuff, sizeof(szBuff), szWork );

				// clear
				memset( szWork, 0x00, sizeof(szWork) );
				memset( szPad, 0x00, sizeof(szPad) );
				isParam = FALSE;
			}
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (isParam) {
				cashChar( szPad, sizeof(szPad), *fmt );
			} else {
				cashChar( szBuff, sizeof(szBuff), *fmt );
			}
			break;

		default:
			if (isParam) {
				// not supported
				// 捨てる
			} else {
				cashChar( szBuff, sizeof(szBuff), *fmt );
			}
			break;
		}

		fmt ++;
	}

	va_end( args );

	sendSequenceSci1( (BYTE*)szBuff, strlen(szBuff) );
}

/**
 * 2進数文字列->数値変換
 */
DWORD bit2val( char *p )
{
	if (!p) {
		return 0;
	}

	char *pTmp = p;
	BYTE bPos = 0;
	BYTE bLen = 0;
	DWORD dwVal = 0;


	/* 数値チェック */
	while (*p) {
		if ((*p != '0') && (*p != '1')) {
			/* error */
			return 0;
		}
		p ++;
		bLen ++;
	}

	/* 下位32bit分のみを変換対象とする */
	if (bLen >= 32) {
		bLen = 32 -1;
	}

	p = pTmp;

	bLen --;
	bPos = 0;
	while ((bLen >= 0) && (bLen < 32)) {
		dwVal |= (*(p+bLen)-'0') << bPos;
		bPos ++;
		bLen --;
	}

	return dwVal;
}
