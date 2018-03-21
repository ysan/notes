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

#include "mycommon.h"
#include "pcap_checksum.h"


uint16_t CheckSum( const uint16_t *pn16bit, size_t nSize )
{
	uint32_t nSum = 0;

	while ( nSize > 1 ) {
		nSum += *pn16bit;

		if ( nSum & 0x10000 ) {
			/*
			 * 16bit目が立っていたら
			 * オーバーフロー分を足す
			 */
			nSum = ( nSum >> 16 ) + ( nSum & 0xffff );
		}
//printf("nSum 0x%04x\n",nSum);

		nSize -= 2;
		pn16bit ++;
	}

	/* 余りの1byte分 */
	if ( nSize == 1 ) {
		nSum += *pn16bit;

		if ( nSum & 0x10000 ) {
			/*
			 * 16bit目が立っていたら
			 * オーバーフロー分を足す
			 */
			nSum = ( nSum >> 16 ) + ( nSum & 0xffff );
		}
//printf("nSum 0x%04x\n",nSum);
	}


	/* 1の補数(ビット反転) */
	return (uint16_t)(~nSum);
}

uint16_t CheckSum2Data(
	const uint16_t *pn16bit1,
	size_t nSize1,
	const uint16_t *pn16bit2,
	size_t nSize2
)
{
	uint32_t nSum = 0;

	/*---------- data1 ----------*/
	while ( nSize1 > 1 ) {
		nSum += *pn16bit1;

		if ( nSum & 0x10000 ) {
			/*
			 * 16bit目が立っていたら
			 * オーバーフロー分を足す
			 */
			nSum = ( nSum >> 16 ) + ( nSum & 0xffff );
		}
//printf("nSum 0x%04x %d\n",nSum,nSize1);

		nSize1 -= 2;
		pn16bit1 ++;
	}

	/* 余りの1byte分 */ /* 現状ここには入らない!!! */
	if ( nSize1 == 1 ) {
		nSum += *pn16bit1;

		/* data2の先頭1byte分を付け足す */
//		nSum += (*pn16bit2 >> 8) & 0xff;
		nSum += *((uint8_t*)pn16bit2);

		if ( nSum & 0x10000 ) {
			/*
			 * 16bit目が立っていたら
			 * オーバーフロー分を足す
			 */
			nSum = ( nSum >> 16 ) + ( nSum & 0xffff );
		}
//printf("*nSum 0x%04x\n",nSum);

		/* data2先頭ポインタを進めておく */
		pn16bit2 = (uint16_t*)( (uint8_t*)pn16bit2 +1 );
		nSize2 --;
	}


	/*---------- data2 ----------*/
	while ( nSize2 > 1 ) {
		nSum += *pn16bit2;

		if ( nSum & 0x10000 ) {
			/*
			 * 16bit目が立っていたら
			 * オーバーフロー分を足す
			 */
			nSum = ( nSum >> 16 ) + ( nSum & 0xffff );
		}
//printf("nSum 0x%04x %d  0x%04x\n",nSum,nSize2,*pn16bit2);

		nSize2 -= 2;
		pn16bit2 ++;
	}

	/* 余りの1byte分 */
	if ( nSize2 == 1 ) {
		nSum += *pn16bit2;

		if ( nSum & 0x10000 ) {
			/*
			 * 16bit目が立っていたら
			 * オーバーフロー分を足す
			 */
			nSum = ( nSum >> 16 ) + ( nSum & 0xffff );
		}
//printf("*nSum 0x%04x\n",nSum);
	}


	/* 1の補数(ビット反転) */
	return (uint16_t)(~nSum);
}
