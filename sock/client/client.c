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


#include "mycommon.h"
#include "client.h"


int SelectProccess( int nFdSockCl )
{
	int nRtn = 0;
	int nFlag = 0;
	unsigned char szStdin[1024];
	unsigned char szBuff[1024*5];
	fd_set strFds;


	while(1){

		/* FD集合体を初期化 */
		FD_ZERO( &strFds );
		FD_SET( STDIN_FILENO, &strFds );
		FD_SET( nFdSockCl, &strFds );

		/*------------------- ファイルディスクリプタ監視 -------------------*/
		/* 第5引数(タイムアウト) NULLを指定すると監視対象がレディになるまで待ち続ける */
		nRtn = select( nFdSockCl+1, &strFds, NULL, NULL, NULL );
		if( nRtn < 0 ){
			if( errno == EINTR ){
				/* システムコールが割り込まれたので再実行 */
                fprintf( stdout, "System call was interrupted.\n" );
				continue;
			}
			else {
				/* select()エラー */
				perror( "select()" );
				return -1;
			}

		} else if( nRtn == 0 ){
			/* タイムアウト 設定してないのでここには来ない */

		}


		/* 監視中の何らかのファイルディスクリプタがレディになった */

		/* 標準入力がレディ */
		if( FD_ISSET( STDIN_FILENO, &strFds ) ){

			memset( szStdin, 0x00, sizeof(szStdin) );

			nRtn = ReadData( STDIN_FILENO, szStdin, sizeof(szStdin) );
			if( nRtn < 0 ){
				fprintf( stderr,"ReadData() is failure.\n" );
				close( nFdSockCl );
				return -1;
			}

			nRtn = SendData( nFdSockCl, szStdin, strlen((char*)szStdin) );
			if( nRtn < 0 ){
				fprintf( stderr,"SendData() is failure.\n" );
				close( nFdSockCl );
				return -1;
			}

			fprintf( stdout, "[%d]bytes sent.\n", nRtn );
		}

		/* 接続先からの受信がレディ */
		if( FD_ISSET( nFdSockCl, &strFds ) ){

			memset( szBuff, 0x00, sizeof(szBuff) );

			nRtn = RecvData( nFdSockCl, szBuff, sizeof(szBuff), &nFlag );
			if( nRtn < 0 ){
				fprintf( stderr,"RecvData() is failure.\n" );
				close( nFdSockCl );
				return -1;

			}

			fprintf( stdout, "%s", szBuff );
			fprintf( stdout, "[%d]bytes received.\n", nRtn );

			if( nFlag ){
				/* 0 byte recieved */
				fprintf( stdout, "Connection with a partner went out...\n" );
				break;
			}
		}
		/*-----------------------------------------------------------------*/
	}

	return 0;
}

/*
 * クライアントソケット作成
 */
int CreateClientSock( unsigned int nPort, uint32_t nDestIP )
{
	int nRtn = 0;
	int nFdSockCl = 0;
	struct sockaddr_in strAddrCl;

	nFdSockCl = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( nFdSockCl < 0 ){
		perror( "socket()" );
		return -1;
	}

	memset( &strAddrCl, 0x00, sizeof(struct sockaddr_in) );
	strAddrCl.sin_family      = AF_INET;
	strAddrCl.sin_addr.s_addr = nDestIP;
	strAddrCl.sin_port        = htons( nPort );

	nRtn = connect( nFdSockCl, (struct sockaddr*)&strAddrCl, sizeof(struct sockaddr) );
	if( nRtn < 0 ){
		perror( "connect()" );
		close( nFdSockCl );
		return -1;
	}

	return nFdSockCl;
}

int main( int argc, char **argv )
{
	int nRtn = 0;
	int nFdSockCl = 0;
	unsigned short nPort = atoi(argv[2]);
	uint32_t nDestIP = 0;
	char *pszDestIP = argv[1];


	if( argc != 3 ){
		fprintf( stderr, "Usage: %s <ip(hostname)> <port>\n", argv[0] );
		exit( EXIT_FAILURE );
	}

	nRtn = GetIpAddr( (const char*)pszDestIP, &nDestIP );
	if( nRtn < 0 ){
		fprintf( stderr, "Error: GetIpAddr() is failure.\n" );
		exit( EXIT_FAILURE );
	}


	nFdSockCl = CreateClientSock( nPort, nDestIP );
	if( nFdSockCl < 0 ){
		fprintf( stderr, "Error: CreateClientSock() is failure.\n" );
		exit( EXIT_FAILURE );
	}

	fprintf( stdout, "connected %s:%d\n", pszDestIP, nPort );


	/* 標準入力を非カノニカルモードに設定 */
/***
	nRtn = SetNonCanon( STDIN_FILENO );
	if( nRtn < 0 ){
		fprintf( stderr, "Error: SetNonCanon() is failure.\n" );
		exit( EXIT_FAILURE );
	}
***/


	nRtn = SelectProccess( nFdSockCl );
	if( nRtn < 0 ){
		fprintf( stderr, "Error: SelectProccess() is failure.\n" );
		exit( EXIT_FAILURE );
	}


	close( nFdSockCl );


	exit( EXIT_SUCCESS );
}
