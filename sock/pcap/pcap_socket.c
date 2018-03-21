#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>

#include "mycommon.h"
#include "pcap_socket.h"


/*
 * ソケット作成
 */
int CreateRawSocket( const char *pszInterface, uint16_t nProtocol )
{
	int nRtn = 0;
	int nFdSock = 0;
	struct ifreq stIFreq;
	struct sockaddr_ll stAddrll;
//	struct packet_mreq stMreq;

	memset( &stIFreq, 0, sizeof(stIFreq) );
	memset( &stAddrll, 0, sizeof(stAddrll) );
//	memset( &stMreq, 0x00, sizeof(stMreq) );

	/* ソケット作成 */
	nFdSock = socket( PF_PACKET, SOCK_RAW, htons(nProtocol) );
	if ( nFdSock < 0 ) {
		perror( "socket()" );
		return -1;
	}

	/* NICのインデックス取得 */
	strncpy( stIFreq.ifr_name, pszInterface, sizeof(stIFreq.ifr_name)-1 );
	nRtn = ioctl( nFdSock, SIOCGIFINDEX, &stIFreq );
	if ( nRtn < 0 ) {
		perror( "ioctl(SIOCGIFINDEX)" );
		return -1;
	}

	/* ソケット確定 */
	stAddrll.sll_family = PF_PACKET;
	stAddrll.sll_protocol = htons(nProtocol);
	stAddrll.sll_ifindex = stIFreq.ifr_ifindex;
	nRtn = bind( nFdSock, (struct sockaddr*)&stAddrll, sizeof(stAddrll) );
	if ( nRtn < 0 ) {
		perror( "bind()" );
		return -1;
	}

	/* プロミスキャスモード有効 */
#if PROMISCUOUS_MODE == 1
	nRtn = ioctl( nFdSock, SIOCGIFFLAGS, &stIFreq );
	if ( nRtn < 0 ) {
		perror( "ioctl(SIOCGIFFLAGS)" );
		return -1;
	}

	stIFreq.ifr_flags |= IFF_PROMISC;
	nRtn = ioctl( nFdSock, SIOCSIFFLAGS, &stIFreq );
	if ( nRtn < 0 ) {
		perror( "ioctl(SIOCSIFFLAGS)" );
		return -1;
	}

#if 0
	stMreq.mr_type = PACKET_MR_PROMISC;
	stMreq.mr_ifindex = stIFreq.ifr_ifindex;
	if ( setsockopt( nFdSock, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
										(void*)&stMreq, sizeof(stMreq) ) < 0 ) {
		perror( "setsockopt()" );
		return -1;
	}
#endif
#endif

	return nFdSock;
}
