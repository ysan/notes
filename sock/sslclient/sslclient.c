#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <getopt.h>
#include <stdbool.h>


#include "mycommon.h"
#include "sslclient.h"
#include "sslfunc.h"
#include "starttls.h"


static void ConvCrlf( char*, int );
static int SelectProc( int );
static int CreateClientSocket( unsigned int, uint32_t );
static void Usage( char* );

/*
 * 改行コード変換 lf->crlf
 */
static void ConvCrlf( char *pszBuff, int nLen )
{
	if ( ( *(pszBuff+nLen-2) != '\r' ) && ( *(pszBuff+nLen-1) == '\n' ) ) {
		*(pszBuff+nLen-1) = '\r';
		*(pszBuff+nLen) = '\n'; /* バッファオーバ注意 */
	}

	return;
}

/*
 * select
 */
static int SelectProc( int nFdSockCl )
{
	int nRtn = 0;
	int nFlagConn = 0;
	char szStdin[1024];
	unsigned char szBuff[65536];
	fd_set strFds;


	while (1) {

		/* FD集合体を初期化 */
		FD_ZERO( &strFds );
		FD_SET( STDIN_FILENO, &strFds );
		FD_SET( nFdSockCl, &strFds );

		/*
		 * ファイルディスクリプタ監視
		 * 第5引数(タイムアウト) NULLを指定
		 * 監視対象がレディになるまで待ち続ける
		 */
		nRtn = select( nFdSockCl+1, &strFds, NULL, NULL, NULL );
		if ( nRtn < 0 ) {
				perror( "select()" );
				return -1;

		} else if ( nRtn == 0 ) {
			/* タイムアウト 設定してないのでここには来ない */

		}


		/* 監視中の何らかのファイルディスクリプタがレディになった */

		/* 標準入力がレディ */
		if ( FD_ISSET( STDIN_FILENO, &strFds ) ) {

			memset( szStdin, 0x00, sizeof(szStdin) );

			nRtn = ReadData( STDIN_FILENO, (unsigned char*)szStdin, sizeof(szStdin) );
			if ( nRtn < 0 ) {
				fprintf( stderr,"ReadData() is failure.\n" );
				return -1;
			}

			/* 改行コード変換 lf->crlf */
			ConvCrlf( szStdin, strlen(szStdin) );

			nRtn = SendBio( gpstrBio, (unsigned char*)szStdin, strlen(szStdin) );
			if ( nRtn < 0 ) {
				fprintf( stderr,"SendBio() is failure.\n" );
				return -1;
			}

			fprintf( stdout, "[%d]bytes sent.\n", nRtn );
		}

		/* 接続先からの受信がレディ */
		if ( FD_ISSET( nFdSockCl, &strFds ) ) {

			memset( szBuff, 0x00, sizeof(szBuff) );

			nRtn = RecvBio( gpstrBio, szBuff, sizeof(szBuff), &nFlagConn );
			if ( nRtn < 0 ) {
				fprintf( stderr,"RecvBio() is failure.\n" );
				return -1;

			}

			fprintf( stdout, "%s", szBuff );
			fprintf( stdout, "[%d]bytes received.\n", nRtn );


			if ( nFlagConn ) {
				/* 0 byte recieved */
				fprintf( stdout, "Connection with a partner went out...\n" );
				break;
			}

		}

	} /* select() loop end */

	return 0;
}

/*
 * クライアントソケット作成
 */
static int CreateClientSocket( unsigned int nPort, uint32_t nDestIP )
{
	int nRtn = 0;
	int nFdSockCl = 0;
	struct sockaddr_in strAddrCl;

	nFdSockCl = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( nFdSockCl < 0 ) {
		perror( "socket()" );
		return -1;
	}

	memset( &strAddrCl, 0x00, sizeof(struct sockaddr_in) );
	strAddrCl.sin_family      = AF_INET;
	strAddrCl.sin_addr.s_addr = nDestIP;
	strAddrCl.sin_port        = htons( nPort );

	nRtn = connect( nFdSockCl, (struct sockaddr*)&strAddrCl, sizeof(struct sockaddr) );
	if ( nRtn < 0 ) {
		perror( "connect()" );
		close( nFdSockCl );
		return -1;
	}

	return nFdSockCl;
}

/*
 * Usage print
 */
static void Usage( char *argv0 )
{
	fprintf(
		stderr,
		"Usage: %s -h hostname(IPaddr) -p port {Options}\n",
		argv0
	);
	fprintf( stderr, "\n" );
	fprintf( stderr, "Options:\n" );
	fprintf( stderr, "  -s             : SSL/TLS Enabled\n" );
	fprintf( stderr, "  -s --starttls  : SSL/TLS Enabled (Encryption Enabling STARTTLS in SMTP)\n" );
	return;
}

/*
 * メイン
 */
int main( int argc, char **argv )
{
	int nRtn = 0;
	int nFdSockCl = 0;
	unsigned short nPort = 0;
	uint32_t nDestIP = 0;
	char *pszHostname = NULL;
	bool bHost = FALSE;
	bool bPort = FALSE;
	bool bSsl = FALSE;
	bool bStartTls = FALSE;
	const struct option strLongOptions[] = {
		{ "starttls", no_argument, NULL, 't' },
		{ NULL, 0, NULL, 0 }
	};


	/* getopt_long 引数解析 */
	while ( ( nRtn = getopt_long( argc, argv, "h:p:s", strLongOptions, NULL ) ) != -1 ) {
		switch (nRtn) {
			case 'h':
				bHost = TRUE;
				pszHostname = optarg;
				break;

			case 'p':
				bPort = TRUE;
				nPort = atoi(optarg);
				break;

			case 's':
				bSsl = TRUE;
				break;

			case 't':
				bStartTls = TRUE;
				break;

			default:
				Usage( argv[0] );
				exit( EXIT_FAILURE );
		}
	}

	if ( !bHost || !bPort ) {
		Usage( argv[0] );
		exit( EXIT_FAILURE );
	}

	if ( !bSsl && bStartTls ) {
		Usage( argv[0] );
		exit( EXIT_FAILURE );
	}


	nRtn = GetIpAddr( (const char*)pszHostname, &nDestIP );
	if ( nRtn < 0 ) {
		fprintf( stderr, "Error: GetIpAddr() is failure.\n" );
		exit( EXIT_FAILURE );
	}


	nFdSockCl = CreateClientSocket( nPort, nDestIP );
	if ( nFdSockCl < 0 ) {
		fprintf( stderr, "Error: CreateClientSocket() is failure.\n" );
		exit( EXIT_FAILURE );
	}


	if ( bStartTls ) {
		if ( StartTlsSmtp( nFdSockCl ) < 0 ) {
			fprintf( stderr, "Error: StartTlsSmtp() is failure.\n" );
			exit( EXIT_FAILURE );		
		}
	}


	nRtn = (int)pFuncInitSsl[bSsl]( nFdSockCl );
	if ( nRtn < 0 ) {
		fprintf( stderr, "Error: pFuncInitSsl() is failure.\n" );
		exit( EXIT_FAILURE );
	}


	fprintf( stdout, "connected %s:%d %s\n", pszHostname, nPort, bSsl ? "SSL/TLS" : ""  );

	fprintf( stdout, "\n------\n" );

	nRtn = SelectProc( nFdSockCl );
	if ( nRtn < 0 ) {
		fprintf( stderr, "Error: SelectProc() is failure.\n" );
		close( nFdSockCl );
		exit( EXIT_FAILURE );
	}


	close( nFdSockCl );


	exit( EXIT_SUCCESS );
}
