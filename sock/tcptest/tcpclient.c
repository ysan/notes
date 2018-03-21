#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>


struct info {
	unsigned int m_nAAA;
	unsigned int m_nBBB;
	unsigned int m_nCCC;
};

int main()
{
	int nRtn = 0;
	int nFdSockCl = 0;
	unsigned char szBuff[128];
	unsigned short nPort = 0;
	struct sockaddr_in strAddrCl;

	memset( &strAddrCl, 0x00, sizeof(struct sockaddr_in) );

	nPort = 20001;
	strAddrCl.sin_family = AF_INET;
	strAddrCl.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	strAddrCl.sin_port = htons( nPort );


	if( ( nFdSockCl = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ){
		perror( "socket()" );
		exit( EXIT_FAILURE );
	}


	nRtn = connect( nFdSockCl, (struct sockaddr*)&strAddrCl, sizeof(struct sockaddr) );
	if( nRtn < 0 ){
		perror( "connect()" );
		close( nFdSockCl );
		exit( EXIT_FAILURE );
	}


	struct info strInfo;
	strInfo.m_nAAA = 0x12345678;
	strInfo.m_nBBB = 0xabcd1234;
	strInfo.m_nCCC = 0x5678abcd;

	memset( szBuff, 0x00, sizeof(szBuff) );
	send( nFdSockCl, (unsigned char*)&strInfo, sizeof(strInfo), 0 );


	exit( EXIT_SUCCESS );
}
