#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>

#include "mycommon.h"
#include "sendarp.h"
#include "pcap_socket.h"


#define HW_ADDR_SIZE			(6)
#define IPV4_ADDR_SIZE			(4)
#define IPV4_ADDR_STRING_LEN	(3+1+3+1+3+1+3+1)

#define MALLOC_NUM		(1024)

typedef struct interface_info {
	uint8_t nHwAddr[ HW_ADDR_SIZE ];
	uint32_t nIpAddr;		/* ネットワークバイトオーダ */
	uint32_t nSubnetmask;	/* ネットワークバイトオーダ */
} ST_INTERFACE_INFO;


/*
 * 変数宣言
 */
static ST_INTERFACE_INFO gstInterfaceInfo;


/*
 * プロトタイプ宣言
 */
static int CreateArpRequestPacket( const ST_INTERFACE_INFO*, const uint32_t*, uint8_t*, size_t );
static uint32_t *CreateSubnetList( void );
static void DestorySubnet( uint32_t *p );
static int SendArpRequest( int nFdSock );
static int GetInterfaceInfo( int, const char*, ST_INTERFACE_INFO* );

/*
 * ARP要求パケット作成
 */
static int CreateArpRequestPacket (
	const ST_INTERFACE_INFO *pstInterfaceInfo,	/* in */
	const uint32_t *pnDestIpAddr,				/* in */ /* ネットワークバイトオーダ */
	uint8_t *pszBuff,							/* out */
	size_t nBuffSize							/* in */
)
{
	/* 引数チェック */
	if (
		( !pstInterfaceInfo )	||
		( !pnDestIpAddr )		||
		( !pszBuff )			||
		( nBuffSize == 0 )
	) {
		fprintf( stderr, "Err: Argument is abnormal.\n" );
		return -1;
	}

	/* バッファオーバーチェック */
	if ( nBuffSize < ( sizeof(struct ether_header) + sizeof(struct ether_arp) ) ) {
		fprintf( stderr, "Err: Buffer over. (set ARP packet)\n" );
		return -1;
	}

	struct ether_header *pstEthHeader = NULL;
	struct ether_arp *pstEthArp = NULL;



	/* Etherヘッダセット */
	pstEthHeader = (struct ether_header*)pszBuff;

						/* ブロードキャスト */
	pstEthHeader->ether_dhost[0] = 0xff;
	pstEthHeader->ether_dhost[1] = 0xff;
	pstEthHeader->ether_dhost[2] = 0xff;
	pstEthHeader->ether_dhost[3] = 0xff;
	pstEthHeader->ether_dhost[4] = 0xff;
	pstEthHeader->ether_dhost[5] = 0xff;

	pstEthHeader->ether_shost[0] = pstInterfaceInfo->nHwAddr[0];
	pstEthHeader->ether_shost[1] = pstInterfaceInfo->nHwAddr[1];
	pstEthHeader->ether_shost[2] = pstInterfaceInfo->nHwAddr[2];
	pstEthHeader->ether_shost[3] = pstInterfaceInfo->nHwAddr[3];
	pstEthHeader->ether_shost[4] = pstInterfaceInfo->nHwAddr[4];
	pstEthHeader->ether_shost[5] = pstInterfaceInfo->nHwAddr[5];

	pstEthHeader->ether_type = htons( ETHERTYPE_ARP );

	pszBuff += sizeof(struct ether_header);


	/* ARPデータセット */
	pstEthArp = (struct ether_arp*)pszBuff;

	pstEthArp->arp_hrd = htons( ARPHRD_ETHER );
	pstEthArp->arp_pro = htons( ETHERTYPE_IP );
	pstEthArp->arp_hln = HW_ADDR_SIZE;
	pstEthArp->arp_pln = IPV4_ADDR_SIZE;
	pstEthArp->arp_op = htons( ARPOP_REQUEST );

	pstEthArp->arp_sha[0] = pstInterfaceInfo->nHwAddr[0];
	pstEthArp->arp_sha[1] = pstInterfaceInfo->nHwAddr[1];
	pstEthArp->arp_sha[2] = pstInterfaceInfo->nHwAddr[2];
	pstEthArp->arp_sha[3] = pstInterfaceInfo->nHwAddr[3];
	pstEthArp->arp_sha[4] = pstInterfaceInfo->nHwAddr[4];
	pstEthArp->arp_sha[5] = pstInterfaceInfo->nHwAddr[5];

	pstEthArp->arp_spa[0] = pstInterfaceInfo->nIpAddr & 0xff;
	pstEthArp->arp_spa[1] = ( pstInterfaceInfo->nIpAddr >> 8 ) & 0xff;
	pstEthArp->arp_spa[2] = ( pstInterfaceInfo->nIpAddr >> 16 ) & 0xff;
	pstEthArp->arp_spa[3] = ( pstInterfaceInfo->nIpAddr >> 24 ) & 0xff;

				/* ARP要求の為 0 をセット */
	pstEthArp->arp_tha[0] = 0x00;
	pstEthArp->arp_tha[1] = 0x00;
	pstEthArp->arp_tha[2] = 0x00;
	pstEthArp->arp_tha[3] = 0x00;
	pstEthArp->arp_tha[4] = 0x00;
	pstEthArp->arp_tha[5] = 0x00;

				/* 宛先アドレスをセット */
	pstEthArp->arp_tpa[0] =   *pnDestIpAddr         & 0xff;
	pstEthArp->arp_tpa[1] = ( *pnDestIpAddr >>  8 ) & 0xff;
	pstEthArp->arp_tpa[2] = ( *pnDestIpAddr >> 16 ) & 0xff;
	pstEthArp->arp_tpa[3] = ( *pnDestIpAddr >> 24 ) & 0xff;


	return 0;
}

#if 0
/*
 * サブネット内における全IPアドレスを生成
 */
static uint32_t *CreateSubnetList( void )
{
	int i = 0;
	int j = 0;
	uint8_t nOctet[4];
	uint16_t nTmpWord[4];
	uint32_t ntNetworkAddr = 0;
	uint32_t nTmpDword[4];
	uint32_t *pnAddrStat = NULL;
	uint32_t *pnAddr = NULL;
	uint32_t *pnAddrTmp = NULL;


	memset( nTmpWord, 0x00, sizeof(nTmpWord) );
	memset( nTmpDword, 0x00, sizeof(nTmpDword) );


	pnAddrStat = (uint32_t*)malloc( sizeof(uint32_t)*1024 );
	if ( !pnAddrStat ) {
		perror( "malloc()" );
		return NULL;
	}

	memset( pnAddrStat, 0x00, sizeof(uint32_t)*1024 );

	pnAddr = pnAddrStat; /* 作業用にコピー */


//DDDDDDDDDDDDDDDDDDDDDD
//addr 192.168.0.1
//sub 255.255.240.0
gstInterfaceInfo.nSubnetmask = 0x00f0ffff;


	nNetworkAddr = gstInterfaceInfo.nIpAddr & gstInterfaceInfo.nSubnetmask; // 192.168.0.0

	nOctet[0] = ( ~gstInterfaceInfo.nSubnetmask       ) & 0xff; // 0
	nOctet[1] = ( ~gstInterfaceInfo.nSubnetmask >>  8 ) & 0xff; // 0
	nOctet[2] = ( ~gstInterfaceInfo.nSubnetmask >> 16 ) & 0xff; // 15
	nOctet[3] = ( ~gstInterfaceInfo.nSubnetmask >> 24 ) & 0xff; // 255

	while (1) {

		nTmpDword[0] |= (uint32_t)( ( nTmpWord[0] ) & 0x000000ff );

		while (1) {

			nTmpDword[1] = nTmpDword[0];
			nTmpDword[1] |= (uint32_t)( ( nTmpWord[1] <<  8 ) & 0x0000ff00 );

			while (1) {

				nTmpDword[2] = nTmpDword[1];
				nTmpDword[2] |= (uint32_t)( ( nTmpWord[2] << 16 ) & 0x00ff0000 );

				while (1) {

					nTmpDword[3] = nTmpDword[2];
					nTmpDword[3] |= (uint32_t)( ( nTmpWord[3] << 24 ) & 0xff000000 );

					/* 領域拡張 1024bytesごと */
					if (( i%1024 == 0 )&&( i != 0 )) {
puts("re");
						j ++;

						pnAddrTmp = (uint32_t*)realloc( pnAddrStat, sizeof(uint32_t)*1024*(j+1) );
						if ( !pnAddrTmp ) {
							perror( "realloc()" );
							free( pnAddrStat );
							return NULL;
						}

						pnAddrStat = pnAddrTmp; /* 先頭アドレス更新 */

						/* 今確保したアドレスを作業用に */
						pnAddr = pnAddrTmp + 1024*j;
						memset( pnAddr, 0x00, sizeof(uint32_t)*1024 );
					}

					*pnAddr = nNetworkAddr | nTmpDword[3];

					pnAddr ++;
					i ++;

					nTmpWord[3] ++;

					if (( nTmpWord[3] > nOctet[3] )||( nOctet[3] == 0 )) {
						break;
					}

				} /* while end */

				nTmpWord[3] = 0;

				nTmpWord[2] ++;

				if (( nTmpWord[2] > nOctet[2] )||( nOctet[2] == 0 )) {
					break;
				}

			} /* while end */

			nTmpWord[2] = 0;

			nTmpWord[1] ++;

			if (( nTmpWord[1] > nOctet[1] )||( nOctet[1] == 0 )) {
				break;
			}

		} /* while end */

		nTmpWord[1] = 0;

		nTmpWord[0] ++;

		if (( nTmpWord[0] > nOctet[0] )||( nOctet[0] == 0 )) {

			/* 1024で割りきれた場合 終端分1つ追加 */
			if ( (i%1024) == 0 ) {
puts("add last realloc");
				pnAddrTmp = (uint32_t*)realloc( pnAddrStat, sizeof(uint32_t)*(1024*(j+1) +1) );
				if ( !pnAddrTmp ) {
					perror( "realloc()" );
					free( pnAddrStat );
					return NULL;
				}

				pnAddrStat = pnAddrTmp; /* 先頭アドレス更新 */

				memset( pnAddrTmp + 1024*(j+1), 0x00, sizeof(uint32_t) );
			}

			break;
		}

	} /* while end */


	return pnAddrStat;
}
#endif

/*
 * サブネット内における全IPアドレスを生成
 */
static uint32_t *CreateSubnetList( void )
{
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t nNetworkAddr = 0;
	uint32_t nCntFull = 0;
	uint32_t *pnAddrSta = NULL;
	uint32_t *pnAddrWork = NULL;
	uint32_t *pnAddrRe = NULL;


	pnAddrSta = (uint32_t*)malloc( sizeof(uint32_t)*MALLOC_NUM );
	if ( !pnAddrSta ) {
		perror( "malloc()" );
		return NULL;
	}

	memset( pnAddrSta, 0x00, sizeof(uint32_t)*MALLOC_NUM );

	pnAddrWork = pnAddrSta;


//DDDDDDDDDDDDDDDDDDDDDD
//addr 192.168.0.100
//sub 255.255.240.0
gstInterfaceInfo.nSubnetmask = 0x00f0ffff;


	nNetworkAddr = gstInterfaceInfo.nIpAddr & gstInterfaceInfo.nSubnetmask; // 192.168.0.0

	nCntFull = ntohl( ~gstInterfaceInfo.nSubnetmask );

	while ( nCntFull >= i ) {

		/* 領域拡張 MALLOC_NUMごと */
		if ( ( i%MALLOC_NUM == 0 ) && ( i != 0 ) ) {
puts("realloc");
			j ++;

			pnAddrRe = (uint32_t*)realloc( pnAddrSta, sizeof(uint32_t)*MALLOC_NUM*(j+1) );
			if ( !pnAddrRe ) {
				perror( "realloc()" );
				free( pnAddrSta );
				return NULL;
			}

			pnAddrSta = pnAddrRe; /* 先頭アドレス変わってるかもしれないので更新 */

			/* 今確保した分の先頭アドレスを作業用に */
			pnAddrWork = pnAddrRe + MALLOC_NUM*j;
			memset( pnAddrWork, 0x00, sizeof(uint32_t)*MALLOC_NUM );
		}

		/* アドレス値格納 */
		*pnAddrWork = htonl( ntohl(nNetworkAddr) + i );

		i ++;
		pnAddrWork ++;
	}

	/* 念のため終端分1つ追加 */
	pnAddrRe = (uint32_t*)realloc( pnAddrSta, sizeof(uint32_t)*(MALLOC_NUM*(j+1) +1) );
	if ( !pnAddrRe ) {
		perror( "realloc()" );
		free( pnAddrSta );
		return NULL;
	}

	pnAddrSta = pnAddrRe; /* 先頭アドレス変わってるかもしれないので更新 */

	memset( pnAddrRe + MALLOC_NUM*(j+1), 0x00, sizeof(uint32_t) );


	return pnAddrSta;
}

/*
 * 確保した領域を破棄
 */
static void DestorySubnet( uint32_t *p )
{
	if ( !p ) {
		fprintf( stderr, "Err: Argument is ubnormal.\n" );
		return;
	}

	free( p );
	p = NULL;
}

/*
 * ARP要求送信
 */
static int SendArpRequest( int nFdSock )
{
	uint8_t szBuff[1024];

	memset( szBuff, 0x00, sizeof(szBuff) );

// TODO
// ここでサブネット内にブロードキャストする

struct in_addr addr;
inet_aton("192.168.0.1",&addr);


	if ( CreateArpRequestPacket( &gstInterfaceInfo, &addr.s_addr, szBuff, sizeof(szBuff) ) < 0 ) {
		fprintf( stderr, "CreateArpRequestPacket() is failure.\n" );
		return -1;
	}

	SendData( nFdSock, szBuff, ( sizeof(struct ether_header) + sizeof(struct ether_arp) ) );


	return 0;
}

/*
 * ネットワークインターフェース情報取得 
 */
static int GetInterfaceInfo (
	int nFdSock,
	const char *pszInterface,
	ST_INTERFACE_INFO *pstInterfaceInfo
)
{
	if ( ( !pszInterface ) || ( !pstInterfaceInfo ) ) {
		fprintf( stderr, "Argument is abnormal\n" );
		return -1;
	}

	struct ifreq stIFreq;
	struct sockaddr_in stAddrin;

	memset( &stIFreq, 0x00, sizeof(stIFreq) );
	memset( &stAddrin, 0x00, sizeof(stAddrin) );


	/* MACアドレス取得 */
	strncpy( stIFreq.ifr_name, pszInterface, sizeof(stIFreq.ifr_name)-1 );

	if ( ioctl( nFdSock, SIOCGIFHWADDR, &stIFreq ) < 0 ) {
		perror( "ioctl( SIOCGIFHWADDR )" );
		return -1;
	}

	memcpy (
		&(pstInterfaceInfo->nHwAddr[0]),
		(uint8_t*)&stIFreq.ifr_hwaddr.sa_data,
		HW_ADDR_SIZE
	);


	/* IPアドレス取得 */
	if ( ioctl( nFdSock, SIOCGIFADDR, &stIFreq ) < 0 ) {
		perror( "ioctl( SIOCGIFADDR )" );
		return -1;
	}

	if ( stIFreq.ifr_addr.sa_family != PF_INET ) {
		fprintf( stderr, "[%s] is not PF_INET.\n", pszInterface );
		return -1;
	}

	memcpy( &stAddrin, &(stIFreq.ifr_addr), sizeof(stAddrin) );
	pstInterfaceInfo->nIpAddr = stAddrin.sin_addr.s_addr;


	/* サブネットマスク取得 */
	if ( ioctl( nFdSock, SIOCGIFNETMASK, &stIFreq ) < 0 ) {
		perror( "ioctl( SIOCGIFNETMASK )" );
		return -1;
	}

	memcpy( &stAddrin, &(stIFreq.ifr_addr), sizeof(stAddrin) );
	pstInterfaceInfo->nSubnetmask = stAddrin.sin_addr.s_addr;


	return 0;
}

/*
 * メイン
 */
int main( int argc, char **argv )
{
	int nFdSock = 0;
	char szTmp[ IPV4_ADDR_STRING_LEN ];


	if ( argc != 2 ) {
		fprintf( stderr, "Usage: %s <NetworkInterface>\n", argv[0] );
		exit( EXIT_FAILURE );
	}


	if ( ( nFdSock = CreateRawSocket( argv[1], ETH_P_ALL ) ) < 0 ) {
		fprintf( stderr, "Err: CreateRawSocket()\n" );
		exit( EXIT_FAILURE );
	}

	if ( GetInterfaceInfo( nFdSock, argv[1], &gstInterfaceInfo ) < 0 ) {
		fprintf( stderr, "Err: GetInterfaceInfo()\n" );
		close( nFdSock );
		exit( EXIT_FAILURE );
	}


	fprintf (
		stdout,
		"mac addr; %02x.%02x.%02x.%02x.%02x.%02x\n",
		gstInterfaceInfo.nHwAddr[0],
		gstInterfaceInfo.nHwAddr[1],
		gstInterfaceInfo.nHwAddr[2],
		gstInterfaceInfo.nHwAddr[3],
		gstInterfaceInfo.nHwAddr[4],
		gstInterfaceInfo.nHwAddr[5]
	);

	if ( inet_ntop( AF_INET, &(gstInterfaceInfo.nIpAddr), szTmp, IPV4_ADDR_STRING_LEN -1 ) ) {
		fprintf( stdout, "ipv4 addr: %s\n", szTmp );
	}

	if ( inet_ntop( AF_INET, &(gstInterfaceInfo.nSubnetmask), szTmp, IPV4_ADDR_STRING_LEN -1 ) ) {
		fprintf( stdout, "subnet mask: %s\n", szTmp );
	}


	uint32_t *pnSubnetList = CreateSubnetList();
	if ( !pnSubnetList ) {
		fprintf( stderr, "CreateSubnetList() is failure.\n" );
		close( nFdSock );
		exit( EXIT_FAILURE );
	}

	uint32_t *pnWork = pnSubnetList;

	while ( *pnWork ) {
char szTmp3[16];
puts( inet_ntop( AF_INET, pnWork, szTmp3, 16 -1 ) );

		pnWork ++;
	}


	if ( SendArpRequest( nFdSock ) < 0 ) {
		fprintf( stderr, "Err: SendArpRequest()\n" );
		close( nFdSock );
		exit( EXIT_FAILURE );
	}


	DestorySubnet( pnSubnetList );
	close( nFdSock );


	exit( EXIT_SUCCESS );
}
