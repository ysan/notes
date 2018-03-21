#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


/*
 * 定数定義
 */
#ifndef TRUE
#define TRUE	(1)
#endif
#ifndef FALSE
#define FALSE	(0)
#endif

typedef enum {
	EN_MCOM_FALSE = FALSE,
	EN_MCOM_TRUE = TRUE
} EN_MCOM_BOOL;

#ifndef BOOL
#define BOOL EN_MCOM_BOOL
#endif


enum {
	COMMUNICATION_CONNECT = 0,
	COMMUNICATION_DISCONNECT
};


#define DUMP_PUTS_OFFSET	"  "

enum {
	DUMP_PUTS_ASCII_OFF = FALSE,
	DUMP_PUTS_ASCII_ON
};


#define TIME_STR_SIZE				(4+1+2+1+2+1+2+1+2+1+2+1+3+1)
#define TIME_STR_MEAS_RES_M_SIZE	(2+1+2+1+2+1+3+1)
#define TIME_STR_MEAS_RES_S_SIZE	(6+1+3+1)
#define TIME_STR_IDX_MAX			(16)



/*
 * 簡易ログマクロ
 */
#define LOG_I( fmt, ... ) { \
	fprintf( stdout, "I " ); \
	PutsTime(); \
	fprintf( stdout, " [%s,%s(),%d] ", __FILE__, __func__, __LINE__ ); \
	fprintf( stdout, fmt, ##__VA_ARGS__ ); \
	fflush( stdout );\
}
#define LOG_W( fmt, ... ) { \
	fprintf( stdout, "\033[33m" ); \
	fprintf( stdout, "W " ); \
	PutsTime(); \
	fprintf( stdout, " [%s,%s(),%d] ", __FILE__, __func__, __LINE__ ); \
	fprintf( stdout, fmt, ##__VA_ARGS__ ); \
	fprintf( stdout, "\033[39m" ); \
	fflush( stdout );\
}
#define LOG_E( fmt, ... ) { \
	fprintf( stdout, "\033[1m" ); \
	fprintf( stdout, "\033[31m" ); \
	fprintf( stdout, "E " ); \
	PutsTime(); \
	fprintf( stdout, " [%s,%s(),%d] ", __FILE__, __func__, __LINE__ ); \
	fprintf( stdout, fmt, ##__VA_ARGS__ ); \
	fprintf( stdout, "\033[39m" ); \
	fprintf( stdout, "\033[0m" ); \
	fflush( stdout );\
}
#define PERROR( fmt ) { \
	fprintf( stdout, "\033[1m" ); \
	fprintf( stdout, "\033[35m" ); \
	fprintf( stdout, "E " ); \
	PutsTime(); \
	fprintf( stdout, " [%s,%s(),%d] ", __FILE__, __func__, __LINE__ ); \
	fprintf( stdout, "%s: %s\n", fmt, sys_errlist[ errno ] ); \
	fprintf( stdout, "\033[39m" ); \
	fprintf( stdout, "\033[0m" ); \
	fflush( stdout );\
}


/*
 * 型定義
 */
#ifndef uint8_t
typedef unsigned char uint8_t;
#endif
#ifndef uint16_t
typedef unsigned short uint16_t;
#endif
#ifndef uint32_t
typedef unsigned int uint32_t;
#endif
#ifndef uint64_t
typedef unsigned long int uint64_t;
#endif


typedef struct {
	struct timeval stTv;
	char szTime[ TIME_STR_SIZE ];
	char szTimeMeasResM[ TIME_STR_MEAS_RES_M_SIZE ];
	char szTimeMeasResS[ TIME_STR_MEAS_RES_S_SIZE ];
} ST_TIME_STR;


/*
 * 外部宣言
 */
#ifdef __cplusplus
extern "C"
{
#endif
extern int ReadData( int nFd, unsigned char *pszBuff, size_t nBuffSize );
extern int ReadDataPipe( int nFd, unsigned char *pszBuff, size_t nBuffSize );
extern int WriteData( int nFd, const unsigned char *pszBuff, size_t nLen );
extern int RecvData( int nFd, unsigned char *pszBuff, size_t nBuffSize, int *pnFlagConn );
extern int RecvDataHttpReq( int nFd, char *pszBuff, size_t nBuffSize, int *pnFlagConn );
extern int RecvfromData (
	int nFd,
	unsigned char *pszBuff,
	size_t nBuffSize,
	struct sockaddr_in *pstAddr,
	socklen_t *pnAddrlen,
	int *pnFlagConn
);
extern int SendData( int nFd, const unsigned char *pszBuff, size_t nLen );
extern int SendtoData (
	int nFd,
	const unsigned char *pszBuff,
	size_t nLen,
	struct sockaddr_in* pstAddr,
	socklen_t nAddrlen
);
extern int SetNonCanon( int nFd );
extern int GetIpAddr( const char *pszHostname, uint32_t *pnAddr );
extern int GetReverseIp( uint32_t nAddr, char *pszName, size_t nBuffSize );
extern int GetRecvTime( int nFd, char *pszTime, size_t nBuffSize );
extern void PutsTime( void );
extern void Dumper( const unsigned char *pszBuff, size_t nBuffLen, int nAsciiOn );
extern void Float2BitString( const float *pnVal, char *pszRtn, size_t nRtnSize );
extern void GetTimeString( ST_TIME_STR *pstTimeStr, int nIdx );
extern void DeleteLF( char *pszBuff );
extern void DeleteHeadSp( char *pszBuff );
extern void DeleteTailSp( char *pszBuff );
extern uint8_t Bit2Byte( const char *p );

#ifdef __cplusplus
}
#endif

#endif
