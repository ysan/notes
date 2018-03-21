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
	int nFdSockSv = 0;
	int nFdSockCl = 0;
	unsigned char szBuff[128];
	unsigned short nPort = 0;
	struct sockaddr_in strAddrSv;
	struct sockaddr_in strAddrCl;
	socklen_t nAddrLenCl = sizeof(struct sockaddr_in);


	memset( &strAddrSv, 0x00, sizeof(struct sockaddr_in) );
	memset( &strAddrCl, 0x00, sizeof(struct sockaddr_in) );

	nPort = 20001;
	strAddrSv.sin_family = AF_INET;
	strAddrSv.sin_addr.s_addr = htonl( INADDR_ANY );
	strAddrSv.sin_port = htons( nPort );


	if( ( nFdSockSv = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ){
		perror( "socket()" );
		exit( EXIT_FAILURE );
	}

	if( bind( nFdSockSv, (struct sockaddr*)&strAddrSv, sizeof(struct sockaddr_in) ) < 0 ){
		perror( "bind()" );
		close( nFdSockSv );
		exit( EXIT_FAILURE );
	}

	if( listen( nFdSockSv, SOMAXCONN ) < 0 ){
		perror( "listen()" );
		close( nFdSockSv );
		exit( EXIT_FAILURE );
	}


	while(1){
puts("accept blocking...");

		if(( nFdSockCl = accept( nFdSockSv, (struct sockaddr*)&strAddrCl, &nAddrLenCl )) < 0 ){
			perror( "accept()" );
			close( nFdSockSv );
			exit( EXIT_FAILURE );
		}
			
		printf( "clientAddr:[%s] SocketFd:[%d] --- connected.\n",
					inet_ntoa(strAddrCl.sin_addr), nFdSockCl );



		while(1){
			memset( szBuff, 0x00, sizeof(szBuff) );

puts("recv blocking...");
			nRtn = recv( nFdSockCl, szBuff, sizeof(szBuff), 0 );
			if( nRtn < 0 ){
				close( nFdSockSv );
				close( nFdSockCl );
				exit( EXIT_FAILURE );

			} else if( !nRtn ){
				break;

			} else {
				struct info *pstrInfo = NULL;
				pstrInfo = (struct info*)szBuff;
				printf( "[0x%08x]\n", pstrInfo->m_nAAA );
				printf( "[0x%08x]\n", pstrInfo->m_nBBB );
				printf( "[0x%08x]\n", pstrInfo->m_nCCC );
			}

		}

		close( nFdSockCl );
		printf( "clientAddr:[%s] SocketFd:[%d] --- disconnected.\n",
					inet_ntoa(strAddrCl.sin_addr), nFdSockCl );

	}


	exit( EXIT_SUCCESS );
}
