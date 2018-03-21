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
 * リクエストメッセージ内容を確認
 */
int CheckRequestMessage( const char *pszBuff, int nRecvSize, struct request_msg *pstrReqMsg )
{
	int nRtn = 0;
	int i = 0;
	int nLine = 0;
	int nLineTmp = 0;
	const char *pszPnt[CHK_REQMSG_LINE] = { NULL };
	char szTmp[16];


	/* 各行の先頭にポインタを付ける (CHK_REQMSG_LINE行数まで) */
	/* RFCでは 1つ以上の SP や HT をそれぞれの行頭につける事で
	   複数行にまたがる事ができるとの事  これには未対応... */
	while( nRecvSize > i ){
		if( nLine >= CHK_REQMSG_LINE -1 ){
			fprintf( stderr, "Error: Request Message exceeded [%d] lines.\n", CHK_REQMSG_LINE );
			return -1;
		}

		if( !i ){
			/* 1文字目 */

			pszPnt[nLine] = pszBuff;
			nLine ++;

		} else {
			/* 2文字目以降 */

			/* LFの次の文字にポインタを付ける */
			if( *(pszBuff+i) == '\n' ){
				pszPnt[nLine] = pszBuff+i+1;
				nLine ++;
			}
		}

		i ++;
	}


	/* 1行目リクエスト行  [メソッド],[パス],[HTTPバージョン] を取得 */
	/* パスは255bytesまで制限チェックすべき */
	nRtn = sscanf( pszPnt[0], "%s %s %s",
						pstrReqMsg->m_szMethod, pstrReqMsg->m_szPath, pstrReqMsg->m_szHttpVer );
	if( nRtn != 3 ){
		return -2;
	} else if( nRtn == EOF ){
		perror( "sscanf()" );
		return -1;
	}

	nLineTmp = 1;

	/* 2行目以降 ヘッダの内容を確認 */
	/* (nLineは1回余分にカウントされている) */
	while( (nLine -1) >= nLineTmp ){

//printf("[%s]\n",pszPnt[nLineTmp]);

		if( !CompUpperString( pszPnt[nLineTmp], "HOST:" ) ){
			nRtn = GetHeaderField( pszPnt[nLineTmp], pstrReqMsg->m_szHost );
			if( nRtn < 0 ){
				return -3;
			}
//printf("[%s]\n",pstrReqMsg->m_szHost);
		}

		if( !CompUpperString( pszPnt[nLineTmp], "CONNECTION:" ) ){
			nRtn = GetHeaderField( pszPnt[nLineTmp], pstrReqMsg->m_szConnection );
			if( nRtn < 0 ){
				return -3;
			}
//printf("[%s]\n",pstrReqMsg->m_szConnection);
		}

		if( !CompUpperString( pszPnt[nLineTmp], "CONTENT-LENGTH:" ) ){
			memset( szTmp, 0x00, sizeof(szTmp) );
			nRtn = GetHeaderField( pszPnt[nLineTmp], szTmp );
			if( nRtn < 0 ){
				return -3;
			}
			/* 数値であるかチェックすべき */
			pstrReqMsg->m_nContentLength = atoi( szTmp );
//printf("[%d]\n",pstrReqMsg->m_nContentLength);
		}


		nLineTmp ++;
	}


	return 0;
}

/*
 * ヘッダフィールド値の取得
 */
int GetHeaderField( const char *pszBuff, char *pszRtn )
{
	int i = 0;
	int nFindFlag = 0;

	/* 1行分 */
	while(( *pszBuff != C_CR )&&( *pszBuff != C_LF )){
		if( *(pszBuff-1) == ':' ){
			nFindFlag = 1;
		}

		/* スペースは読み飛ばす */
		if(( nFindFlag )&&( *pszBuff != ' ' )){
			*pszRtn = *pszBuff;
			pszRtn ++;
			i ++;
		}

		pszBuff ++;
	}

	/* 何も得られなかった */
	if( !i ){
		return -1;
	}

	return 0;
}

/*
 * 一旦大文字にして文字列比較 (行頭から)
 */
int CompUpperString( const char *pszBuff, const char *pszComp )
{
	size_t nCompSize = strlen(pszComp);

	while(1){
		if( toupper(*pszBuff) != *pszComp ){
			return -1;
		}

		/* 1行分 */ 
		if(( *pszBuff == C_CR )||( *pszBuff == C_LF )){
			break;
		}

		if( nCompSize == 1 ){
			break;
		}

		pszBuff ++;
		pszComp ++;
		nCompSize --;
	}

	return 0;
}

/*
 * POSTのリクエストボディを取得
 */
int GetPostMethod(
	const char *pszBuff,
	int nRecvSize,
	int nContentLength,
	char **pszReqBdy,
	int *pnCpbyte
)
{
	int i = 0;
	const char *pszBodyStat = NULL;

	/* リクエストボディ先頭を探す */

	/* 改行コードが CRLFの場合 */
	if( ( pszBodyStat = strstr( pszBuff, CRLFCRLF ) ) ){
		pszBodyStat += 4;
	} else {
		/* 改行コードが LFの場合 */
		if( ( pszBodyStat = strstr(pszBuff, LFLF ) ) ){
			pszBodyStat += 2;
		} else {
			/* みつからない */
			pszBodyStat = pszBuff;
		}
	}


	/* pszBodyStatはリクエストボディの先頭に位置する */


	/* Content-Length分の領域確保 */
	*pszReqBdy = (char*)malloc( nContentLength+1 );
	if( !(*pszReqBdy) ){
		perror( "malloc()" );
		return -1;
	}

	memset( *pszReqBdy, 0x00, nContentLength+1 );

	i = 0;
	/* リクエストボディにまだ続きがあるか確認 */
	while( *(pszBodyStat+i) ){
		i ++;
	}

	if( i < nContentLength ){
		/* まだ続きがある */

		if( i > 0 ){
			/* 確保した領域に現状受信できているものをコピー */
			memcpy( *pszReqBdy, pszBodyStat, i );
			*pnCpbyte = i;
		}

		/* 戻って続きを受信する */
		return -2;

	} else if( i == nContentLength ){
		/* リクエストボディは全て受信されている */
		/* 確保した領域全てにコピー */
		memcpy( *pszReqBdy, pszBodyStat, nContentLength );
		*pnCpbyte = nContentLength;
	}


	return 0;
}
