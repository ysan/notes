#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include "mycommon.h"
#include "pcap_addr2name.h"

int main( void )
{
	char szTmp[1024];


	InitNameList();


	memset( szTmp, 0x00, sizeof(szTmp) );
	GetName( (uint32_t)inet_addr("157.7.144.5"), szTmp, sizeof(szTmp) );
	printf( "[%s]\n", szTmp );

	memset( szTmp, 0x00, sizeof(szTmp) );
	GetName( (uint32_t)inet_addr("203.216.243.240"), szTmp, sizeof(szTmp) );
	printf( "[%s]\n", szTmp );

	memset( szTmp, 0x00, sizeof(szTmp) );
	GetName( (uint32_t)inet_addr("157.7.144.5"), szTmp, sizeof(szTmp) );
	printf( "[%s]\n", szTmp );

	memset( szTmp, 0x00, sizeof(szTmp) );
	GetName( (uint32_t)inet_addr("203.216.243.240"), szTmp, sizeof(szTmp) );
	printf( "[%s]\n", szTmp );

	memset( szTmp, 0x00, sizeof(szTmp) );
	GetName( (uint32_t)inet_addr("157.7.144.5"), szTmp, sizeof(szTmp) );
	printf( "[%s]\n", szTmp );

	memset( szTmp, 0x00, sizeof(szTmp) );
	GetName( (uint32_t)inet_addr("173.194.126.159"), szTmp, sizeof(szTmp) );
	printf( "[%s]\n", szTmp );

	memset( szTmp, 0x00, sizeof(szTmp) );
	GetName( (uint32_t)inet_addr("173.194.126.159"), szTmp, sizeof(szTmp) );
	printf( "[%s]\n", szTmp );

	memset( szTmp, 0x00, sizeof(szTmp) );
	GetName( (uint32_t)inet_addr("203.190.58.241"), szTmp, sizeof(szTmp) );
	printf( "[%s]\n", szTmp );

	memset( szTmp, 0x00, sizeof(szTmp) );
	GetName( (uint32_t)inet_addr("192.168.0.101"), szTmp, sizeof(szTmp) );
	printf( "[%s]\n", szTmp );

	memset( szTmp, 0x00, sizeof(szTmp) );
	GetName( (uint32_t)inet_addr("127.0.0.1"), szTmp, sizeof(szTmp) );
	printf( "[%s]\n", szTmp );

	memset( szTmp, 0x00, sizeof(szTmp) );
	GetName( (uint32_t)inet_addr("192.168.0.1"), szTmp, sizeof(szTmp) );
	printf( "[%s]\n", szTmp );

	memset( szTmp, 0x00, sizeof(szTmp) );
	GetName( (uint32_t)inet_addr("110.4.144.92"), szTmp, sizeof(szTmp) );
	printf( "[%s]\n", szTmp );


	RefNameList();

	FreeAllMallocList();


	exit( EXIT_SUCCESS );
}
