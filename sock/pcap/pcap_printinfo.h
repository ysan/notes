#ifndef __PCAP_PRINTINFO_H
#define __PCAP_PRINTINFO_H


#define DEF_MAX_ARP_OPCODES		(11)
#define DEF_MAX_ARP_HARD		(9)
#define DEF_MAX_IP_PROTOCOL		(18)
#define DEF_MAX_ICMP_TYPE		(19)
#define DEF_MAX_ICMP_TYPE_IN	(16)


#define CKSUM_CORRECT	"Correct."
#define CKSUM_INCORRECT	"Incorrect!!"


struct ether_type {
	uint16_t m_nEthType;
	char *m_pszEthType;
};


extern void PrintEtherHeader( const struct ether_header* );
extern void PrintArp( const struct ether_arp* );
extern void PrintIpHeader( const struct ip*, const int*, const unsigned char* );
extern void PrintIcmp( const struct icmp* );
extern void PrintTcp( const struct tcphdr* );
extern void PrintUdp( const struct udphdr* );


#endif
