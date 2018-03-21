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
 * 送受信 (HTTP)
 */
int RecvSend( int nFdSockCl, int *pnFlagKeepAlive )
{
	int nFd = 0;
	int nRtn = 0;
	int nFlag = 0;
	int nReadSize = 0;
	int nRecvSize = 0;
	int nResStat = 0;
	int nCpbyte = 0;
	int nContLen = 0;
	int nSendLen = 0;
//	char szReqMsg[1024*2];
	char szReqMsg[65536];
	char szResMsg[1024];
	unsigned char szSendBuff[65536];
	char szResStat[64];
	char szEnvNum[10];
	char szFilePathName[PATH_MAX];
	struct request_msg strReqMsg;
	struct response_element strResElmnt;
	struct stat strStat;


	memset( szReqMsg, 0x00, sizeof(szReqMsg) );
	memset( szResMsg, 0x00, sizeof(szResMsg) );
	memset( szSendBuff, 0x00, sizeof(szSendBuff) );
	memset( szResStat, 0x00, sizeof(szResStat) );
	memset( szEnvNum, 0x00, sizeof(szEnvNum) );
	memset( szFilePathName, 0x00, sizeof(szFilePathName) );
	memset( &strReqMsg, 0x00, sizeof(strReqMsg) );
	memset( &strResElmnt, 0x00, sizeof(strResElmnt) );
	memset( &strStat, 0x00, sizeof(strStat) );


	/* 200 OK を設定 */
	nResStat = RESPONSE_STATUS_200;


	/* クライアントからのリクエストメッセージの受信 */
	nRecvSize = RecvDataHttpReq( nFdSockCl, szReqMsg, sizeof(szReqMsg), &nFlag );
	if( nRecvSize == -1 ){
		fprintf( stderr, "Error: RecvDataHttpReq() is failure. nRecvSize=[%d]\n", nRecvSize );
		/* エラー 500 Internal Server Error を返送 */
		nResStat = RESPONSE_STATUS_500;
		strResElmnt.m_nConnection = CONNECTION_CLOSE;
		strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
		(*pnFlagKeepAlive) = 0;
		goto OPENFILE;

	} else if( nRecvSize == -2 ){
		fprintf( stderr, "Error: RecvDataHttpReq() is failure. nRecvSize=[%d]\n", nRecvSize );
		/* バッファオーバー 400 Bad Request を返送 */
		nResStat = RESPONSE_STATUS_400;
		strResElmnt.m_nConnection = CONNECTION_CLOSE;
		strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
		(*pnFlagKeepAlive) = 0;
		goto OPENFILE;
	}

	fprintf( stdout, "[%d]bytes received.\n", nRecvSize );

	if( nFlag ){
		/* 0 byte received */
		fprintf( stdout, "Connection with a partner went out...\n" );
		return -2;
	}
puts( szReqMsg );



	/* リクエストメッセージ内容を確認 */
	nRtn = CheckRequestMessage( szReqMsg, nRecvSize, &strReqMsg );
	if( nRtn == -1 ){
		fprintf( stderr, "Error: CheckRequestMessage() is failure. nRtn=[%d]\n", nRtn );
		/* エラー 500 Internal Server Error を返送 */
		nResStat = RESPONSE_STATUS_500;
		strResElmnt.m_nConnection = CONNECTION_CLOSE;
		strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
		(*pnFlagKeepAlive) = 0;
		goto OPENFILE;

	} else if( nRtn == -2 ){
		fprintf( stderr, "Error: CheckRequestMessage() is failure. nRtn=[%d]\n", nRtn );
		/* リクエスト行の分解失敗時 501 Not Implemented を返送 */
		nResStat = RESPONSE_STATUS_501;
		strResElmnt.m_nConnection = CONNECTION_CLOSE;
		strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
		(*pnFlagKeepAlive) = 0;
		goto OPENFILE;

	} else if( nRtn == -3 ){
		fprintf( stderr, "Error: CheckRequestMessage() is failure. nRtn=[%d]\n", nRtn );
		/* ヘッダ部の形式がおかしい 400 Bad Request を返送 */
		nResStat = RESPONSE_STATUS_400;
		strResElmnt.m_nConnection = CONNECTION_CLOSE;
		strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
		(*pnFlagKeepAlive) = 0;
		goto OPENFILE;
	}


	/* GETとPOST以外実装していない... 暫定 */
	if( strcmp( strReqMsg.m_szMethod, "GET" ) && strcmp( strReqMsg.m_szMethod, "POST" ) ){
		/* 501 Not Implemented を返送 */
		nResStat = RESPONSE_STATUS_501;
		strResElmnt.m_nConnection = CONNECTION_CLOSE;
		strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
		(*pnFlagKeepAlive) = 0;
		goto OPENFILE;
	}


/* GETのcgi等の場合 &で連結されたパスとデータ部の分割必要 */


	if(( !strcmp( strReqMsg.m_szMethod, "POST" ) )&&( strReqMsg.m_nContentLength )){

		/* POSTのリクエストボディを取得 */
		nRtn = GetPostMethod( szReqMsg, nRecvSize, strReqMsg.m_nContentLength,
									&strReqMsg.m_pszReqBdy, &nCpbyte );
		if( nRtn == -1 ){
			fprintf( stderr, "Error: GetPostMethod() is failure. nRtn=[%d]\n", nRtn );
			/* エラー 500 Internal Server Error を返送 */
			nResStat = RESPONSE_STATUS_500;
			strResElmnt.m_nConnection = CONNECTION_CLOSE;
			strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
			(*pnFlagKeepAlive) = 0;
			goto OPENFILE;

		} else if( nRtn == -2 ){
			fprintf( stderr, "Warning: GetPostMethod() is failure. nRtn=[%d]\n", nRtn );

			/* リクエストボディにまだ続きがあるので さらに受信する */
			nRecvSize = 0;
			while( strReqMsg.m_nContentLength > (nCpbyte+nRecvSize) ){

				/* 一回で送信されるとは限らないと思われるので */
				/* Content-Length値分受信されるまでループする必要がある */
				nRecvSize += RecvData( nFdSockCl, (unsigned char*)(strReqMsg.m_pszReqBdy+nCpbyte+nRecvSize),
										strReqMsg.m_nContentLength-nCpbyte-nRecvSize, &nFlag );
				if( nRecvSize == -1 ){
					fprintf( stderr, "Error: RecvData() is failure. nRecvSize=[%d]\n", nRecvSize );
					/* エラー 500 Internal Server Error を返送 */
					nResStat = RESPONSE_STATUS_500;
					strResElmnt.m_nConnection = CONNECTION_CLOSE;
					strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
					(*pnFlagKeepAlive) = 0;
					goto OPENFILE;

				} else if( nRecvSize == -2 ){
					fprintf( stderr, "Error: RecvData() is failure. nRecvSize=[%d]\n", nRecvSize );
					/* バッファオーバー 400 Bad Request を返送 */
					nResStat = RESPONSE_STATUS_400;
					strResElmnt.m_nConnection = CONNECTION_CLOSE;
					strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
					(*pnFlagKeepAlive) = 0;
					goto OPENFILE;
				}


				if( nFlag ){
					/* 0 byte received */
					fprintf( stdout, "Connection with a partner went out...\n" );
					return -2;
				}
			}

			fprintf( stdout, "[%d]bytes received.\n", nRecvSize );
		}

puts( strReqMsg.m_pszReqBdy );

	}


	/* Keep-ALive要求について */
	if( !strcmp( strReqMsg.m_szHttpVer, HTTP_VER ) ){
		/* HTTPバージョン 1.1 である */

		if( !CompUpperString( strReqMsg.m_szConnection, "KEEP-ALIVE" ) ){

			/* Keep-ALive要求である */

			strResElmnt.m_nConnection = CONNECTION_KEEPALIVE;

			(*pnFlagKeepAlive) ++;
			strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX - (*pnFlagKeepAlive) +1;

			if( !strResElmnt.m_nKeepAlive ){
				/* 要求の残り回数が0回 -> Closeに */
				strResElmnt.m_nConnection = CONNECTION_CLOSE;
				(*pnFlagKeepAlive) = 0;
			}

		} else {
			/* Keep-ALive要求ではない */
			strResElmnt.m_nConnection = CONNECTION_CLOSE;
		}

	} else {
		/* HTTPバージョン 1.1 ではない */
		strResElmnt.m_nConnection = CONNECTION_CLOSE;
		strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
		(*pnFlagKeepAlive) = 0;
	}


	/* パスが空でないか確認 及び Content-Typeを決める */
	if(( strlen(strReqMsg.m_szPath) == 1 )&&( strReqMsg.m_szPath[0] == '/' )){
		/* 空である */

		/* index.html を送る */
		strResElmnt.m_nContentType = CONTENT_TYPE_TEXT_HTML;

		/* ベースディレクトリ付加 */
		snprintf( szFilePathName, sizeof(szFilePathName), "%s%s", CONTENTS_BASE_DIR, "index.html" );

	} else {
		/* 空ではない */

		/* 拡張子を調べ Content-Typeを決める */
		strResElmnt.m_nContentType = ReturnContentType( &strReqMsg.m_szPath[1], nResStat );
		if( strResElmnt.m_nContentType < 0 ){
			/* 実装していない Content-Type 時は 501 Not Implemented を返送 */
			nResStat = RESPONSE_STATUS_501;
			strResElmnt.m_nConnection = CONNECTION_CLOSE;
			strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
			(*pnFlagKeepAlive) = 0;
			goto OPENFILE;

		} else if( strResElmnt.m_nContentType == CONTENT_TYPE_CGI ){
			/* cgiの場合 */

			/* ベースディレクトリ付加 */
			snprintf( szFilePathName, sizeof(szFilePathName), "%s%s", CGI_BASE_DIR, &strReqMsg.m_szPath[1] );

			/* ファイル存在チェック */
			nRtn = stat( szFilePathName, &strStat );
			if( nRtn < 0 ){
				/* ファイルは存在しないので 404 Not Found を返送 */
				nResStat = RESPONSE_STATUS_404;
				strResElmnt.m_nConnection = CONNECTION_CLOSE;
				strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
				(*pnFlagKeepAlive) = 0;
				goto OPENFILE;
			}


			/*----------------- mutexロック -----------------*/
			if( pthread_mutex_lock( &g_nMutex ) != 0 ){
				perror( "pthread_mutex_lock()" );
				/* エラー 500 Internal Server Error を返送 */
				nResStat = RESPONSE_STATUS_500;
				strResElmnt.m_nConnection = CONNECTION_CLOSE;
				strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
				(*pnFlagKeepAlive) = 0;
				goto OPENFILE;
			}

			snprintf( szEnvNum, sizeof(szEnvNum), "%d", strReqMsg.m_nContentLength );

			nRtn = setenv( "CONTENT_LENGTH", szEnvNum, 1 );
			if( nRtn < 0 ){
				perror( "setenv()" );
				/* エラー 500 Internal Server Error を返送 */
				nResStat = RESPONSE_STATUS_500;
				strResElmnt.m_nConnection = CONNECTION_CLOSE;
				strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
				(*pnFlagKeepAlive) = 0;
				goto OPENFILE;
			}

			/* cgi実行 */
			nRtn = ExecCgi( szFilePathName, strReqMsg.m_pszReqBdy,
									(char*)szSendBuff, sizeof(szSendBuff) );
			if( nRtn < 0 ){
				fprintf( stderr, "Error: ExecCgi() is failure." );
				/* エラー 500 Internal Server Error を返送 */
				nResStat = RESPONSE_STATUS_500;
				strResElmnt.m_nConnection = CONNECTION_CLOSE;
				strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
				(*pnFlagKeepAlive) = 0;
				goto OPENFILE;
			}

			nRtn = unsetenv( "CONTENT_LENGTH" );
			if( nRtn < 0 ){
				perror( "unsetenv()" );
				/* エラー 500 Internal Server Error を返送 */
				nResStat = RESPONSE_STATUS_500;
				strResElmnt.m_nConnection = CONNECTION_CLOSE;
				strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
				(*pnFlagKeepAlive) = 0;
				goto OPENFILE;
			}

			/*----------------- mutexアンロック -----------------*/
			if( pthread_mutex_unlock( &g_nMutex ) != 0 ){
				perror( "pthread_mutex_unlock()" );
				/* エラー 500 Internal Server Error を返送 */
				nResStat = RESPONSE_STATUS_500;
				strResElmnt.m_nConnection = CONNECTION_CLOSE;
				strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
				(*pnFlagKeepAlive) = 0;
				goto OPENFILE;
			}

			nContLen = GetCgiBodyLength( (const char*)szSendBuff );
			nSendLen = strlen( (char*)szSendBuff );

			goto CREATE_RESMSG;

		} else {
			/* html 及びその他のファイルの場合 */

			/* ベースディレクトリ付加 */
			snprintf( szFilePathName, sizeof(szFilePathName), "%s%s", CONTENTS_BASE_DIR, &strReqMsg.m_szPath[1] );

			/* ファイル存在チェック */
			nRtn = stat( szFilePathName, &strStat );
			if( nRtn < 0 ){
				/* ファイルは存在しないので 404 Not Found を返送 */
				nResStat = RESPONSE_STATUS_404;
				strResElmnt.m_nConnection = CONNECTION_CLOSE;
				strResElmnt.m_nKeepAlive = KEEP_ALIVE_MAX;
				(*pnFlagKeepAlive) = 0;
				goto OPENFILE;
			}

		}
	}


OPENFILE:

	/* ファイルをオープン エラー系であったらszFilePathNameは書き換わる */
	nFd = OpenFile( szFilePathName, nResStat );
	if( nFd < 0 ){
		fprintf( stderr, "Error: OpenFile() is failure.\n" );
		return -1;
	}

	/* ファイル読み込み (サイズを知りたいため先に実行) */
	nReadSize = ReadData( nFd, szSendBuff, sizeof(szSendBuff) );
	if( nContLen < 0 ){
		fprintf( stderr, "Error: ReadData() is failure. nReadSize=[%d]\n", nReadSize );
		close( nFd );
		return -1;
	}

	close( nFd );

	nContLen = nReadSize;
	nSendLen = nReadSize;


CREATE_RESMSG:

	strResElmnt.m_szResStat = nResStat;
	strResElmnt.m_nCharset = CHARSET_UTF8;
	strResElmnt.m_nContentLength = nContLen;


	/* レスポンスメッセージ作成 */
	CreateResponseMessage( nResStat, &strResElmnt, szResMsg, sizeof(szResMsg) );

	/* レスポンスメッセージを送信 */
	nRtn = SendData( nFdSockCl, (unsigned char*)szResMsg, strlen(szResMsg) );
	if( nRtn < 0 ){
		fprintf( stderr, "Error: SendData() is failure.\n" );
		return -1;
	}

	fprintf( stdout, "[%d]bytes sent.\n", nRtn );

puts( szResMsg );

	/* ボディ部送信 */
	nRtn = SendData( nFdSockCl, szSendBuff, nSendLen );
	if( nRtn < 0 ){
		fprintf( stderr, "Error: SendData() is failure.\n" );
		return -1;
	}

	fprintf( stdout, "[%d]bytes sent.\n", nRtn );

puts( (char*)szSendBuff );


	free( strReqMsg.m_pszReqBdy );

	return 0;
}

/*
 * スレッドメイン
 */
void *ThreadWorkMain( void *arg )
{
	int nRtn = 0;
	int nFlagKeepAlive = 0;
	int nKeepAliveTimeout = 0;
	struct client_info strClientInfo;
	struct timeval strTv;
	pthread_t nThreadId;
	fd_set strFds;


	memcpy( &strClientInfo, arg, sizeof(strClientInfo) );

	nThreadId = pthread_self();

	fprintf( stdout, "clientAddr:[%s] is connect. --- socket fd:[%d] thread id:[%ld]\n",
				inet_ntoa(strClientInfo.m_strAddrCl.sin_addr), strClientInfo.m_nFdSockCl, nThreadId );


	nKeepAliveTimeout = KEEP_ALIVE_TIMEOUT;

	while(1){

		/* FD集合体初期化 当該ソケットのみを対象とする */
		FD_ZERO( &strFds );
		FD_SET( strClientInfo.m_nFdSockCl, &strFds );

		/* 1秒でタイムアウト */
		strTv.tv_sec = 1;
		strTv.tv_usec = 0;

		/*------------------- ファイルディスクリプタ監視 -------------------*/
		nRtn = select( strClientInfo.m_nFdSockCl+1, &strFds, NULL, NULL, &strTv );
		if( nRtn < 0 ){
			if( errno == EINTR ){
				/* システムコールが割り込まれたので再実行 */
				fprintf( stdout, "System call was interrupted.\n" );
				continue;

			} else {
				/* select()エラー */
				perror( "select()" );
				return NULL;

			}

		} else if( nRtn == 0 ){
			/* タイムアウト */

			/* 1秒経過(減じる) */
			nKeepAliveTimeout --;

			/* Keep-Alive timeoutで指定の秒数経過した */
			if( !nKeepAliveTimeout ){
				fprintf( stdout, "%d seconds of specification passed in Keep-Alive timeout.\n", KEEP_ALIVE_TIMEOUT );
				break;
			}

			continue;
		}

		/* ここに来た場合、監視中のファイルディスクリプタ変化があった */

		/* 当該ソケットがレディ */
		if( FD_ISSET( strClientInfo.m_nFdSockCl, &strFds ) ){

			/* Keep-Alive timeout値再設定 */
			nKeepAliveTimeout = KEEP_ALIVE_TIMEOUT;

			/* 送受信 (HTTP) */
			nRtn = RecvSend( strClientInfo.m_nFdSockCl, &nFlagKeepAlive );
			if( nRtn == -1 ){
				fprintf( stderr, "Error: RecvSend() is failure.\n" );
				break;

			} else if( nRtn == -2 ){
				fprintf( stdout, "Connection is Close.\n" );
				break;
			}

		}

		if( !nFlagKeepAlive ){
			/* Keep-Alive持続しない */
			fprintf( stdout, "TCP connection is cut for Keep-Alive Close.\n" );
			break;
		}
	}


	close( strClientInfo.m_nFdSockCl );

	fprintf( stdout, "clientAddr:[%s] is closed. --- socket fd:[%d] thread id:[%ld]\n",
				inet_ntoa(strClientInfo.m_strAddrCl.sin_addr), strClientInfo.m_nFdSockCl, nThreadId );

	return NULL;
}

/*
 * サーバソケット作成
 */
int CreateServSock( unsigned short nPort )
{
	int nRtn = 0;
	int nFdSockSv = 0;
	const unsigned char szOptOn = 1;
	struct sockaddr_in strAddrSv;


	nFdSockSv = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( nFdSockSv < 0 ){
		perror( "socket()" );
		return -1;
	}

	memset( &strAddrSv, 0x00, sizeof(struct sockaddr_in) );
	strAddrSv.sin_family      = AF_INET;
	strAddrSv.sin_addr.s_addr = htonl( INADDR_ANY );
	strAddrSv.sin_port        = htons( nPort );


	setsockopt( nFdSockSv, SOL_SOCKET, SO_REUSEADDR, &szOptOn, sizeof(szOptOn) );

	nRtn = bind( nFdSockSv, (struct sockaddr*)&strAddrSv, sizeof(struct sockaddr) );
	if( nRtn < 0 ){
		perror( "bind()" );
		close( nFdSockSv );
		return -1;
	}

	nRtn = listen( nFdSockSv, 5 );
	if( nRtn < 0 ){
		perror( "listen()" );
		close( nFdSockSv );
		return -1;
	}


	return nFdSockSv;
}

int main()
{
	int nRtn = 0;
	int nFdSockSv = 0;
	int nFdSockCl = 0;
	struct sockaddr_in strAddrCl;
	socklen_t nLen = sizeof(struct sockaddr_in);
	pthread_t nThreadId;
	struct client_info strClientInfo;


	memset( &strAddrCl, 0x00, sizeof(strAddrCl) );
	memset( &strClientInfo, 0x00, sizeof(strClientInfo) );


	nFdSockSv = CreateServSock( SERVER_PORT );
	if( nFdSockSv < 0 ){
		fprintf( stderr, "Error: CreateServSock() is failure.\n" );
		exit( EXIT_FAILURE );
	}


	/* acceptループ */
	while(1){

		nFdSockCl = accept( nFdSockSv, (struct sockaddr*)&strAddrCl, &nLen );
		if( nRtn < 0 ){
			perror( "accept()" );
			close( nFdSockSv );
			exit( EXIT_FAILURE );
		}

		strClientInfo.m_nFdSockCl = nFdSockCl;
		strClientInfo.m_strAddrCl = strAddrCl;

		nRtn = pthread_create( &nThreadId, NULL, (void*)ThreadWorkMain, (void*)&strClientInfo );
		if( nRtn ){
			perror( "pthread_create()" );
			close( nFdSockSv );
			exit( EXIT_FAILURE );
		}

		fprintf( stdout, "Thread is generated. nThreadId=[%ld]\n", nThreadId );

		nRtn = pthread_detach( nThreadId );
		if( nRtn ){
			perror( "pthread_detach()" );
			close( nFdSockSv );
			exit( EXIT_FAILURE );
		}

	}


	close( nFdSockSv );
	close( nFdSockCl );


	exit( EXIT_SUCCESS );
}
