#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

#include "mycommon.h"
#include "xmodem.h"
#include "serial_com.h"


/*
 * プロトタイプ宣言
 */
static void DebugWrite( ST_XMODEM_INFO *pstXmodemInfo, int n );


/*
 * XMODEM送信処理
 */
int SendXmodem( int nFdCom, unsigned char *pszBuff )
{
	int i = 0;
	int j = 0;
	int k = 0;
	int nRetry = 0;
	int nRtn = 0;
	int nFd = 0;
	int nCksumTmp = 0;
	int nAdd = 0;
	int nError = 0;
	unsigned char c;
	unsigned char szReadBuff[ XMODEM_READ_BUFF_SIZE ];
	unsigned char *pszReadBuff = szReadBuff;
	char szFilePath[ PATH_MAX ];
	ST_XMODEM_INFO stXmodemInfo[ XMODEM_READ_BUFF_SIZE / XMODEM_DATA_SIZE ];


	/* 送信ファイル指定 */
	fprintf( stdout, "Info: XMODEM send -- Please specify a file to send.\n" );
	fprintf( stdout, "__: " );
	fflush( stdout );

	nRtn = ReadData( STDIN_FILENO, (unsigned char*)szFilePath, sizeof(szFilePath) );
	if ( nRtn < 0 ) {
		fprintf( stderr,"Err: ReadData() is failure.\n" );
		return -1;
	}

	/* 改行コード削除 */
	DeleteLF( szFilePath );

	/* ファイルオープン */
	if ( ( nFd = open( szFilePath, O_RDONLY ) ) < 0 ) {
		perror( "open()" );
		return -1;
	}

	/* 読み込みバッファを0x1a(XMODEM_EOF)で埋めておく */
	memset( szReadBuff, XMODEM_EOF, sizeof(szReadBuff) );

	/* ファイル読み込み */
	nRtn = ReadData( nFd, szReadBuff, sizeof(szReadBuff) );
	if ( nRtn < 0 ) {
		fprintf( stderr, "Err: ReadData() is failure.\n" );
		close( nFd );
		return -1;
	}

	close( nFd );


	i = 0;
	nAdd = (nRtn % XMODEM_DATA_SIZE) > 0 ? 1 : 0;

	/* 読み込んだデータをXMODEM構造体にセット */
	while ( i < nRtn / XMODEM_DATA_SIZE + nAdd ) {

		stXmodemInfo[ i ].cHeader = XMODEM_SOH;
		stXmodemInfo[ i ].cBlockNo = (unsigned short)( i + 1 ) & 0xff;
		stXmodemInfo[ i ].cBlockNoNgtv = ~(stXmodemInfo[ i ].cBlockNo);
		memcpy( stXmodemInfo[ i ].szData, pszReadBuff, XMODEM_DATA_SIZE );

		/* チェックサム */
		while ( j < XMODEM_DATA_SIZE ) {
			nCksumTmp += stXmodemInfo[ i ].szData[ j ];
			j ++;
		}
		stXmodemInfo[ i ].cCksum = (unsigned char)nCksumTmp;
		j = 0;
		nCksumTmp = 0;

		i ++;
		pszReadBuff += XMODEM_DATA_SIZE;
	}

	fprintf( stdout, "Info: XMODEM send -- %d block %dbytes(%dbytes)\n", i, i*XMODEM_SIZE, nRtn );


	/* NAK(0x15)を待つ */

	fprintf( stdout, "Info: XMODEM send -- waiting NAK...\n" );

	while (1) {
		c = RecvByte( nFdCom, &nError );
		if ( nError ) {
			fprintf( stderr, "Err: RecvData() is failure.\n" );
			return -1;
		}

		if ( c == XMODEM_NAK ) {
			break;
		}
	}


	k = 0;

	/* データ送信 */
	while ( k < i ) {

		/* ブロック単位の送信 */
		if ( SendSequence( nFdCom, (unsigned char*)&stXmodemInfo[ k ], XMODEM_SIZE ) < 0 ) {
			fprintf( stderr, "Err: SendSequence() is failure.\n" );
			return -1;
		}
//		DebugWrite( &stXmodemInfo[ k ], k );

		/* ACK or NAK の受信 */
		c = RecvByte( nFdCom, &nError );
		if ( nError ) {
			fprintf( stderr, "Err: RecvData() is failure.\n" );
			return -1;
		}

		if ( c == XMODEM_ACK ) {
			/* ACKの場合 次ブロックを送信 */
			fprintf( stdout, "Info: XMODEM send -- block %d  -Result ACK-\n", k+1 );

			/* 次ブロックへ */			
			k ++;

			nRetry = 0;

		} else if ( c == XMODEM_NAK ) {
			/* NAKの場合 現ブロックを再送 */
			fprintf( stdout, "Info: XMODEM send -- block %d  -Result NAK-\n", k+1 );

			/* 再送回数終了 */
			if ( nRetry == XMODEM_SEND_RSLT_RETRY_CNT ) {
				fprintf( stdout, "Err: XMODEM send -- Send retry count over\n" );
				c = XMODEM_CAN;
				if ( SendByte( nFdCom, &c ) < 0 ) {
					fprintf( stderr, "Err: SendByte() is failure.\n" );
					return -1;
				}

				return -2;				
			}

			nRetry ++;

		} else {
			/*
			 * ACK でも NAK でもない場合エラーとする
			 * CANを送信
			 */
			fprintf( stdout, "Err: XMODEM send -- block %d  -Result Irregular[0x%02x]-\n", k+1, c );
			c = XMODEM_CAN;
			if ( SendByte( nFdCom, &c ) < 0 ) {
				fprintf( stderr, "Err: SendByte() is failure.\n" );
				return -1;
			}

			return -2;
		}
	}

	/* 全ブロックを送信した */

	/* EOT(0x04)を送信 */
	c = XMODEM_EOT;
	if ( SendByte( nFdCom, &c ) < 0 ) {
		fprintf( stderr, "Err: SendByte() is failure.\n" );
		return -1;
	}

	/* ACK(0x06)を待つ */
	c = RecvByte( nFdCom, &nError );
	if ( nError ) {
		fprintf( stderr, "Err: RecvData() is failure.\n" );
		return -1;
	}

	if ( c == XMODEM_ACK ) {
		/* ACKの場合 終了 */
		fprintf( stdout, "Info: XMODEM send -- Normal end\n" );
		return 0;

	} else {
		/*
		 * ACKではない場合エラーとする
		 * CANを送信
		 */
		fprintf( stdout, "Err: XMODEM send -- Abnormal end\n" );
		c = XMODEM_CAN;
		if ( SendByte( nFdCom, &c ) < 0 ) {
			fprintf( stderr, "Err: SendByte() is failure.\n" );
			return -1;
		}

		return -2;
	}

	return 0;
}

/*
 * デバッグファイル出力
 */
static void DebugWrite( ST_XMODEM_INFO *pstXmodemInfo, int n )
{
	int nFd = 0;
	char szFile[32];
	memset( szFile, 0x00, sizeof(szFile) );
	snprintf( szFile, sizeof(szFile), "send_XMODEM_%02d", n );
	nFd = open( szFile, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	write( nFd, pstXmodemInfo, XMODEM_SIZE );
	close( nFd );
}
