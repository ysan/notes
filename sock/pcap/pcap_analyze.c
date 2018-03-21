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
#include "pcap_analyze.h"
#include "pcap_printinfo.h"
#include "pcap_checksum.h"


int gnResIpCksum;
int gnResIcmpCksum;
int gnResTcpCksum;
int gnResUdpCksum;

int AnalyzePacket( const unsigned char *pszBuff, size_t nLestSize )
{
	int nOptLen = 0;
	uint16_t nRtnIpCksum = 0;
	uint16_t nRtnIcmpCksum = 0;
	uint16_t nRtnTcpCksum = 0;
	uint16_t nRtnUdpCksum = 0;
	const unsigned char *pszOpt = NULL;
	struct ether_header *pstrEtherheader = NULL;
	struct ip *pstrIp = NULL;
	struct pseudo_ip_info stPseudoIpInfo;


	gnResIpCksum = CORRECT;
	gnResIcmpCksum = CORRECT;
	gnResTcpCksum = CORRECT;
	gnResUdpCksum = CORRECT;


	/* サイズチェック */
	if( nLestSize < (int)sizeof(struct ether_header) ){
		fprintf( stderr,
					"Err: nLestSize=[%d] < sizeof(struct ether_header)\n",
					(int)nLestSize );
		return -1;
	}

	pstrEtherheader = (struct ether_header*)pszBuff;

	/* etherヘッダ情報表示 */
	PrintEtherHeader( pstrEtherheader );

	/* ポインタ進める */
	pszBuff += sizeof(struct ether_header);
	nLestSize -= sizeof(struct ether_header);


	/*---------- 以下etherタイプごとに処理 ----------*/
	switch( ntohs(pstrEtherheader->ether_type) ){

		case ETHERTYPE_ARP:

			/* サイズチェック */
			if( nLestSize < (int)sizeof(struct ether_arp) ){
				fprintf( stderr,
							"Err: nLestSize=[%d] < sizeof(struct ether_arp)\n",
							(int)nLestSize );
				return -1;
			}

			/* ARPパケット情報表示 */
			PrintArp( (const struct ether_arp*)pszBuff );

			/* ポインタ進める */
			pszBuff += sizeof(struct ether_arp);
			nLestSize -= sizeof(struct ether_arp);

		break;

		case ETHERTYPE_IP:

			/* サイズチェック */
			if( nLestSize < (int)sizeof(struct ip) ){
				fprintf( stderr,
							"Err: nLestSize=[%d] < sizeof(struct ip)\n",
							(int)nLestSize );
				return -1;
			}

			pstrIp = (struct ip*)pszBuff;

			/*
			 * オプションの存在チェック
			 * (先頭20bytes分は基本ヘッダで固定長、それを越えた分がオプション)
			 */
			nOptLen = ( pstrIp->ip_hl * 4 ) - sizeof(struct ip);
			if( nOptLen < 0 ){
				fprintf( stderr,
							"Err: Option length is abnormal.  nOptLen=[%d]\n",
							nOptLen );
				return -1;
			}

			/* オプション用ポインタ */
			pszOpt = pszBuff;
			pszOpt += sizeof(struct ip);

			/*
			 * IPチェックサム
			 * 16bitごとの1の補数和を取り、さらにその1の補数を取る
			 */
			nRtnIpCksum = CheckSum( (const uint16_t*)pszBuff, (pstrIp->ip_hl)*4 );
			if( nRtnIpCksum != 0x0 && nRtnIpCksum != 0xffff ){
//				fprintf( stderr,
//							"Err: IP checksum is abnormal.  nRtnIpCksum=[0x%04x]\n",
//							nRtnIpCksum );
				gnResIpCksum = INCORRECT;
			}

			/* IPヘッダ情報表示 */
			PrintIpHeader( pstrIp, &nOptLen, pszOpt );

			/* ポインタ進める (IPヘッダ分とオプション長) */
			pszBuff += ( sizeof(struct ip) + nOptLen );
			nLestSize -= ( sizeof(struct ip) + nOptLen );


			/*---------- 以下IPプロトコルごとに処理 ----------*/
			switch( pstrIp->ip_p ){

				case IPPROTO_ICMP:

					/* サイズチェック */
					if( nLestSize < (int)sizeof(struct icmp) ){
						fprintf( stderr,
									"Err: nLestSize=[%d] < sizeof(struct icmp)\n",
									(int)nLestSize );
						return -1;
					}

					/*
					 * ICMPチェックサム
					 */
					nRtnIcmpCksum = CheckSum( (const uint16_t*)pszBuff,
												ntohs(pstrIp->ip_len) - (pstrIp->ip_hl)*4 );
					if( nRtnIcmpCksum != 0x0 && nRtnIcmpCksum != 0xffff ){
//						fprintf( stderr,
//									"Err: ICMP checksum is abnormal. nRtnIcmpCksum=[0x%04x]\n",
//									nRtnIcmpCksum );
						gnResIcmpCksum = INCORRECT;
					}

					/* ICMP情報表示 */
					PrintIcmp( (const struct icmp*)pszBuff );

					/* データ部はどうなってる? */

					/* ポインタ進める */
					/* struct icmpでunion有り サイズは合わないと思われる */
					pszBuff += sizeof(struct icmp);
					nLestSize -= sizeof(struct icmp);

				break;

				case IPPROTO_TCP:

					/* サイズチェック */
					if( nLestSize < (int)sizeof(struct tcphdr) ){
						fprintf( stderr,
									"Err: nLestSize=[%d] < sizeof(struct tcphdr)\n",
									(int)nLestSize );
						return -1;
					}

					/* 擬似IPヘッダにデータをセット */
					memset( &stPseudoIpInfo, 0x00, sizeof(stPseudoIpInfo) );
					stPseudoIpInfo.nIpSrc = pstrIp->ip_src.s_addr;
					stPseudoIpInfo.nIpDst = pstrIp->ip_dst.s_addr;
					stPseudoIpInfo.nDummy = (uint8_t)0x00;
					stPseudoIpInfo.nIpProto = pstrIp->ip_p;
					/* TCPヘッダ以下のサイズをセット */
					stPseudoIpInfo.nIpLen = htons( ntohs(pstrIp->ip_len) - (pstrIp->ip_hl)*4 );

					/*
					 * TCPチェックサム
					 */
					nRtnTcpCksum = CheckSum2Data(
										(uint16_t*)&stPseudoIpInfo,
										sizeof(struct pseudo_ip_info),
										(uint16_t*)pszBuff,
										(size_t)( ntohs(pstrIp->ip_len) - (pstrIp->ip_hl)*4 ) );
					if ( (nRtnTcpCksum != 0x00) && (nRtnTcpCksum != 0xffff) ) {
//						fprintf( stderr,
//									"Err: TCP checksum is abnormal. nRtnTcpCksum=[0x%04x]\n",
//									nRtnTcpCksum );
						gnResTcpCksum = INCORRECT;
					}

					/* TCP情報表示 */
					PrintTcp( (const struct tcphdr*)pszBuff );


				break;

				case IPPROTO_UDP:

					/* サイズチェック */
					if( nLestSize < (int)sizeof(struct udphdr) ){
						fprintf( stderr,
									"Err: nLestSize=[%d] < sizeof(struct udphdr)\n",
									(int)nLestSize );
						return -1;
					}

					const struct udphdr *pstUdphdr = (struct udphdr*)pszBuff;

					/* UDPチェックサム0でなければ以下行う */
					if( pstUdphdr->check ) {
						/* 擬似IPヘッダにデータをセット */
						memset( &stPseudoIpInfo, 0x00, sizeof(stPseudoIpInfo) );
						stPseudoIpInfo.nIpSrc = pstrIp->ip_src.s_addr;
						stPseudoIpInfo.nIpDst = pstrIp->ip_dst.s_addr;
						stPseudoIpInfo.nDummy = (uint8_t)0x00;
						stPseudoIpInfo.nIpProto = pstrIp->ip_p;
						/* UDPヘッダ以下のサイズをセット */
						stPseudoIpInfo.nIpLen = htons( ntohs(pstrIp->ip_len) - (pstrIp->ip_hl)*4 );

						/*
						 * UDPチェックサム
						 */
						nRtnUdpCksum = CheckSum2Data(
											(uint16_t*)&stPseudoIpInfo,
											sizeof(struct pseudo_ip_info),
											(uint16_t*)pszBuff,
											(size_t)( ntohs(pstrIp->ip_len) - (pstrIp->ip_hl)*4 ) );
						if ( (nRtnUdpCksum != 0x00) && (nRtnUdpCksum != 0xffff) ) {
//							fprintf( stderr,
//										"Err: UDP checksum is abnormal. nRtnUdpCksum=[0x%04x]\n",
//										nRtnUdpCksum );
							gnResIcmpCksum = INCORRECT;
						}
					}

					/* UDP情報表示 */
					PrintUdp( (const struct udphdr*)pszBuff );

				break;

				default:
				break;

			} /* switch( pstrIp->ip_p ) end  */

		break;

		case ETHERTYPE_IPV6:

		break;

		default:
		break;

	} /* switch( ntohs(pstrEtherheader->ether_type) ) end  */

fprintf( stdout, "debug... nLestSize=[%d]\n", (int)nLestSize );

	return 0;
}
