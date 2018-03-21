#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#include "mycommon.h"
#include "checkroute.h"
#include "pcap_checksum.h"


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

static int CreateIcmpEchoRequestPacket( const ST_REQUEST_INFO*, unsigned char*, size_t, size_t* );
static int AnalyzePacket( const unsigned char*, ST_RESPONSE_INFO* );
static int SendRecv ( int, const ST_REQUEST_INFO*, ST_RESPONSE_INFO* );
static void PutsResult( const ST_RESPONSE_INFO*, const int* );
static int SendRecvLoop( int, uint32_t, uint32_t );
static int CreateSocket( void );


/*
 * ICMP Echo要求パケット作成
 */
static int CreateIcmpEchoRequestPacket (
	const ST_REQUEST_INFO *pstRequestInfo,	/* in */
	unsigned char *pszBuff,					/* out */
	size_t nBuffSize,						/* in */
	size_t *pnSendLen						/* out */
)
{
	struct ip stIp;
	struct icmp stIcmp;

	memset( &stIp, 0x00, sizeof(stIp) );
	memset( &stIcmp, 0x00, sizeof(stIcmp) );

	/* IPヘッダセット */
	stIp.ip_v = 4;													/* version */
	stIp.ip_hl = (uint8_t)(sizeof(stIp)/4); /* 基本ヘッダのみ */	/* header lenfth */
	stIp.ip_tos = 0x00;												/* type of service */
	stIp.ip_len =													/* total length */
		htons( (uint16_t)(sizeof(stIp)+sizeof(stIcmp)) ); /* 値が勝手に入る... */
	stIp.ip_id = 0x0000; /* 値が勝手に入る... */					/* identification */
	stIp.ip_off = 0x0000;											/* fragment offset field */
	stIp.ip_ttl = pstRequestInfo->nTtl; /* 1,2,3,...と増やす */		/* time to live */
	stIp.ip_p = IPPROTO_ICMP;										/* protocol */
	stIp.ip_sum = 0x0000;											/* checksum */
	stIp.ip_src.s_addr = pstRequestInfo->nSrcAddr;					/* source address */
	stIp.ip_dst.s_addr = pstRequestInfo->nDestAddr;					/* dest address */
	/* オプションなし */
	
	/*
	 * IPチェックサム計算
	 * ここで入れなくてもTCPオーバーロードしてくれている様...
	 * htons()不要?
	 */
	stIp.ip_sum = CheckSum( (const uint16_t*)&stIp, sizeof(stIp) );


	/* ICMPデータセット */
	stIcmp.icmp_type = ICMP_ECHO;
	stIcmp.icmp_code = 0x00;
	stIcmp.icmp_cksum = 0x0000;
	stIcmp.icmp_id = htons( (uint16_t)getpid() );
	stIcmp.icmp_seq = 0x0000;
	/* 以降のicmp_dun共用体は空... */

	/* 
	 * ICMPチェックサム計算
	 * htons()不要?
	 */
	stIcmp.icmp_cksum = CheckSum( (const uint16_t*)&stIcmp, sizeof(stIcmp) );



	/* バッファオーバーチェック */
	if ( nBuffSize < sizeof(stIp) ) {
		fprintf( stderr, "Err: Buffer over. (set IPheader)\n" );
		return -1;
	}
 
	/* IPヘッダ書き込み */
	memcpy( pszBuff, &stIp, sizeof(stIp) );

	pszBuff += sizeof(stIp);
	nBuffSize -= sizeof(stIp);


	/* バッファオーバーチェック */
	if ( nBuffSize < sizeof(stIcmp) ) {
		fprintf( stderr, "Err: Buffer over. (set ICMP)\n" );
		return -1;
	}
 
	/* 以下ICMPデータ書き込み */
	memcpy( pszBuff, &stIcmp, sizeof(stIcmp) );


	/* 書き込みバイト長返却 */
	*pnSendLen = sizeof(stIp) + sizeof(stIcmp);


	return 0;
}

/*
 * 受信パケット解析
 */
static int AnalyzePacket( const unsigned char *pszBuff, ST_RESPONSE_INFO *pstResponseInfo )
{
	int nRtn = 0;
	const struct ip *pstIp = (const struct ip*)pszBuff;
	const struct icmp *pstIcmp = NULL;


	/* 送信元アドレス文字列返却 */
	inet_ntop( AF_INET, &(pstIp->ip_src), pstResponseInfo->szReachedAddr, ADDR_STRING_SIZE );

	/* 逆引き アドレス->名前変換 */
	GetReverseIp( pstIp->ip_src.s_addr, pstResponseInfo->szName, NAME_SIZE );


	/* IPヘッダ部分はとばしてICMP部を解析 */
	pszBuff += sizeof(struct ip);
	pstIcmp = (const struct icmp*)pszBuff;

	/* ICMP TypeとCodeを返却 */
	pstResponseInfo->nIcmpType = pstIcmp->icmp_type;
	pstResponseInfo->nIcmpCode = pstIcmp->icmp_code;


	if ( pstIcmp->icmp_type == ICMP_ECHOREPLY ) {
		/* 宛先アドレスまで到達 */

		if ( ntohs(pstIcmp->icmp_id) == (uint16_t)getpid() ) {

			nRtn = ICMP_ECHOREPLY;

		} else {
			/* 本アプリからのICMP Echo要求に対するものではない */
			nRtn = UNEXPECTED_ICMP;

		}

	} else if ( pstIcmp->icmp_type == ICMP_TIME_EXCEEDED ) {
		/* TTL 0 */
		nRtn = ICMP_TIME_EXCEEDED;

	} else {
		/* 想定外のICMP Type */
		nRtn = UNEXPECTED_ICMP;
	}

	return nRtn;
}

/*
 * パケット送受信
 */
static int SendRecv (
	int nFdSock,							/* in */
	const ST_REQUEST_INFO *pstRequestInfo,	/* in */
	ST_RESPONSE_INFO *pstResponseInfo		/* out */
)
{
	int nRtn = 0;
	size_t nSendLen = 0;
	unsigned char szSendBuff[SEND_BUFF_SIZE];
	unsigned char szRecvBuff[RECV_BUFF_SIZE];
	struct sockaddr_in stAddr;
	fd_set stFds;
	struct timeval stTimeval;


	memset( szSendBuff, 0x00, sizeof(szSendBuff) );

	/* ICMP Echo要求パケット作成 */
	if ( CreateIcmpEchoRequestPacket(
				pstRequestInfo, szSendBuff, sizeof(szSendBuff), &nSendLen ) < 0 ) {
		fprintf( stderr, "Err: CreateIcmpEchoRequestPacket() is failure.\n" );
		return -1;
	}

	memset( &stAddr, 0x00, sizeof(stAddr) );
	stAddr.sin_family = AF_INET;
	stAddr.sin_addr.s_addr = pstRequestInfo->nDestAddr;

	/* パケット送信 */
	nRtn = SendtoData( nFdSock, szSendBuff, nSendLen, &stAddr, sizeof(stAddr) );
	if ( nRtn < 0 ) {
		fprintf( stderr, "Err: SendtoData() is failure.\n" );
		return -1;
	}

	/* 送信時刻取得 RTTタイムカウント開始 */
	GetTimeString( &(pstResponseInfo->stTimeStr), 0 );


	FD_ZERO( &stFds );
	FD_SET( nFdSock, &stFds );
	stTimeval.tv_sec = TIMEOUT_SEC_VAL;
	stTimeval.tv_usec = TIMEOUT_USEC_VAL;

	/* 受信監視 */
	nRtn = select( nFdSock+1, &stFds, NULL, NULL, &stTimeval );
	if ( nRtn < 0 ) {
		perror( "select()" );
		return -1;

	} else if ( nRtn == 0 ) {
		/* タイムアウト RTTタイムカウント終了 */
		GetTimeString( &(pstResponseInfo->stTimeStr), 0 );
		return SELECT_TIMEOUT;
	}

	/* ここに来た場合 監視対象がレディである */


	/* 受信時刻取得 RTTタイムカウント終了 */
	GetTimeString( &(pstResponseInfo->stTimeStr), 0 );


	memset( szRecvBuff, 0x00, sizeof(szRecvBuff) );

	/* パケット受信 */
	nRtn = RecvData( nFdSock, szRecvBuff, sizeof(szRecvBuff), NULL );
	if ( nRtn < 0 ) {
		fprintf( stderr, "Err: RecvData() is failure.\n" );
		return -1;
	}
	
	nRtn = AnalyzePacket( szRecvBuff, pstResponseInfo );

	return nRtn;
}

/*
 * 結果表示
 */
static void PutsResult( const ST_RESPONSE_INFO *pstResponseInfo, const int *pnLoop )
{
	static int i = 0;

	if ( *pnLoop == 0 ) {
		i ++;
		fprintf (
			stdout,
			"TTL=%2d %s (%s)\n",
			i,
			!(pstResponseInfo->nFlagTimeout) ? pstResponseInfo->szReachedAddr : "xxx.xxx.xxx.xxx",
			pstResponseInfo->szName[0] ? pstResponseInfo->szName : "---"
		);
	}

	if ( !(pstResponseInfo->nFlagTimeout) ) {
		fprintf (
			stdout,
			"       [%s]  RTT: %ss\n",
			!(pstResponseInfo->nFlagTimeout) ? gpszIcmpType[pstResponseInfo->nIcmpType] : "xxx",
			pstResponseInfo->stTimeStr.szTimeMeasResS
		);

	} else {
		fprintf( stdout, "       timeout(%ss)\n", pstResponseInfo->stTimeStr.szTimeMeasResS );
	}

	if ( *pnLoop == LOOP_MAX -1 ) {

	}

	return;
}

/*
 * 送受信ループ
 */
static int SendRecvLoop( int nFdSock, uint32_t nSrcAddr, uint32_t nDestAddr )
{
	int nRtn = 0;
	int nCntRetryTimeout = TIMEOUT_RETRY_COUNT;
	int nCntTotalTimeout = 0;
	int nLoop = 0;
	int nFlagEchoReply = 0;
	ST_REQUEST_INFO stRequestInfo;
	ST_RESPONSE_INFO stResponseInfo;


	memset( &stRequestInfo, 0x00, sizeof(stRequestInfo) );

	stRequestInfo.nSrcAddr = nSrcAddr;
	stRequestInfo.nDestAddr = nDestAddr;
	stRequestInfo.nTtl = FIRST_TTL_VAL;

	while (1) {

		nLoop = 0;

		while( nLoop < LOOP_MAX ) {

			memset( &stResponseInfo, 0x00, sizeof(stResponseInfo) );

			nRtn = SendRecv( nFdSock, &stRequestInfo, &stResponseInfo );
			if ( nRtn < 0 ) {
				fprintf( stderr, "Err: SendRecv() is failure.\n" );
				return -1;

			} else if ( nRtn == ICMP_ECHOREPLY ) {
				PutsResult( &stResponseInfo, &nLoop );
				nFlagEchoReply = TRUE;

			} else if ( nRtn == ICMP_TIME_EXCEEDED ) {
				PutsResult( &stResponseInfo, &nLoop );
				nCntTotalTimeout = 0;

			} else if ( nRtn == SELECT_TIMEOUT ) {

				nCntRetryTimeout --;

				if ( nCntRetryTimeout == 0 ) {

					stResponseInfo.nFlagTimeout = TRUE;
					PutsResult( &stResponseInfo, &nLoop );

					nCntTotalTimeout ++;

					/* 初期値に戻す */
					stResponseInfo.nFlagTimeout = FALSE;
					nCntRetryTimeout = TIMEOUT_RETRY_COUNT;
				}

			} else {
				/* UNEXPECTED_ICMP */
				fprintf( stderr, "Warn: ICMP Type is [%s].\n",
										gpszIcmpType[stResponseInfo.nIcmpType] );
			}

			nLoop ++;
		}

		if ( nFlagEchoReply ) {
			break;
		}

		if ( nCntTotalTimeout > TOTAL_TIMEOUT_COUNT ) {
			break;
		}

		stRequestInfo.nTtl ++;
	}


	return 0;
}

/*
 * ソケット作成
 */
static int CreateSocket( void )
{
	int nFdSock = 0;
	int nFlag = SOCKOPT_ON;

	/*
	 * RAWソケット
	 * IPPROTO_RAWを指定すると強制的にIP_HDRINCLが有効になるので
	 * 以下のソケットオプションは不要になる
	 */
	if ( ( nFdSock = socket( AF_INET, SOCK_RAW, IPPROTO_ICMP ) ) < 0 ) {
		perror( "socket()" );
		return -1;
	}

	/*
	 * ソケットオプション
	 * 送信時IPヘッダを自前で作成する
	 * 受信時もIPヘッダが付いてくる
	 */
	if ( setsockopt( nFdSock, IPPROTO_IP, IP_HDRINCL, (void*)&nFlag, sizeof(nFlag) ) < 0 ) {
		perror( "setsockopt()" );
		return -1;
	}

	return nFdSock;
}

/*
 * メイン
 */
int main( int argc, char **argv )
{
	int nFdSock = 0;
	uint32_t nSrcAddr = INADDR_ANY;
	uint32_t nDestAddr = 0;
	char *pszHostname = NULL;


	if ( argc != 2 ) {
		fprintf( stderr, "Usage: %s hostname(IPaddr)\n", argv[0] );
		exit( EXIT_FAILURE );
	}


	pszHostname = argv[1];
	if ( GetIpAddr( pszHostname, &nDestAddr ) < 0 ) {
		fprintf( stderr, "Err: GetIpAddr() is failure.\n" );
		exit( EXIT_FAILURE );
	}


	if ( ( nFdSock = CreateSocket() ) < 0 ) {
		fprintf( stderr, "Err: CreateSocket() is failure.\n" );
		exit( EXIT_FAILURE );
	}


	if ( ( SendRecvLoop( nFdSock, nSrcAddr, nDestAddr ) ) < 0 ) {
		fprintf( stderr, "Err: SendRecvLoop() is failure.\n" );
		close( nFdSock );	
		exit( EXIT_FAILURE );
	}


	close( nFdSock );

	exit( EXIT_SUCCESS );
}
