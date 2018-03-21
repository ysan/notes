#include "mycommon.h"


/**
 * read()
 */
int ReadData( int nFd, unsigned char *pszBuff, size_t nBuffSize )
{
	int nReadSize = 0;
	int nDone = 0;

	while (1) {
		if ( nBuffSize == 0 ) {
			/* バッファ残りサイズチェック */
			/* バッファサイズと同サイズが受信された場合もここに来る */
			LOG_E( "Buffer over.\n" );
			return -2;
		}

		nReadSize = read( nFd, pszBuff, nBuffSize );
		if ( nReadSize < 0 ) {
			PERROR( "read()" );
			return -1;

		} else if ( nReadSize == 0 ) {
			/* ファイルエンド */
			break;

		} else {
			/* read ok */
			pszBuff += nReadSize;
			nBuffSize -= nReadSize;
			nDone += nReadSize;

			/* whileループ抜ける (標準入力時) */
			if ( (nFd == STDIN_FILENO) && (*(pszBuff-1) == '\n') ) {
				break;
			}
		}
	}

	return nDone;
}

/**
 * read()
 * パイプ用
 */
int ReadDataPipe( int nFd, unsigned char *pszBuff, size_t nBuffSize )
{
	int nRtn = 0;
	int nReadSize = 0;
	int nDone = 0;
	struct timeval stTv;
	fd_set stFds;

	while (1) {
		if ( nBuffSize == 0 ) {
			/* バッファ残りサイズチェック */
			/* バッファサイズと同サイズが受信された場合もここに来る */
			LOG_E( "Buffer over.\n" );
			return -2;
		}

		nReadSize = read( nFd, pszBuff, nBuffSize );
		if ( nReadSize < 0 ) {
			PERROR( "read()" );
			return -1;

		} else if ( nReadSize == 0 ) {
			/* ここには来ない? */
			break;

		} else {
			/* read ok */
			pszBuff += nReadSize;
			nBuffSize -= nReadSize;
			nDone += nReadSize;

			/*------------ 接続先からの受信が終了したか確認 ------------*/
			FD_ZERO( &stFds );
			FD_SET( nFd, &stFds );
			stTv.tv_sec = 0;
			stTv.tv_usec = 500000; /* タイムアウト 500mS */
									/* もしパイプからの出力が分割された場合、
									   一旦レディではなくなり、その後再びレディになると思われるので
									   受信続行の為、あえて間隔を空けてselectしている */
			nRtn = select( nFd+1, &stFds, NULL, NULL, &stTv );
			if ( nRtn < 0 ) {
				/* select()エラー */
				PERROR( "select()" );
				return -3;
			} else if ( nRtn == 0 ) {
				/* タイムアウト */
			}

			/* レディではなくなったらwhileループから抜ける */
			if ( !FD_ISSET( nFd, &stFds ) ) {
				break;
			}
			/*----------------------------------------------------------*/
		}
	}

	return nDone;
}

/**
 * write()
 */
int WriteData( int nFd, const unsigned char *pszBuff, size_t nLen )
{
	int nWriteSize = 0;
	int nDone = 0;

	while (1) {
		nWriteSize = write( nFd, pszBuff, nLen-nDone );
		if ( nWriteSize < 0 ) {
			PERROR( "write()" );
			return -1;

		} else {
			/* write ok */
			pszBuff += nWriteSize;
			nDone += nWriteSize;

			/* 全て送信できたらループ抜ける */
			if ( nLen == nDone ) {
				break;
			}
		}
	}

	return nDone;
}

/**
 * recv()
 */
int RecvData( int nFd, unsigned char *pszBuff, size_t nBuffSize, int *pnFlagConn )
{
	int nRtn = 0;
	int nRecvSize = 0;
	int nDone = 0;
	struct timeval stTv;
	fd_set stFds;

	while (1) {
		if ( nBuffSize == 0 ) {
			/* バッファ残りサイズチェック */
			/* バッファサイズと同サイズが受信された場合もここに来る */
			LOG_E( "Buffer over.\n" );
			return -2;
		}

		nRecvSize = recv( nFd, pszBuff, nBuffSize, 0 );
		if ( nRecvSize < 0 ) {
			PERROR( "recv()" );
			return -1;

		} else if ( nRecvSize == 0 ) {
			/* 相手との接続が切れた場合? or 正しく切断? */
			if ( pnFlagConn ) {
				*pnFlagConn = COMMUNICATION_DISCONNECT;
			}
			break;

		} else {
			/* recv ok */
			pszBuff += nRecvSize;
			nBuffSize -= nRecvSize;
			nDone += nRecvSize;

			/*------------ 接続先からの受信が終了したか確認 ------------*/
			FD_ZERO( &stFds );
			FD_SET( nFd, &stFds );
			stTv.tv_sec = 0;
			stTv.tv_usec = 0; /* タイムアウト 0S */
			nRtn = select( nFd+1, &stFds, NULL, NULL, &stTv );
			if ( nRtn < 0 ) {
				/* select()エラー */
				PERROR( "select()" );
				return -3;
			} else if ( nRtn == 0 ) {
				/* タイムアウト */
			}

			/* レディではなくなったらwhileループから抜ける */
			if ( !FD_ISSET( nFd, &stFds ) ) {
				break;
			}
			/*----------------------------------------------------------*/
		}
	}

	return nDone;
}

/**
 * recv()
 * HTTPリクエスト用
 */
int RecvDataHttpReq( int nFd, char *pszBuff, size_t nBuffSize, int *pnFlagConn )
{
	int nRtn = 0;
	int nRecvSize = 0;
	int nDone = 0;
	char *pszBuffStat = pszBuff;
	struct timeval stTv;
	fd_set stFds;

	while (1) {
		if ( nBuffSize == 0 ) {
			/* バッファ残りサイズチェック */
			/* バッファサイズと同サイズが受信された場合もここに来る */
			LOG_E( "Buffer over.\n" );
			return -2;
		}

		nRecvSize = recv( nFd, pszBuff, nBuffSize, 0 );
		if ( nRecvSize < 0 ) {
			PERROR( "recv()" );
			return -1;

		} else if ( nRecvSize == 0 ) {
			/* 相手との接続が切れた場合? or 正しく切断? */
			if ( pnFlagConn ) {
				*pnFlagConn = COMMUNICATION_DISCONNECT;
			}
			break;

		} else {
			/* recv ok */
			pszBuff += nRecvSize;
			nBuffSize -= nRecvSize;
			nDone += nRecvSize;

			/*------------ 接続先からの受信が終了したか確認 ------------*/
			FD_ZERO( &stFds );
			FD_SET( nFd, &stFds );
			stTv.tv_sec = 0;
			stTv.tv_usec = 0; /* タイムアウト 0S */
			nRtn = select( nFd+1, &stFds, NULL, NULL, &stTv );
			if ( nRtn < 0 ) {
				/* select()エラー */
				PERROR( "select()" );
				return -3;
			} else if ( nRtn == 0 ) {
				/* タイムアウト */
			}

			/* レディではなくなったらwhileループから抜ける */
			if ( !FD_ISSET( nFd, &stFds ) ) {

				/* リクエストメッセージ末尾のチェック */

				/* 改行コードが CRLFの場合 */
				if ( strstr( pszBuffStat, "\r\n\r\n" ) ) {
					break;
				} else {
					/* 改行コードが LFの場合 */
					if ( strstr( pszBuffStat, "\n\n" ) ) {
						break;
					} else {
						/* まだ末尾が受信できていない 受信続行 */
						continue;
					}
				}
			}
			/*----------------------------------------------------------*/
		}
	}

	return nDone;
}

/**
 * recvfrom()
 */
int RecvfromData (
	int nFd,
	unsigned char *pszBuff,
	size_t nBuffSize,
	struct sockaddr_in *pstAddr,
	socklen_t *pnAddrlen,
	int *pnFlagConn
)
{
	int nRtn = 0;
	int nRecvSize = 0;
	int nDone = 0;
	struct timeval stTv;
	fd_set stFds;

	while (1) {
		if ( nBuffSize ==0 ) {
			/* バッファ残りサイズチェック */
			/* バッファサイズと同サイズが受信された場合もここに来る */
			LOG_E( "Buffer over.\n" );
			return -2;
		}

		nRecvSize = recvfrom( nFd, pszBuff, nBuffSize, 0, (struct sockaddr*)pstAddr, pnAddrlen );
		if ( nRecvSize < 0 ) {
			PERROR( "recvfrom()" );
			return -1;

		} else if( nRecvSize == 0 ){
			/* 相手との接続が切れた場合? or 正しく切断? */
			if ( pnFlagConn ) {
				*pnFlagConn = COMMUNICATION_DISCONNECT;
			}
			break;

		} else {
			/* recvfrom ok */
			pszBuff += nRecvSize;
			nBuffSize -= nRecvSize;
			nDone += nRecvSize;

			/*------------ 接続先からの受信が終了したか確認 ------------*/
			FD_ZERO( &stFds );
			FD_SET( nFd, &stFds );
			stTv.tv_sec = 0;
			stTv.tv_usec = 0; /* タイムアウト 0S */
			nRtn = select( nFd+1, &stFds, NULL, NULL, &stTv );
			if ( nRtn < 0 ) {
				/* select()エラー */
				PERROR( "select()" );
				return -3;
			} else if ( nRtn == 0 ) {
				/* タイムアウト */
			}

			/* レディではなくなったらwhileループから抜ける */
			if ( !FD_ISSET( nFd, &stFds ) ) {
				break;
			}
			/*----------------------------------------------------------*/
		}
	}

	return nDone;
}

/**
 * send()
 */
int SendData( int nFd, const unsigned char *pszBuff, size_t nLen )
{
	int nSendSize = 0;
	int nDone = 0;

	while(1){
		nSendSize = send( nFd, pszBuff, nLen-nDone, 0 );
		if ( nSendSize < 0 ) {
			PERROR( "send()" );
			return -1;

		} else {
			/* send ok */
			pszBuff += nSendSize;
			nDone += nSendSize;

			/* 全て送信できたらループ抜ける */
			if ( nLen == nDone ) {
				break;
			}
		}
	}

	return nDone;
}

/**
 * sendto()
 */
int SendtoData ( 
	int nFd,
	const unsigned char *pszBuff,
	size_t nLen,
	struct sockaddr_in* pstAddr,
	socklen_t nAddrlen
)
{
	int nSendSize = 0;
	int nDone = 0;

	while (1) {
		nSendSize = sendto( nFd, pszBuff, nLen-nDone, 0, (struct sockaddr*)pstAddr, nAddrlen );
		if ( nSendSize < 0 ) {
			PERROR( "sendto()" );
			return -1;

		} else {
			/* sendto ok */
			pszBuff += nSendSize;
			nDone += nSendSize;

			/* 全て送信できたらループ抜ける */
			if ( nLen == nDone ) {
				break;
			}
		}
	}

	return nDone;
}

/**
 * 標準入力を非カノニカルモードに設定する。
 *
 * param[in]   nFd   対象ファイルディスクリプタ
 *                    (ここは標準入力(STDIN_FILENO)に)
 *
 * retern  正常   0
 * retern  異常  -1
 */
int SetNonCanon( int nFd )
{
	struct termios stTermios;

	/* 現ターミナル属性を取得 */
	if ( tcgetattr( nFd, &stTermios ) < 0 ) {
		PERROR( "tcsetattr()" );
		return -1;
	}

	/* 各フラグ値を非カノニカルモードに変更 */
	stTermios.c_lflag &= ~ICANON;
	stTermios.c_cc[ VMIN ] = 1;
	stTermios.c_cc[ VTIME ] = 0;

	/* ターミナル属性を設定 */
	if ( tcsetattr( nFd, TCSANOW, &stTermios ) < 0 ) {
		PERROR( "tcsetattr() ICANON" );
		return -1;
	}

	return 0;
}

/**
 * ホスト名->IPアドレス変換 (DNS正引き)
 *
 * getaddrinfo()でホスト名->IPアドレス変換する。
 * etc/host.conf、/etc/nsswitch.conf の設定によりDNS問い合わせするかどうか決まる。
 * ( /etc/hostsを先に参照、DNS問い合わせ自体しない等。)
 *
 * param[in]   *pszHostname   対象ホスト名文字列ポインタ
 * param[out]  *pnAddr        IPアドレス出力ポインタ
 *
 * retern  正常   0
 * retern  異常  -1
 */
int GetIpAddr( const char *pszHostname, uint32_t *pnAddr )
{
	int nRtn = 0;
	struct addrinfo stAddrInfoHints;
	struct addrinfo *pstAddrInfoRes = NULL;
	struct sockaddr_in *pstSockAddrIn = NULL;

	memset( &stAddrInfoHints, 0x00, sizeof(struct addrinfo) );

	/* IPv4 */
	stAddrInfoHints.ai_family = AF_INET;

	if ( getaddrinfo( pszHostname, NULL, &stAddrInfoHints, &pstAddrInfoRes ) ) {
		LOG_E( "getaddrinfo(): [%d:%s]\n", nRtn, gai_strerror(nRtn) );
		return -1;
	}

	/* sockaddr_in構造体でキャスト */
	pstSockAddrIn = (struct sockaddr_in*)pstAddrInfoRes->ai_addr;

	/* ネットワークバイトオーダーのまま返却 */
	*pnAddr = pstSockAddrIn->sin_addr.s_addr;

	freeaddrinfo( pstAddrInfoRes );

	return 0;
}

/**
 * IPアドレス->名前変換 (DNS逆引き)
 *
 * getnameinfo()でIPアドレス->名前変換する。
 *
 * param[in]   nAddr       対象IPアドレス(※ネットワークバイトオーダで指定)
 * param[out]  *pszName    名前変換結果文字列ポインタ
 * param[in]   nBuffSize   名前変換結果文字列サイズ
 *
 * retern  正常   0
 * retern  異常  -1
 */
int GetReverseIp( uint32_t nAddr, char *pszName, size_t nBuffSize )
{
	int nRtn = 0;
	struct sockaddr_in strAddr;

	memset( &strAddr, 0x00, sizeof(strAddr) );
	strAddr.sin_family = AF_INET;
	strAddr.sin_addr.s_addr = nAddr;

	nRtn = getnameinfo(
				(const struct sockaddr*)&strAddr,
				(socklen_t)sizeof(struct sockaddr),
				pszName,
				nBuffSize-1, /* 終端分考慮 */
				NULL,
				0,
				NI_NAMEREQD
	);
	if ( ( nRtn ) && ( nRtn != EAI_NONAME ) ) {
		LOG_E( "getnameinfo(): [%d:%s]\n", nRtn, gai_strerror(nRtn) );
		return -1;
	}

	return 0;
}

/**
 * パケットを受信した日時を取得
 *
 * recv(),recvfrom()でパケットを受信した日時を取得する。
 * (YYYY-MM-DD HH:MM:SS.SSS 形式)
 * リエントラント。
 *
 * param[in]   nFd        対象ファイルディスクリプタ
 * param[out]  *pszTime   出力バッファ(文字列)ポインタ
 * param[out]  nBuffSize  出力バッファサイズ
 *
 * retern  正常   0
 * retern  異常  -1
 */
int GetRecvTime( int nFd, char *pszTime, size_t nBuffSize )
{
	struct tm *pstTmLocal = NULL;
	struct tm stTmLocalTmp;
	struct timeval stTv;

	if ( !pszTime ) {
		LOG_E( "Err: Output buffer is null.\n" );
		return -1;
	}

	if ( ioctl( nFd ,SIOCGSTAMP ,&stTv ) < 0 ) {
		PERROR( "ioctl()" );
		return -1;
	}

	pstTmLocal = localtime_r( &stTv.tv_sec, &stTmLocalTmp ); /* リエントラント版 */

	snprintf (
		pszTime,
		nBuffSize,
		"%04d-%02d-%02d %02d:%02d:%02d.%03ld",
		pstTmLocal->tm_year+1900,
		pstTmLocal->tm_mon+1,
		pstTmLocal->tm_mday,
		pstTmLocal->tm_hour,
		pstTmLocal->tm_min,
		pstTmLocal->tm_sec,
		stTv.tv_usec/1000
	);

	return 0;
}

/**
 * システム現在時刻を出力
 * ログマクロでも使用
 */
void PutsTime( void )
{
	time_t timer;
	struct tm *pstTmLocal = NULL;
	struct tm stTmLocalTmp;

	timer = time( NULL );
	pstTmLocal = localtime_r( &timer, &stTmLocalTmp ); /* リエントラント版 */

	fprintf (
		stdout,
		"%02d/%02d %02d:%02d:%02d",
		pstTmLocal->tm_mon+1,
		pstTmLocal->tm_mday,
		pstTmLocal->tm_hour,
		pstTmLocal->tm_min,
		pstTmLocal->tm_sec
	);
}

struct dump16 {
	unsigned char n1;
	unsigned char n2;
	unsigned char n3;
	unsigned char n4;
	unsigned char n5;
	unsigned char n6;
	unsigned char n7;
	unsigned char n8;
	unsigned char n9;
	unsigned char n10;
	unsigned char n11;
	unsigned char n12;
	unsigned char n13;
	unsigned char n14;
	unsigned char n15;
	unsigned char n16;
};
/**
 * 16進ダンプ
 *
 * 第1引数で指定したバッファデータを16進数表示でダンプする。
 * 第3引数でascii文字表示をするか指定。
 *
 * param[in]  *pszBuff   対象バッファ
 * param[in]  nBuffLen   バッファ長(※データの詰まったサイズ)
 * param[in]  nAsciiOn   ascii文字表示フラグ(0意外の値)
 */
void Dumper( const unsigned char *pszBuff, size_t nBuffLen, int nAsciiOn )
{
	if ((!pszBuff) || (nBuffLen <= 0)) {
		return;
	}

	int i = 0;
	int j = 0;
	int k = 0;
	struct dump16 *pstDump16 = NULL;

	while ( nBuffLen >= 16 ) {

		fprintf( stdout, "%s0x%08x: ", DUMP_PUTS_OFFSET, i );

		/* 16進dump */
		pstDump16 = (struct dump16*)pszBuff;
		fprintf (
			stdout,
			"%02x%02x %02x%02x %02x%02x %02x%02x "\
			"%02x%02x %02x%02x %02x%02x %02x%02x",
			pstDump16->n1, pstDump16->n2,
			pstDump16->n3, pstDump16->n4,
			pstDump16->n5, pstDump16->n6,
			pstDump16->n7, pstDump16->n8,
			pstDump16->n9, pstDump16->n10,
			pstDump16->n11, pstDump16->n12,
			pstDump16->n13, pstDump16->n14,
			pstDump16->n15, pstDump16->n16
		 );

		/* ascii文字表示 */
		if ( nAsciiOn ) {
			fprintf( stdout, "  " );
			k = 0;
			while( k < 16 ){
				/* 制御コード系は'.'で代替 */
				fprintf (
					stdout,
					"%c",
					(*(pszBuff+k)>0x1f) && (*(pszBuff+k)<0x7f) ? *(pszBuff+k) : '.'
				);
				k ++;
			}
		}

		fprintf( stdout, "\n" );

		pszBuff += 16;
		i += 16;
		nBuffLen -= 16;
	}

	/* 余り分(16byte満たない分) */
	if ( nBuffLen ) {

		/* 16進dump */
		fprintf( stdout, "%s0x%08x: ", DUMP_PUTS_OFFSET, i );
		while ( j < 16 ) {
			if ( j && !(j%2) ) {
				fprintf( stdout, " " );
			}

			if ( j < nBuffLen ) {
				fprintf( stdout, "%02x", *(pszBuff+j) );
			} else {
				fprintf( stdout, "  " );
			}

			j ++;
		}

		/* ascii文字表示 */
		if ( nAsciiOn ) {
			fprintf( stdout, "  " );
			k = 0;
			while ( k < nBuffLen ) {
				/* 制御コード系は'.'で代替 */
				fprintf( stdout, "%c",
						(*(pszBuff+k)>0x20) && (*(pszBuff+k)<0x7f) ? *(pszBuff+k) : '.' );
				k ++;
			}
		}

		fprintf( stdout, "\n" );
	}
}

/**
 * float値->2進数表記(浮動小数点)変換
 *
 * 第1引数で指定したfloat値を、
 * 第2引数で指定した文字列バッファに2進数表記(浮動小数点)で格納する。
 * 最後にNULL文字を付け足す。
 *
 * param[in]   *pnVal    float値ポインタ
 * param[out]  *pszRtn   出力文字列ポインタ
 * param[in]   nRtnSize  出力文字列サイズ
 */
void Float2BitString( const float *pnVal, char *pszRtn, size_t nRtnSize )
{
	if ( !pnVal || !pszRtn || (nRtnSize < 32+1+1+1) ) {
		return;
	}

	int i = 32 -1;

	/* 終端文字分マージンとる */
	while ( (i > -1) && (nRtnSize > 1) ) {

		*pszRtn = ( ( *(unsigned int*)pnVal >> i ) & 0x01 ) + '0';
		pszRtn ++;
		nRtnSize --;

		if ( (i == 31) || (i==23) ) {
			*pszRtn = ',';
			pszRtn ++;
			nRtnSize --;
		}

		i --;
	}

	*pszRtn = 0x00;
}

/**
 * システムの現在時刻取得
 *
 * YYYY-MM-DD HH:MM:SS.SSS 形式で時刻取得。
 * 同時に前回呼び出されてからの時間間隔計測も行う。
 * リエントラント。
 *
 * param[out] *pstTimeStr  時刻データ文字列構造体返却ポインタ
 * param[in]  nIdx         時間間隔計測系統インデックス ( 0 ～ TIME_STR_IDX_MAX-1 の値 )
 */
void GetTimeString( ST_TIME_STR *pstTimeStr, int nIdx )
{
 	int nHour = 0;
 	int nMin = 0;
	int nSec = 0;
	static int nFlgFirstTime[ TIME_STR_IDX_MAX ] = { 0 };
 	struct tm *pstTmLocal = NULL;
 	struct tm stTmLocalTmp;
	struct timeval stTv;
	struct timeval stTvDiff;
	static struct timeval stTvBefore[ TIME_STR_IDX_MAX ];

	if ( !pstTimeStr ) {
		return;
	}

	if ( nIdx >= TIME_STR_IDX_MAX ) {
		nIdx = TIME_STR_IDX_MAX;
	}

 	/* マイクロ秒で現在時刻取得 */
 	gettimeofday( &stTv, NULL );
	memcpy( &(pstTimeStr->stTv), &stTv, sizeof(struct timeval) );
 	pstTmLocal = localtime_r( &stTv.tv_sec, &stTmLocalTmp ); /* リエントラント版 */

 	snprintf (
		pstTimeStr->szTime,	
		TIME_STR_SIZE,
		"%04d-%02d-%02d %02d:%02d:%02d.%03ld",
		pstTmLocal->tm_year+1900,
		pstTmLocal->tm_mon+1,
		pstTmLocal->tm_mday,
		pstTmLocal->tm_hour,
		pstTmLocal->tm_min,
		pstTmLocal->tm_sec,
		stTv.tv_usec/1000
	);

 	/* 以下 経過時間計測部分 */

	if ( !nFlgFirstTime[nIdx] ) {
 		/* 初回 */

		/* HH:MM:SS.SSS 形式 */
		snprintf( pstTimeStr->szTimeMeasResM, TIME_STR_MEAS_RES_M_SIZE, "%02d:%02d:%02d.%03d", 0, 0, 0, 0 );

		/* SSSSSS.SSS 形式 */
		snprintf( pstTimeStr->szTimeMeasResS, TIME_STR_MEAS_RES_S_SIZE, "%d.%03d", 0, 0 );

		stTvBefore[ nIdx ].tv_sec = stTv.tv_sec;
		stTvBefore[ nIdx ].tv_usec = stTv.tv_usec;

		nFlgFirstTime[nIdx] ++;

	} else {
		/* 2回目以降 */

		stTvDiff.tv_sec = stTv.tv_sec - stTvBefore[nIdx].tv_sec;
		stTvDiff.tv_usec = stTv.tv_usec - stTvBefore[nIdx].tv_usec;

		/* マイクロ秒差分がマイナスの時 1の位を繰り下げて小数点第1位に加える */
		if ( stTvDiff.tv_usec < 0 ) {
			stTvDiff.tv_sec -= 1;
			stTvDiff.tv_usec += 1000000;
		}
      
		nHour = stTvDiff.tv_sec / 3600;
		nMin = ( stTvDiff.tv_sec % 3600 ) / 60;
		nSec = ( stTvDiff.tv_sec % 3600 ) % 60;

		/* HH:MM:SS.SSS 形式 */
		snprintf (
			pstTimeStr->szTimeMeasResM,
			TIME_STR_MEAS_RES_M_SIZE,
			"%02d:%02d:%02d.%03ld",
			nHour,
			nMin,
			nSec,
			stTvDiff.tv_usec/1000
		);      

		/* SSSSSS.SSS 形式 */
		snprintf (
			pstTimeStr->szTimeMeasResS,
			TIME_STR_MEAS_RES_S_SIZE,
			"%ld.%03ld",
			stTvDiff.tv_sec,
			stTvDiff.tv_usec/1000
		);
	
		stTvBefore[ nIdx ].tv_sec = stTv.tv_sec;
		stTvBefore[ nIdx ].tv_usec = stTv.tv_usec;
	}
}

/**
 * 文字列の改行コードLFを削除
 * CRLFの場合 CRも削除する
 *
 * param[in/out]  *pszBuff  データ入出力ポインタ
 */
void DeleteLF( char *pszBuff )
{
	if ( !pszBuff ) {
		return;
	}

	if( *( pszBuff + ( strlen(pszBuff) -1 ) ) == '\n' ){

		/* 改行コードLF削除 */
		*( pszBuff + ( strlen(pszBuff) -1 ) ) = '\0';

		/* CRLFの場合 CRも削除 */
		if( *( pszBuff + ( strlen(pszBuff) -1 ) ) == '\r' ){
			*( pszBuff + ( strlen(pszBuff) -1 ) ) = '\0';
		}
	}
}

/**
 * 文字列の先頭スペース削除
 *
 * param[in/out]  *pszBuff  データ入出力ポインタ
 */
void DeleteHeadSp( char *pszBuff )
{
	if ( !pszBuff ) {
		return;
	}

	int i = 0;

	while (1) {

		if ( *pszBuff == ' ' ) {
			if ( (int)strlen(pszBuff) > 1 ) {
				i = 0;
				while ( *(pszBuff+i) ) {
					*(pszBuff+i) = *(pszBuff+i+1);
					i ++;
				}

			} else {
				/* 全文字スペースだった */
				*pszBuff = 0x00;
			}
		} else {
			break;
		}
	}
}

/**
 * 文字列の末尾スペース削除
 *
 * param[in/out]  *pszBuff  データ入出力ポインタ
 */
void DeleteTailSp( char *pszBuff )
{
	if ( !pszBuff ) {
		return;
	}

	int nLen = (int)strlen(pszBuff);
	if (nLen == 0) {
		return;
	}

	/* 終端の1つ前にするため */
	nLen --;

	while ( nLen >= 0 ) {

		if ( *(pszBuff+nLen) == ' ' ) {
			*(pszBuff+nLen) = 0x00;
		} else {
			break;
		}
		nLen --;
	}
}

/**
 * 2進数文字列->数値変換(byte)
 *
 * param[in]  *p  2進数文字列ポインタ 
 *                null終端されていること前提
 *
 * retern  byte数値
 */
uint8_t Bit2Byte( const char *p )
{
	if (!p) {
		return 0;
	}

	const char *pTmp = p;
	int nPos = 0;
	int nLen = 0;
	uint8_t nByte = 0;


	/* 数値チェック */
	while (*p) {
		if ((*p != '0') && (*p != '1')) {
			/* error */
			return 0;
		}
		p ++;
		nLen ++;
	}

	/* 下位8bit分のみを変換対象とする */
	if (nLen >= 8) {
		nLen = 8 -1;
	}

	p = pTmp;

	nLen --;
	nPos = 0;
	while (nLen >= 0) {
		nByte |= (*(p+nLen)-'0') << nPos;
		nPos ++;
		nLen --;
	}

	return nByte;
}
