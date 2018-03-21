#ifndef _NTPMONI_H_
#define _NTPMONI_H_


/*
 * Constant define
 */
#define NTP_SERVER_PORT				( 123 )
#define NTP_SERVER_NAME_LEN			( 128 )
#define TIMESTAMP_STR_LEN			( 32 )
#define IPV4_ADDR_STR_LEN			( 32 )	/* INET_ADDRSTRLEN $B0J>e$K$9$k$3$H(B */

#define CONF_FILE_PATH				"./ntpmoni.conf"


typedef enum {
	EN_COND_WAIT_KIND_CONN_START = 0,
	EN_COND_WAIT_KIND_THREAD_END
} EN_COND_WAIT_KIND;

typedef enum {
	EN_STATE_INIT = 0,
	EN_STATE_THREAD_CREATE,
	EN_STATE_THREAD_WAIT,
	EN_STATE_THREAD_EXE,
	EN_STATE_THREAD_END,
} EN_STATE;


/*
 * Type define
 */
struct serv_info {
	char szName[ NTP_SERVER_NAME_LEN ];
	uint32_t nAddr;								/* network byte order */
	BOOL isInitValid;
	BOOL isInitTerm;

	EN_STATE enState;

	int nFdSock;
	pthread_t nThId;
	char szRefTimestamp[ TIMESTAMP_STR_LEN ];
	char szOrgTimestamp[ TIMESTAMP_STR_LEN ];
	char szRecvTimestamp[ TIMESTAMP_STR_LEN ];
	char szTransTimestamp[ TIMESTAMP_STR_LEN ];
};

struct bit64_fixed_point {
	uint32_t nInteger;							/* $B@0?tIt(B */ 
	uint32_t nFraction;							/* $B>.?tIt(B */
};

struct ntp_packet {
	uint8_t nLI_VN_MD;							/* LeapIndi($B1<IC;X<(;R(B) - VersionNo - Mode */
	uint8_t nStratum;							/* $B3,AX(B */
	uint8_t nPoll;								/* $B%]!<%j%s%04V3V(B */
	uint8_t nPrecision;							/* $B@:EY(B */
	uint32_t nRootDelay;						/* $B%k!<%HCY1d(B */
	uint32_t nRootDispersion;					/* $B%k!<%HJ,4t(B */
	uint32_t nRefIdentifier;					/* $B4p=`(BID */
	struct bit64_fixed_point stRefTimestamp;	/* $B4p=`%?%$%`%9%?%s%W(B */
	struct bit64_fixed_point stOrgTimestamp;	/* $B4pE@%?%$%`%9%?%s%W(B */
	struct bit64_fixed_point stRecvTimestamp;	/* $B<u?.%?%$%`%9%?%s%W(B */
	struct bit64_fixed_point stTransTimestamp;	/* $BAw?.%?%$%`%9%?%s%W(B */
};


/*
 * External
 */


#endif
