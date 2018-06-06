#ifndef _UTIL_H_
#define _UTIL_H_

/*
 * 定数定義
 */


/*
 * 外部宣言
 */
extern void logString( char *p );
extern void logDecVal( char *pszPrefix, DWORD dwVal );
extern void logHexVal( char *pszPrefix, DWORD dwVal );
extern void logBitVal( char *pszPrefix, DWORD dwVal, BYTE bPadding );
extern void logPrint( char *fmt, ... );
extern DWORD bit2val( char *p );

#endif
