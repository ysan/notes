#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "echoserv_socket.h"

/* 
 * サーバソケット作成
 */
int CreateServSock( unsigned short nPort )
{
	int nRtn = 0;
	int nFdSockSv = 0;
	struct sockaddr_in stAddrSv;

	memset( &stAddrSv, 0x00, sizeof(struct sockaddr_in) );
	stAddrSv.sin_family = AF_INET;
	stAddrSv.sin_addr.s_addr = htonl( INADDR_ANY );
	stAddrSv.sin_port = htons( nPort );

	nFdSockSv = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( nFdSockSv < 0 ) {
		perror( "socket()" );
		return -1;
	}

	nRtn = bind( nFdSockSv, (struct sockaddr*)&stAddrSv, sizeof(struct sockaddr) );
	if ( nRtn < 0 ) {
		perror( "bind()" );
		return -1;
	}

	nRtn = listen( nFdSockSv, 5 );
	if ( nRtn < 0 ) {
		perror( "listen()" );
		return -1;
	}

	return nFdSockSv;
}
