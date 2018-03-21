#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>
#include <linux/limits.h>

#include "mycommon.h"
#include "http_defines.h"
#include "hserv.h"


/*
 * 拡張子を調べ Content-Typeを決める
 */
int ReturnContentType( const char *pszBuff, int nResStat )
{
	if(( nResStat >= RESPONSE_STATUS_200 )&&( nResStat <= RESPONSE_STATUS_206 )){
		/* 正常系 */

		if( strstr( pszBuff+strlen(pszBuff)-5, ".html" ) ){
			return CONTENT_TYPE_TEXT_HTML;
		}

		if( strstr( pszBuff+strlen(pszBuff)-4, ".txt" ) ){
			return CONTENT_TYPE_TEXT_PLAIN;
		}

		if( strstr( pszBuff+strlen(pszBuff)-4, ".xml" ) ){
			return CONTENT_TYPE_TEXT_XML;
		}

		if( strstr( pszBuff+strlen(pszBuff)-3, ".js" ) ){
			return CONTENT_TYPE_TEXT_JAVASCRIPT;
		}

		if( strstr( pszBuff+strlen(pszBuff)-4, ".gif" ) ){
			return CONTENT_TYPE_IMAGE_GIF;
		}

		if( strstr( pszBuff+strlen(pszBuff)-4, ".jpg" ) ){
			return CONTENT_TYPE_IMAGE_JPG;
		}

		if( strstr( pszBuff+strlen(pszBuff)-4, ".png" ) ){
			return CONTENT_TYPE_IMAGE_PNG;
		}




		if( strstr( pszBuff+strlen(pszBuff)-4, ".cgi" ) ){
			return CONTENT_TYPE_CGI;
		}

		if( strstr( pszBuff+strlen(pszBuff)-3, ".pl" ) ){
			return CONTENT_TYPE_CGI;
		}

		if( strstr( pszBuff+strlen(pszBuff)-4, ".ico" ) ){
			return CONTENT_TYPE_IMAGE_ICO;
		}

		/* 何も該当しない */
		return -1;

	} else {
		/* エラー系 */
		return CONTENT_TYPE_TEXT_HTML;
	}

}

/*
 * ファイルオープン
 */
int OpenFile( char *pszBuff, int nResStat )
{
	int nFd = 0;

	if(( nResStat >= RESPONSE_STATUS_200 )&&( nResStat <= RESPONSE_STATUS_206 )){
		/* 正常系 */
		;
	} else {
		/* エラー系 */
		/* コンテンツのベースディレクトリ付加 */
		snprintf( pszBuff, PATH_MAX, "%s%s", CONTENTS_BASE_DIR, g_pszResponseStatus[nResStat][1] );

	}

	/* ファイルオープン */
	nFd = open( pszBuff, O_RDONLY, 0666 );
	if( nFd < 0 ){
		perror( "open()" );
		return -1;
	}

	return nFd;
}

/*
 * レスポンスメッセージ作成
 */
void CreateResponseMessage(
	int nResStat,
	const struct response_element *pstrResElmnt,
	char *pszRtnBuff,
	size_t nRtnBuffSize
)
{
	char szBuff[256];

	memset( szBuff, 0x00, sizeof(szBuff) );

	/* レスポンス行 */
	strncat( pszRtnBuff, HTTP_VER, nRtnBuffSize-1 );
	strncat( pszRtnBuff, SP, nRtnBuffSize-1 );
	strncat( pszRtnBuff, g_pszResponseStatus[nResStat][0], nRtnBuffSize-1 );
	strncat( pszRtnBuff, CRLF, nRtnBuffSize-1 );

	/* 以下ヘッダ部 */

	/* Content-Type */
	if( pstrResElmnt->m_nContentType != CONTENT_TYPE_CGI ){

		memset( szBuff, 0x00, sizeof(szBuff) );
		snprintf( szBuff, sizeof(szBuff), CONTENT_TYPE, g_pszContentType[pstrResElmnt->m_nContentType] );
		strncat( pszRtnBuff, szBuff, nRtnBuffSize-1 );

		/* text形式であればcharsetも指定 */
		if(( pstrResElmnt->m_nContentType >= CONTENT_TYPE_TEXT_HTML )&&
		   ( pstrResElmnt->m_nContentType <= CONTENT_TYPE_TEXT_XML)){
			memset( szBuff, 0x00, sizeof(szBuff) );
			snprintf( szBuff, sizeof(szBuff), CHARSET, g_pszCharset[pstrResElmnt->m_nCharset] );
			strncat( pszRtnBuff, szBuff, nRtnBuffSize-1 );
		}

		strncat( pszRtnBuff, CRLF, nRtnBuffSize-1 );
	}

	/* Content-Length */
	memset( szBuff, 0x00, sizeof(szBuff) );
	snprintf( szBuff, sizeof(szBuff), CONTENT_LENGTH, pstrResElmnt->m_nContentLength );
	strncat( pszRtnBuff, szBuff, nRtnBuffSize-1 );
	strncat( pszRtnBuff, CRLF, nRtnBuffSize-1 );

	/* Connection */
	memset( szBuff, 0x00, sizeof(szBuff) );
	snprintf( szBuff, sizeof(szBuff), CONNECTION, g_pszConnection[pstrResElmnt->m_nConnection] );
	strncat( pszRtnBuff, szBuff, nRtnBuffSize-1 );
	strncat( pszRtnBuff, CRLF, nRtnBuffSize-1 );

	/* Keep-Alive */
	memset( szBuff, 0x00, sizeof(szBuff) );
	snprintf( szBuff, sizeof(szBuff), KEEP_ALIVE, KEEP_ALIVE_TIMEOUT, pstrResElmnt->m_nKeepAlive );
	strncat( pszRtnBuff, szBuff, nRtnBuffSize-1 );
	strncat( pszRtnBuff, CRLF, nRtnBuffSize-1 );


	/* 最後の改行コード */
	if( pstrResElmnt->m_nContentType != CONTENT_TYPE_CGI ){
		strncat( pszRtnBuff, CRLF, nRtnBuffSize-1 );
	}

	return;
}


