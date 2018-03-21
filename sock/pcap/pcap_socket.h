#ifndef __PCAP_SOCKET_H
#define __PCAP_SOCKET_H


#define PROMISCUOUS_MODE	(1) /* 0:無効  1:有効 */


extern int CreateRawSocket( const char*, uint16_t );

#endif
