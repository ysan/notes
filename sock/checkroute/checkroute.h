#ifndef _CHECKROUTE_H_
#define _CHECKROUTE_H_

#include "mycommon.h"


#define UNEXPECTED_ICMP			(ICMP_MAXTYPE+1)
#define SELECT_TIMEOUT			(ICMP_MAXTYPE+2)
#define DEF_MAX_ICMP_TYPE		(ICMP_MAXTYPE+1)

#define SOCKOPT_ON				(1)

#define SEND_BUFF_SIZE			(1024)
#define RECV_BUFF_SIZE			(1024)

#define TIMEOUT_SEC_VAL			(5)
#define TIMEOUT_USEC_VAL		(500000)

#define ADDR_STRING_SIZE		(16)
#define NAME_SIZE				(128)

#define LOOP_MAX				(3)

#define FIRST_TTL_VAL			(1)
#define TIMEOUT_RETRY_COUNT		(1)
#define TOTAL_TIMEOUT_COUNT		(5)

typedef struct request_info {
	uint32_t nSrcAddr;		/* ネットワークバイトオーダ */
	uint32_t nDestAddr;		/* ネットワークバイトオーダ */
	uint8_t nTtl;
} ST_REQUEST_INFO;

typedef struct response_info {
	char szReachedAddr[ADDR_STRING_SIZE];
	char szName[NAME_SIZE];
	uint8_t nIcmpType;
	uint8_t nIcmpCode;
	int nFlagTimeout;
	ST_TIME_STR stTimeStr;
} ST_RESPONSE_INFO;



#endif
