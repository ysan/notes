#ifndef _NTPMONI_H_
#define _NTPMONI_H_


/*
 * Constant define
 */
#define NTP_SERVER_PORT				( 123 )
#define NTP_SERVER_NAME_LEN			( 128 )
#define TIMESTAMP_STR_LEN			( 32 )
#define IPV4_ADDR_STR_LEN			( 32 )	/* INET_ADDRSTRLEN 以上にすること */

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
	uint32_t nInteger;							/* 整数部 */ 
	uint32_t nFraction;							/* 小数部 */
};

struct ntp_packet {
	uint8_t nLI_VN_MD;							/* LeapIndi(閏秒指示子) - VersionNo - Mode */
	uint8_t nStratum;							/* 階層 */
	uint8_t nPoll;								/* ポーリング間隔 */
	uint8_t nPrecision;							/* 精度 */
	uint32_t nRootDelay;						/* ルート遅延 */
	uint32_t nRootDispersion;					/* ルート分岐 */
	uint32_t nRefIdentifier;					/* 基準ID */
	struct bit64_fixed_point stRefTimestamp;	/* 基準タイムスタンプ */
	struct bit64_fixed_point stOrgTimestamp;	/* 基点タイムスタンプ */
	struct bit64_fixed_point stRecvTimestamp;	/* 受信タイムスタンプ */
	struct bit64_fixed_point stTransTimestamp;	/* 送信タイムスタンプ */
};


/*
 * External
 */


#endif
