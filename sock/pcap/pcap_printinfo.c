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
#include "pcap_printinfo.h"
#include "pcap_analyze.h"
#include "pcap_addr2name.h"


/* <net/ethernet.h> より */
static const struct ether_type gstrEthType[] = {
	{ ETHERTYPE_PUP,		"Xerox PUP" },
	{ ETHERTYPE_IP,			"IP(Internet Protocol)" },
	{ ETHERTYPE_ARP,		"ARP(Address Resolution)" },
	{ ETHERTYPE_REVARP,		"Reverse ARP" },
	{ ETHERTYPE_AT,			"AppleTalk" },
	{ ETHERTYPE_AARP,		"AppleTalk ARP" },
	{ ETHERTYPE_IPV6,		"IP protocol version 6" },
	{ ETHERTYPE_LOOPBACK,	"used to test interfaces" },
	{ 0xffff,				"undefine" }
};

/* <net/if_arp.h> より */
static const char *gpszOpcodes[DEF_MAX_ARP_OPCODES] = {
	"undefine",
	"ARP request",
	"ARP reply",
	"RARP request",
	"RARP reply",
	"undefine",
	"undefine",
	"undefine",
	"InARP request",
	"InARP reply",
	"(ATM)ARP NAK"
};

/* <net/if_arp.h> より */
static const char *gpszHard[DEF_MAX_ARP_HARD] = {
	"From KA9Q: NET/ROM pseudo",
	"Ethernet 10/100Mbps",
	"Experimental Ethernet",
	"AX.25 Level 2",
	"PROnet token ring",
	"Chaosnet",
	"IEEE 802.2 Ethernet/TR/TB",
	"ARCnet",
	"APPLEtalk"
};

static const char *gpszProtocol[DEF_MAX_IP_PROTOCOL] = {
	"undefine",
	"ICMP",
	"IGMP",
	"undefine",
	"IPIP",
	"undefine",
	"TCP",
	"undefine",
	"EGP",
	"undefine",
	"undefine",
	"undefine",
	"PUP",
	"undefine",
	"undefine",
	"undefine",
	"undefine",
	"UDP"
};

/* <netinet/ip_icmp.h> より */
static const char *gpszIcmpType[DEF_MAX_ICMP_TYPE] = {
	"Echo Reply",				/*0*/
	"undefine",					/*1*/
	"undefine",					/*2*/
	"Destination Unreachable",	/*3*/
	"Source Quench",			/*4*/
	"Redirect (change route)",	/*5*/
	"undefine",					/*6*/
	"undefine",					/*7*/
	"Echo Request",				/*8*/
	"Router advertisement",		/*9*/
	"Route selection",			/*10*/
	"Time Exceeded",			/*11*/
	"Parameter Problem",		/*12*/
	"Timestamp Request",		/*13*/
	"Timestamp Reply",			/*14*/
	"Information Request",		/*15*/
	"Information Reply",		/*16*/
	"Address Mask Request",		/*17*/
	"Address Mask Reply"		/*18*/
};

static const char *gpszIcmpCode[DEF_MAX_ICMP_TYPE][DEF_MAX_ICMP_TYPE_IN] = {
	{ "-" },	/*0*/
	{ "-" },	/*1*/
	{ "-" },	/*2*/
	{
		"Network Unreachable",
		"Host Unreachable",
		"Protocol Unreachable",
		"Port Unreachable",
		"Fragmentation needed and DF (Don't Fragment) set",
		"Source route failed",
		"Destination Network unknown",
		"Destination Host unknown",
		"Source Host isolated",
		"Communication with Destination Network Administratively Prohibited",
		"Communication with Destination Host Administratively Prohibited",
		"Network Unreachable for Type Of Service",
		"Host Unreachable for Type Of Service",
		"Communication Administratively Prohibited by Filtering",
		"Host Precedence Violation",
		"Precedence Cutoff in Effect"
	},			/*3*/
	{ "-" },	/*4*/
	{
		"Redirect for network",
		"Redirect for host",
		"Redirect for TOS and network",
		"Redirect for TOS and host"
	},			/*5*/
	{ "-" },	/*6*/
	{ "-" },	/*7*/
	{ "-" },	/*8*/
	{
		"Normal router advertisement",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"Does not route common traffic"
	},			/*9*/
	{ "-" },	/*10*/
	{
		"TTL equals 0 during transit",
		"TTL equals 0 during reassembly"
	},			/*11*/
	{
		"IP header bad (catchall error)",
		"Required options missing",
		"IP Header bad length"
	},			/*12*/
	{ "-" }, 	/*13*/
	{ "-" }, 	/*14*/
	{ "-" },	/*15*/
	{ "-" }, 	/*16*/
	{ "-" }, 	/*17*/
	{ "-" }		/*18*/
};


void PrintEtherHeader( const struct ether_header *pstrEtherheader )
{
	int i = 0;

	fprintf( stdout, "----- Ether Header -----\n" );

	/* 送信元 source host adr */
	fprintf(
		stdout,
		"ether_shost=[%02x:%02x:%02x:%02x:%02x:%02x]\n",
		pstrEtherheader->ether_shost[0],
		pstrEtherheader->ether_shost[1],
		pstrEtherheader->ether_shost[2],
		pstrEtherheader->ether_shost[3],
		pstrEtherheader->ether_shost[4],
		pstrEtherheader->ether_shost[5]
	);

	/* 宛先 destination host adr */
	fprintf(
		stdout,
		"ether_dhost=[%02x:%02x:%02x:%02x:%02x:%02x]\n",
		pstrEtherheader->ether_dhost[0],
		pstrEtherheader->ether_dhost[1],
		pstrEtherheader->ether_dhost[2],
		pstrEtherheader->ether_dhost[3],
		pstrEtherheader->ether_dhost[4],
		pstrEtherheader->ether_dhost[5]
	);

	for( i=0; gstrEthType[i].m_nEthType!=0xffff; i++ ){
		if( ntohs(pstrEtherheader->ether_type) == gstrEthType[i].m_nEthType ){
			break;
		}
	}

	fprintf( stdout, "ether_type=[0x%x:%s]\n",
						 ntohs(pstrEtherheader->ether_type), gstrEthType[i].m_pszEthType );

	return;
}

void PrintArp( const struct ether_arp *pstrArp )
{
	int i = 0;

	fprintf( stdout, "----- ARP -----\n" );

	if( ntohs(pstrArp->arp_hrd) < DEF_MAX_ARP_HARD ){
		fprintf( stdout, "arp_hrd(Format of hardware address)=[%d:%s]\n",
				 ntohs(pstrArp->arp_hrd), gpszHard[ntohs(pstrArp->arp_hrd)] );
	} else {
		fprintf( stdout, "arp_hrd(Format of hardware address)=[%d:%s]\n",
				 ntohs(pstrArp->arp_hrd), "undefine" );
	}

	for( i=0; gstrEthType[i].m_nEthType!=0xffff; i++ ){
		if( ntohs(pstrArp->arp_pro) == gstrEthType[i].m_nEthType ){
			break;
		}
	}
	fprintf( stdout, "arp_pro(Format of protocol address)=[0x%x:%s]\n",
				ntohs(pstrArp->arp_pro), gstrEthType[i].m_pszEthType );

	fprintf( stdout, "arp_hln(Length of hardware address.)=[%d]\n", pstrArp->arp_hln );
	fprintf( stdout, "arp_pln(Length of protocol address.)=[%d]\n", pstrArp->arp_pln );

	if(ntohs(pstrArp->arp_op) < DEF_MAX_ARP_OPCODES ){
		fprintf( stdout, "arp_op(ARP opcode)=[%d:%s]\n",
				 ntohs(pstrArp->arp_op), gpszOpcodes[ntohs(pstrArp->arp_op)] );
	} else {
		fprintf( stdout, "arp_op(ARP opcode)=[%d:%s]\n",
				 ntohs(pstrArp->arp_op), "undefine" );
	}

	fprintf(
		stdout,
		"arp_sha(sender hardware address)=[%02x:%02x:%02x:%02x:%02x:%02x]\n",
		pstrArp->arp_sha[0],
		pstrArp->arp_sha[1],
		pstrArp->arp_sha[2],
		pstrArp->arp_sha[3],
		pstrArp->arp_sha[4],
		pstrArp->arp_sha[5]
	);

	fprintf(
		stdout,
		"arp_spa(sender protocol address)=[%d.%d.%d.%d]\n",
		pstrArp->arp_spa[0],
		pstrArp->arp_spa[1],
		pstrArp->arp_spa[2],
		pstrArp->arp_spa[3]
	);

	fprintf(
		stdout,
		"arp_tha(target hardware address)=[%02x:%02x:%02x:%02x:%02x:%02x]\n",
		pstrArp->arp_tha[0],
		pstrArp->arp_tha[1],
		pstrArp->arp_tha[2],
		pstrArp->arp_tha[3],
		pstrArp->arp_tha[4],
		pstrArp->arp_tha[5]
	);

	fprintf(
		stdout,
		"arp_tpa(target protocol address)=[%d.%d.%d.%d]\n",
		pstrArp->arp_tpa[0],
		pstrArp->arp_tpa[1],
		pstrArp->arp_tpa[2],
		pstrArp->arp_tpa[3]
	);

	return;
}

void PrintIpHeader( const struct ip *pstrIp, const int *pnOptLen, const unsigned char* pszOpt )
{
	int i = 0;
	char szName[NAME_SIZE];

	fprintf( stdout, "----- IP Header -----\n" );

	fprintf( stdout, "ip_v(version)=[%d]\n", pstrIp->ip_v );
	fprintf( stdout, "ip_hl(header length)=[%d(%dbytes)]\n", pstrIp->ip_hl, pstrIp->ip_hl * 4 );
	fprintf( stdout, "ip_tos(type of service)=[0x%02x]\n", pstrIp->ip_tos );
	fprintf( stdout, "ip_len(total length)=[%d]\n", ntohs(pstrIp->ip_len) );
	fprintf( stdout, "ip_id(identification)=[%d]\n", ntohs(pstrIp->ip_id) );
	fprintf( stdout, "ip_off(fragment offset field)=[flags:0x%02x][offset:%d]\n",
			 (ntohs(pstrIp->ip_off)>>13) & 0x07,
			 ntohs(pstrIp->ip_off) & 0x1fff );
	fprintf( stdout, "ip_ttl(time to live)=[%d]\n", pstrIp->ip_ttl );

	if( pstrIp->ip_p < DEF_MAX_IP_PROTOCOL ){
		fprintf( stdout, "ip_p(protocol)=[%d:%s]\n", pstrIp->ip_p, gpszProtocol[pstrIp->ip_p] );

	} else {
		fprintf( stdout, "ip_p(protocol)=[%d:%s]\n", pstrIp->ip_p, "undefine" );
	}

	fprintf( stdout, "ip_sum(checksum)=[0x%04x] %s\n",
							ntohs(pstrIp->ip_sum), !gnResIpCksum ? CKSUM_CORRECT : CKSUM_INCORRECT );

	/* アドレス->名前変換 */
	memset( szName, 0x00, sizeof(szName) );
	szName[0] = ':';
	if ( GetName( pstrIp->ip_src.s_addr, &szName[1], sizeof(szName)-1 ) < 0 ) {
		memset( szName, 0x00, sizeof(szName) );
	}

	fprintf(
		stdout,
		"ip_src(source address)=[%d.%d.%d.%d%s]\n",
		ntohl(pstrIp->ip_src.s_addr)>>24 & 0xff,
		ntohl(pstrIp->ip_src.s_addr)>>16 & 0xff,
		ntohl(pstrIp->ip_src.s_addr)>>8 & 0xff,
		ntohl(pstrIp->ip_src.s_addr) & 0xff,
		szName
	);
	
	/*DDDDDDDDDDDDDDDDDDDDDDDDDDDDD*/
/*
	char buf[20];
	memset(buf,0x00,sizeof(buf));
	inet_ntop(AF_INET,&(pstrIp->ip_src),buf,sizeof(buf));
	puts(buf);
*/

	/* アドレス->名前変換 */
	memset( szName, 0x00, sizeof(szName) );
	szName[0] = ':';
	if ( GetName( pstrIp->ip_dst.s_addr, &szName[1], sizeof(szName)-1 ) < 0 ) {
		memset( szName, 0x00, sizeof(szName) );
	}

	fprintf(
		stdout,
		"ip_dst(dest address)=[%d.%d.%d.%d%s]\n",
		ntohl(pstrIp->ip_dst.s_addr)>>24 & 0xff,
		ntohl(pstrIp->ip_dst.s_addr)>>16 & 0xff,
		ntohl(pstrIp->ip_dst.s_addr)>>8 & 0xff,
		ntohl(pstrIp->ip_dst.s_addr) & 0xff,
		szName
	);

	fprintf( stdout, "option length=[%d]\n", *pnOptLen );

	/* オプションの内容表示 */
	if( *pnOptLen > 0 ){
		fprintf( stdout, "option:\n" );
		for(i=0; i<*pnOptLen; i++ ){
			if( (i%16) == 0 && i != 0 ){
				fprintf( stdout, "\n" );
			}
			fprintf( stdout, "%02x ", *(pszOpt+i) );
		}
		fprintf( stdout, "\n" );
	}

	return;
}

void PrintIcmp( const struct icmp *pstrIcmp )
{

	fprintf( stdout, "----- ICMP -----\n" );

	if( pstrIcmp->icmp_type < DEF_MAX_ICMP_TYPE ){
		fprintf( stdout, "icmp_type=[%d:%s]\n",
					pstrIcmp->icmp_type, gpszIcmpType[pstrIcmp->icmp_type] );
	} else {
		fprintf( stdout, "icmp_type=[%d:%s]\n", pstrIcmp->icmp_type, "undefine" );
	}

	fprintf( stdout, "icmp_code=[%d:%s]\n",
				pstrIcmp->icmp_code, gpszIcmpCode[pstrIcmp->icmp_type][pstrIcmp->icmp_code] );

	fprintf( stdout, "icmp_cksum=[0x%x] %s\n",
						ntohs(pstrIcmp->icmp_cksum), !gnResIcmpCksum ? CKSUM_CORRECT : CKSUM_INCORRECT );

	/* タイプが EchoReply と EchoRequest 時のみ以下を表示 */
	if( pstrIcmp->icmp_type == ICMP_ECHOREPLY || pstrIcmp->icmp_type == ICMP_ECHO ){
		fprintf( stdout, "icmp_id=[%d]\n", pstrIcmp->icmp_id );
		fprintf( stdout, "icmp_seq=[%d]\n", pstrIcmp->icmp_seq );
	}

	return;
}

void PrintTcp( const struct tcphdr *pstrTcphdr )
{
	fprintf( stdout, "----- TCP Header -----\n" );

	fprintf( stdout, "source(port)=[%d]\n", ntohs(pstrTcphdr->source) );
	fprintf( stdout, "dest(port)=[%d]\n", ntohs(pstrTcphdr->dest) );
	fprintf( stdout, "seq(Sequence Number)=[0x%08x]\n", ntohl(pstrTcphdr->seq) );
	fprintf( stdout, "ack_seq(Acknowledgement Number)=[0x%08x]\n",
						ntohl(pstrTcphdr->ack_seq) );
	/*------> 以下16bit分はtcphdr構造体にてエンディアン変換済み */
	fprintf( stdout, "doff(Data Offset)=[%d(%doctet)]\n",
						pstrTcphdr->doff, pstrTcphdr->doff * 4 );
	fprintf( stdout, "res1(Reserved)=[%d]\n",
						pstrTcphdr->res1 );
	fprintf( stdout, "res2(Reserved)=[%d]\n",
						pstrTcphdr->res2 );
	fprintf( stdout, "codebit=[%s%s%s%s%s%s]\n",
						pstrTcphdr->urg ? "URG," : "",
						pstrTcphdr->ack ? "ACK," : "",
						pstrTcphdr->psh ? "PSH," : "",
						pstrTcphdr->rst ? "RST," : "",
						pstrTcphdr->syn ? "SYN," : "",
						pstrTcphdr->fin ? "FIN," : "" );
	/*------> ここまで */
	fprintf( stdout, "window=[%d]\n", ntohs(pstrTcphdr->window) );
	fprintf( stdout, "check(Checksum)=[0x%04x] %s\n",
						ntohs(pstrTcphdr->check), !gnResTcpCksum ? CKSUM_CORRECT : CKSUM_INCORRECT );
	fprintf( stdout, "urg_ptr(Urgent Pointer)=[%d]\n", ntohs(pstrTcphdr->urg_ptr) );

	return;
}

void PrintUdp( const struct udphdr *pstrUdphdr )
{
	fprintf( stdout, "----- UDP Header -----\n" );

	fprintf( stdout, "source(port)=[%d]\n", ntohs(pstrUdphdr->source) );
	fprintf( stdout, "dest(port)=[%d]\n", ntohs(pstrUdphdr->dest) );
	fprintf( stdout, "len(length)=[%d]\n", ntohs(pstrUdphdr->len) );
	fprintf( stdout, "check(Checksum)=[0x%04x] %s\n",
						ntohs(pstrUdphdr->check), !gnResUdpCksum ? CKSUM_CORRECT : CKSUM_INCORRECT );

	return;
}

