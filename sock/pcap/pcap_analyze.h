#ifndef __PCAP_ANALYZE_H
#define __PCAP_ANALYZE_H


enum cksum_result {
	CORRECT = 0,
	INCORRECT
};

struct pseudo_ip_info {
	uint32_t nIpSrc;
	uint32_t nIpDst;
	uint8_t nDummy;
	uint8_t nIpProto;
	uint16_t nIpLen;
};


extern int gnResIpCksum;
extern int gnResIcmpCksum;
extern int gnResTcpCksum;
extern int gnResUdpCksum;


extern int AnalyzePacket( const unsigned char*, size_t );


#endif
