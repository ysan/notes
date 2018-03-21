#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <math.h>

#include "mycommon.h"
#include "ntpmoni.h"
#include "util.h"


/*
 * Variables
 */
static volatile sig_atomic_t gnSigFlag = 0;
static struct serv_info *gpstServInfoList = NULL;
static pthread_mutex_t gMutex;   
static pthread_cond_t gCond;  
static EN_COND_WAIT_KIND genCondWaitKind;

/*
 * Prototypes
 */



/**
 * シグナルハンドラ関数
 */
static void SigHandler( int nSig )
{
	gnSigFlag = (volatile sig_atomic_t)nSig;
}

/**
 * シグナルハンドラ設定
 */
static BOOL SetSigHandle( int nSigKind, void *pFunc )
{
	struct sigaction stSigAct;

	memset( &stSigAct, 0x00, sizeof(stSigAct) );  
	stSigAct.sa_handler = pFunc;

	if ( sigaction( nSigKind, &stSigAct, NULL ) < 0 ) {
		PERROR( "sigaction()" );
		return FALSE;
	}

	return TRUE;
}

/**
 * ソケット生成
 */
static int CreateSocket( void )
{
	int nFdSock = 0;
	struct timeval stTimeout;

	if ( ( nFdSock = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 ) {
		PERROR( "socket()" );
		return -1;
	}

	/* 受信タイムアウト設定 */
	memset( &stTimeout, 0x00, sizeof(stTimeout) );
	stTimeout.tv_sec  = 5;
	stTimeout.tv_usec = 0;
	if ( setsockopt( nFdSock, SOL_SOCKET, SO_RCVTIMEO, &stTimeout, sizeof(stTimeout) ) < 0 ) {
        PERROR( "setsockopt()" );
		return -1;
	}

	return nFdSock;
}

/**
 * サーバリストにソケット割り当て
 */
static void AttachSocket( void )
{
	int i = 0;

	while (1) {
		if (((gpstServInfoList+i)->nFdSock = CreateSocket()) < 0) {
			LOG_E( "CreateSocket() is failure.\n" );
			LOG_W( "continue...\n" );
			(gpstServInfoList+i)->isInitValid = FALSE;
		}

		if ((gpstServInfoList+i)->isInitTerm) {
			break;
		}

		i ++;
	}
}

/**
 * NTPタイム文字列変換
 */
static BOOL ConvNtp2Str( const struct bit64_fixed_point *pstBit64, char *pszTime, size_t nBuffSize )
{
	long nNtpRes;
	struct tm stLocaltime;
	struct tm *pstLocaltime = NULL;
	double nFraction;
	char szTmp[ TIMESTAMP_STR_LEN ];

	if ( (!pstBit64) || (!pszTime) ) {
		return FALSE;
	}

	if ( pstBit64->nInteger == 0 ) {
		*pszTime = '-';
		return TRUE;
	}

	memset( szTmp, 0x00, sizeof(szTmp) );

	/*
	 * 整数部
	 * 1970.1.1 からの秒数に変換
	 * NTPタイムは1900.1.1 0時との相対的な差を秒単位で表している
	 * Unixタイムは1970.1.1 0時からはじまるので70年分の秒を減算する
	 */
    nNtpRes = ntohl(pstBit64->nInteger) - 2208988800UL;

	pstLocaltime = localtime_r( (const time_t*)&nNtpRes, &stLocaltime );
	if ( !pstLocaltime ) {
		PERROR( "localtime_r()" );
		strncpy( pszTime, "localtime_r() err", nBuffSize-1 );
		return FALSE;
	}

	/* yyyy.mm.dd hh:mm:ss形式 */
	strftime( szTmp, TIMESTAMP_STR_LEN -1, "%Y.%m.%d %H:%M:%S", pstLocaltime );


	/* 小数部 */
//	nFraction = 
//	( ( ntohl(pstBit64->nFraction) >> 24 ) & 0xff ) * 1/pow( 2.0,  8 ) + 
//	( ( ntohl(pstBit64->nFraction) >> 16 ) & 0xff ) * 1/pow( 2.0, 16 ) + 
//	( ( ntohl(pstBit64->nFraction) >>  8 ) & 0xff ) * 1/pow( 2.0, 24 ) + 
//	(   ntohl(pstBit64->nFraction)         & 0xff ) * 1/pow( 2.0, 32 ) ;
	nFraction = Fixed2DoubleOnlyFraction( ntohl(pstBit64->nFraction) );

	/* yyyy.mm.dd hh:mm:ss.ssssss形式 */
	snprintf( pszTime, nBuffSize, "%s.%06d", szTmp, (int)(nFraction*1000000) );

	return TRUE;
}

/**
 * NTPパケットチェック
 */
static BOOL CheckPacket( const struct ntp_packet *pstNtpPacket, struct serv_info *pstServInfo )
{
	char szTime[ TIMESTAMP_STR_LEN ];
 
	LOG_I( "nLI_VN_MD=[0x%02x]\n", pstNtpPacket->nLI_VN_MD );
	LOG_I( " -- LeapIndicator=[0x%02x]\n", pstNtpPacket->nLI_VN_MD >> 6 );
	LOG_I( " -- VersionNo=[0x%02x]\n", (pstNtpPacket->nLI_VN_MD>>3) & 0x07 );
	LOG_I( " -- Mode=[0x%02x]\n", pstNtpPacket->nLI_VN_MD & 0x07 );
	LOG_I( "nStratum=[0x%02x]\n", pstNtpPacket->nStratum );
	LOG_I( "nPoll=[0x%02x]\n", pstNtpPacket->nPoll );
	LOG_I( "nPrecision=[0x%02x]\n", pstNtpPacket->nPrecision );
	LOG_I( "nRootDelay=[0x%08x]\n", ntohl(pstNtpPacket->nRootDelay) );
	LOG_I( "nRootDispersion=[0x%08x]\n", ntohl(pstNtpPacket->nRootDispersion) );
	LOG_I( "nReferenceIdentifier=[0x%08x]\n", ntohl(pstNtpPacket->nRefIdentifier) );


	/* 以下NTPサーバから取得した時刻を現地時間に変換 */

	/* stRefTimestamp -------------------------------------------*/
	memset( szTime, 0x00, sizeof(szTime) );
	if ( !ConvNtp2Str( &(pstNtpPacket->stRefTimestamp), szTime, TIMESTAMP_STR_LEN -1 ) ) {
		LOG_E( "ConvNtp2Str() is failure.\n" );
		/* 続行 */
	}

	strncpy( pstServInfo->szRefTimestamp, szTime, TIMESTAMP_STR_LEN -1 );

	LOG_I (
		"nRefTimestamp=[0x%08x,0x%08x]--[%s]\n",
		ntohl(pstNtpPacket->stRefTimestamp.nInteger),
		ntohl(pstNtpPacket->stRefTimestamp.nFraction),
		szTime
	);

	/* stOrgTimestamp -------------------------------------------*/
	memset( szTime, 0x00, sizeof(szTime) );
	if( !ConvNtp2Str( &(pstNtpPacket->stOrgTimestamp), szTime, TIMESTAMP_STR_LEN -1 ) ) {
		LOG_E( "ConvNtp2Str() is failure.\n" );
		/* 続行 */
	}

	strncpy( pstServInfo->szOrgTimestamp, szTime, TIMESTAMP_STR_LEN -1 );

	LOG_I (
		"nOrgTimestamp=[0x%08x,0x%08x]--[%s]\n",
		ntohl(pstNtpPacket->stOrgTimestamp.nInteger),
		ntohl(pstNtpPacket->stOrgTimestamp.nFraction),
		szTime
	);

	/* stRecvTimestamp -------------------------------------------*/
	memset( szTime, 0x00, sizeof(szTime) );
	if ( !ConvNtp2Str( &(pstNtpPacket->stRecvTimestamp), szTime, TIMESTAMP_STR_LEN -1 ) ) {
		LOG_E( "ConvNtp2Str() is failure.\n" );
		/* 続行 */
	}

	strncpy( pstServInfo->szRecvTimestamp, szTime, TIMESTAMP_STR_LEN -1 );

	LOG_I (
		"nRecvTimestamp=[0x%08x,0x%08x]--[%s]\n",
		ntohl(pstNtpPacket->stRecvTimestamp.nInteger),
		ntohl(pstNtpPacket->stRecvTimestamp.nFraction),
		szTime
	);

	/* stTransTimestamp -------------------------------------------*/
	memset( szTime, 0x00, sizeof(szTime) );
	if ( !ConvNtp2Str( &(pstNtpPacket->stTransTimestamp), szTime, TIMESTAMP_STR_LEN -1 ) ) {
		LOG_E( "ConvNtp2Str() is failure.\n" );
		/* 続行 */
	}

	strncpy( pstServInfo->szTransTimestamp, szTime, TIMESTAMP_STR_LEN -1 );

	LOG_I (
		"nTransTimestamp=[0x%08x,0x%08x]--[%s]\n",
		ntohl(pstNtpPacket->stTransTimestamp.nInteger),
		ntohl(pstNtpPacket->stTransTimestamp.nFraction),
		szTime
	);


	return TRUE;
}

/**
 * 送信タイムスタンプ取得
 */
static void GetTransTime( struct bit64_fixed_point *pstBit64 )
{
	ST_TIME_STR stTime;
	double nFraction;

	GetTimeString( &stTime, 0 );

	/* 整数部 */
	pstBit64->nInteger = htonl( stTime.stTv.tv_sec + 2208988800UL );

	/* 小数部 */
	nFraction = ((double)stTime.stTv.tv_usec) / 1000000;
	pstBit64->nFraction = htonl( Double2FixedOnlyFraction( nFraction ) );
}

/**
 * NTP Requestパケット作成
 */
static void CreateRequestPacket( struct ntp_packet *pstNtpPacket )
{
	/*
	 * 上位bitから
	 * LI: 00  警告無し
	 * VN: 011 通常のNTP
	 * MD: 011 クライアント
	 */
	pstNtpPacket->nLI_VN_MD = Bit2Byte( "00011011" );

	/* 送信タイムスタンプ取得 */
	GetTransTime( &(pstNtpPacket->stTransTimestamp) ); 
}

/**
 * NTP Requestパケット送信 
 */
static BOOL SendPacket( const struct serv_info *pstServInfo )
{
	int nSendSize = 0;
	struct sockaddr_in stSendAddr;
	struct ntp_packet stNtpPacket;


	/* Requestパケット作成 */
	memset( &stNtpPacket, 0x00, sizeof(stNtpPacket) );
	CreateRequestPacket( &stNtpPacket );


	memset( &stSendAddr, 0x00, sizeof(stSendAddr) );
	stSendAddr.sin_family = AF_INET;
	stSendAddr.sin_port = htons( NTP_SERVER_PORT );
	stSendAddr.sin_addr.s_addr = pstServInfo->nAddr;

	nSendSize = SendtoData( pstServInfo->nFdSock, (unsigned char*)&stNtpPacket,
										sizeof(stNtpPacket), &stSendAddr, sizeof(stSendAddr) );
	if( nSendSize < 0 ){
		LOG_E( "SendtoData() is failure.\n" );
		return FALSE;
	}

	LOG_I( "nSendSize=[%d]\n", nSendSize );
//	Dumper( (const unsigned char*)&stNtpPacket, sizeof(stNtpPacket), TRUE );

	return TRUE;
}

/**
 * NTP Replyパケット受信
 */
static BOOL RecvPacket( struct serv_info *pstServInfo )
{
	int nRecvSize = 0;
	unsigned char szBuff[ 128 ];
	struct sockaddr_in stRecvAddr;
	socklen_t nSockLen = sizeof(stRecvAddr);

	memset( szBuff, 0x00, sizeof(szBuff) );

	memset( &stRecvAddr, 0x00, sizeof(stRecvAddr) );
	stRecvAddr.sin_family = AF_INET;
	stRecvAddr.sin_port = htons( NTP_SERVER_PORT );
	stRecvAddr.sin_addr.s_addr = pstServInfo->nAddr;

	nRecvSize = RecvfromData( pstServInfo->nFdSock, szBuff, sizeof(szBuff), &stRecvAddr, &nSockLen, NULL );
	if ( nRecvSize < 0 ) {
		LOG_E( "RecvfromData() is failure\n" );
		return FALSE;
	}

	LOG_I( "nRecvSize=[%d]\n", nRecvSize );
	Dumper( szBuff, nRecvSize, TRUE );

	if ( nRecvSize != sizeof(struct ntp_packet) ) {
		LOG_W( "PacketSize=[%d]\n", nRecvSize );
	}

	if ( !CheckPacket( (struct ntp_packet*)szBuff, pstServInfo ) ) {
		LOG_E( "CheckPacket() is failure.\n" );
		return FALSE;
	}

	return TRUE;
}

/**
 * スレッド関数
 */
static void *ThreadFunc( void *pThArg )
{
	struct serv_info *pstServInfo = (struct serv_info*)pThArg;

	while (1) {

		pstServInfo->enState = EN_STATE_THREAD_WAIT;

		pthread_mutex_lock( &gMutex );
		pthread_cond_wait( &gCond, &gMutex );
		pthread_mutex_unlock( &gMutex );

		pstServInfo->enState = EN_STATE_THREAD_EXE;

		if ( genCondWaitKind == EN_COND_WAIT_KIND_CONN_START ) {

			if ( !SendPacket( pstServInfo ) ) {
				LOG_E( "SendPacket() is failure.\n" );
				/* 続行 */

			} else {
				if ( !RecvPacket( pstServInfo ) ) {
					LOG_E( "RecvPacket() is failure.\n" );
					/* 続行 */
				}
			}

		} else {

			/* Thread end */
			break;
		}

	}

	pstServInfo->enState = EN_STATE_THREAD_END;

	return NULL;
}

/**
 * スレッド生成
 */
static BOOL CreateThread( void )
{
	int i = 0;
	pthread_t nThId;
	pthread_attr_t attr;

	if ( pthread_attr_init( &attr ) != 0 ) {
		PERROR( "pthread_attr_init()" );
		return FALSE;
	}

	if ( pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED ) != 0 ) {
		PERROR( "pthread_attr_setdetachstate()" );
		return FALSE;
	}

	/* スレッド生成 */
	while (1) {	

		nThId = 0;

		(gpstServInfoList+i)->enState = EN_STATE_THREAD_CREATE;

		if ( (gpstServInfoList+i)->isInitValid ) {
			if ( pthread_create( &nThId, &attr, ThreadFunc, (void*)(gpstServInfoList+i) ) != 0 ) {
				PERROR( "pthread_create()" );
				LOG_W( "continue...\n" );
				/* 続行 */

			} else {
				(gpstServInfoList+i)->nThId = nThId;
			}
		}

		if ( (gpstServInfoList+i)->isInitTerm ) {
			break;
		}

		i ++;
	}

	return TRUE;
}

/**
 * スレッド終了
 * (消滅確認)
 */
static void FinalizeThread( void )
{
	int i = 0;
	BOOL isEnd = TRUE;

	while (1) {
		i = 0;
		isEnd = TRUE;

		while (1) {
			if ( (gpstServInfoList+i)->isInitValid && ((gpstServInfoList+i)->nThId > 0) ) {

				LOG_I (
					"idx:[%d] nThId:[%lu] enState:[%d]\n",
					i,
					(gpstServInfoList+i)->nThId,
					(gpstServInfoList+i)->enState
				);

				if ( (gpstServInfoList+i)->enState != EN_STATE_THREAD_END ) {
					isEnd = FALSE;
					break;
				}
			}

			if ( (gpstServInfoList+i)->isInitTerm ) {
				break;
			}

			i ++;
		}

		if (isEnd) {
			/* 全スレッド終了 */
			LOG_I( "All thread end.\n" );
			break;

		} else {
			/*
			 * まだ生存しているスレッドあり
			 * 1sec待ち 念の為broadcast
			 */
			LOG_W( "Thread[%lu] is still exist. Wait 1sec...\n", (gpstServInfoList+i)->nThId );
			sleep(1);

			genCondWaitKind = EN_COND_WAIT_KIND_THREAD_END;
			pthread_cond_broadcast( &gCond );

			continue;
		}
	}
}

/**
 * usage
 */
static void Usage( const char *p )
{
	fprintf( stdout, "Usage: %s\n", p );
	fprintf( stdout, "    ---------------------------------\n" );
	fprintf( stdout, "    Ctrl+\\ (SIGQUIT) --> Send NTP Request\n" );
	fprintf( stdout, "    Ctrl+c (SIGINT) --> End Process\n" );
	fprintf( stdout, "    ---------------------------------\n" );
}

/**
 * main
 */
int main( int argc, char **argv )
{
	Usage( argv[0] );


	/* init values */
	gnSigFlag = 0;
	gpstServInfoList = NULL;
	pthread_mutex_init( &gMutex, NULL );
	pthread_cond_init( &gCond, NULL );
	genCondWaitKind = EN_COND_WAIT_KIND_CONN_START;


	if (!SetSigHandle( SIGQUIT, SigHandler )) {
		LOG_E( "SetSigHandle()\n" );
		exit( EXIT_FAILURE );
	}

	if (!SetSigHandle( SIGINT, SigHandler )) {
		LOG_E( "SetSigHandle()\n" );
		exit( EXIT_FAILURE );
	}


	if (!(gpstServInfoList = GetServerList())) {
		LOG_E( "GetServerList() is failure.\n" );
		DestroyServerList( gpstServInfoList );
		exit( EXIT_FAILURE );
	}

	AttachSocket();

	if (!CreateThread()) {
		LOG_E( "CreateThread() is failure.\n" );
		DestroyServerList( gpstServInfoList );
		exit( EXIT_FAILURE );
	}


	/* main loop */
	while (1) {

		/* 割り込まれるまで寝る */
		LOG_I( "Waiting interrupt...\n" );
		sleep(10);

		if (gnSigFlag == SIGQUIT) {

			genCondWaitKind = EN_COND_WAIT_KIND_CONN_START;
			pthread_cond_broadcast( &gCond );
			gnSigFlag = 0;

		} else if (gnSigFlag == SIGINT) {

			genCondWaitKind = EN_COND_WAIT_KIND_THREAD_END;
			pthread_cond_broadcast( &gCond );
			gnSigFlag = 0;
			break;
		}

	}

	FinalizeThread();

	DestroyServerList( gpstServInfoList );

	LOG_I( "process end.\n" );


	exit( EXIT_SUCCESS );
}
