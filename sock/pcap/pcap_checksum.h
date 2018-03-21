#ifndef __PCAP_CHECKSUM_H
#define __PCAP_CHECKSUM_H


extern uint16_t CheckSum( const uint16_t*, size_t );
extern uint16_t CheckSum2Data( const uint16_t*, size_t, const uint16_t*, size_t );


#endif
