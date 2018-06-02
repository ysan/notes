#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#include "mycommon.h"
#include "serial_com.h"
#include "xmodem.h"


/*
 * 変数宣言
 */
static int gnFlagExecSendXmodem;
static int gnFlagDump;
static int gnPos;
static int gnBuffFlag;
static int gnStdinCancel;

/*
 * プロトタイプ宣言 
 */
static int InitCom( int nFd, int nBrate );
//static int RecvString( int nFd, char *pszBuff, size_t nBuffSize );
static int RecvSequence( int nFd, unsigned char *pszBuff, size_t nBuffSize );
static int SendString( int nFd, const char *pszBuff );
static int RecvSend( int nFd );
static int CheckInnerCommand( const char *pszComm );

/*
 * メイン
 */
int main( int argc, char **argv )
{
	int nFd = 0;


	if ( argc != 2 ) {
		fprintf( stderr, "Usage: %s SerialDevice\n", argv[0] );
		exit( EXIT_FAILURE );
	}

	/* 内部変数初期化 */
	gnFlagExecSendXmodem = FALSE;
	gnFlagDump = FALSE;	
	gnPos = 0;
	gnBuffFlag = FALSE;
	gnStdinCancel = FALSE;

	if ( ( nFd = open( argv[1], O_RDWR ) ) < 0 ) {
		perror( "open()" );
		exit( EXIT_FAILURE );
	}

	if ( InitCom( nFd, BAUDRATE ) < 0 ) {
		fprintf( stderr, "Err: InitCom() is failure.\n" );
		close( nFd );
		exit( EXIT_FAILURE );
	}

	/* 標準入力を非カノニカルモードに設定 */
	if ( SetNonCanon( STDIN_FILENO ) < 0 ) {
		fprintf( stderr, "Err: SetNonCanon() is failure.\n" );
		close( nFd );
		exit( EXIT_FAILURE );
	}

	if ( RecvSend( nFd ) < 0 ) {
		fprintf( stderr, "Err: RecvSend() is failure.\n" );
		close( nFd );
		exit( EXIT_FAILURE ); 
	}


	close( nFd );

	exit( EXIT_SUCCESS );
}

/*
 * シリアルポート初期化
 */
static int InitCom( int nFd, int nBrate )
{
	int nBrateSet = 0;
	struct termios stTty;

	/* ボーレートの選択 */
	switch ( nBrate ) {
		case 2400:
			nBrateSet = B2400;
			break;
		case 4800:
			nBrateSet = B4800;
			break;
		case 9600:
			nBrateSet = B9600;
			break;
		case 19200:
			nBrateSet = B19200;
			break;
		case 38400:
			nBrateSet = B38400;
			break;
		case 57600:
			nBrateSet = B57600;
			break;
		case 115200:
			nBrateSet = B115200;
			break;
		default:
			fprintf( stderr,"Err: Please set up a baudrate correctly.\n" );
			return -1;
			break;
	}

	/* 端末のフォアグラウンド プロセスグループの設定 */
	tcsetpgrp( nFd, getpgrp() );

	/* termios構造体の取得 */
	tcgetattr( nFd, &stTty );

	/* 入出力ボーレートの設定 */
	/* tcsetattr()が呼び出されるまでは有効にはならない */
	cfsetispeed( &stTty, nBrateSet );
	cfsetospeed( &stTty, nBrateSet );

	/*
	 * 入力エコー無効
	 * 非カノニカルモード
	 * 実装依存の入力処理？無効
	 * シグナル発生無効
	 */
	stTty.c_lflag &= ~( ECHO | ICANON | IEXTEN | ISIG );

	/*
	 * BREAK信号有効
	 * 入力のCRをNLに置き換えない
	 * 入力パリティチェック無効
	 * 8bit目を落とさない
	 * 出力のXON/XOFFフロー制御を無効
	 */
	stTty.c_iflag &= ~( BRKINT | ICRNL | INPCK | ISTRIP | IXON );

	/*
	 * 出力パリティ付加しない 入力パリティチェック無効
	 * ストップビット1
	 */
	stTty.c_cflag &= ~( PARENB | CSTOPB );

	/* データビット数を8にする */
	stTty.c_cflag |= CS8;

	/* 実装に依存した出力処理？無効 */
	stTty.c_oflag &= ~( OPOST );

	/* 1文字毎に受信する（readのブロックを1文字ごとにする）*/
	stTty.c_cc[VTIME] = 0; /* 非カノニカル読み込み時のタイムアウト時間 */
	stTty.c_cc[VMIN] = 1; /* 非カノニカル読み込み時の最小文字数 */

	/*
	 * 最後のプロセスがデバイスをクローズした後 モデムの制御線を low にする
	 * モデムの制御線を無視
	 */
	stTty.c_cflag |= HUPCL | CLOCAL ;

	/* 上記のパラメータを設定 TCSANOW(ただちに変更が有効に) */
	if ( tcsetattr( nFd, TCSANOW, &stTty ) < 0 ) {
		perror( "tcsetattr()" );
		return -1;
	}

	return 0;
}

/*
 * 1バイト受信
 */
unsigned char RecvByte( int nFd, int *pnFlagError )
{
	int nRtn = 0;
	unsigned char c = 0;

	if ( !pnFlagError ) {
		fprintf( stderr, "Warn: pnFlagError is null.\n" );
	}

	if ( pnFlagError ) {
		*pnFlagError = FALSE;
	}

	nRtn = read( nFd, &c, 1 );
	if ( nRtn < 0 ) {
		perror( "read()" );
		if ( pnFlagError ) {
			*pnFlagError = TRUE;
		}
	}

	/* 受信した1byteを返却 */
	return c;
}

#if 0
/*
 * 文字列受信
 * (文字列がLFで終わっていること前提)
 */
static int RecvString( int nFd, char *pszBuff, size_t nBuffSize )
{
	int nCnt = 0;
	int nError = FALSE;
	unsigned char c = 0;

	while ( nBuffSize > 0 ) {

		c = RecvByte( nFd, &nError );
		if ( nError ) {
			fprintf( stderr, "Err: RecvByte() is failure.\n" );
			return -1;
		}

		*pszBuff = c;
		pszBuff ++;
		nCnt ++;
		nBuffSize --;

		if ( c == '\n' ) {
			/* LFが来たら受信ループを抜ける */
			break;
		}
	}

	return nCnt;
}
#endif

/*
 * バイト列受信
 */
static int RecvSequence( int nFd, unsigned char *pszBuff, size_t nBuffSize )
{
	int nCnt = 0;
	int nRtn = 0;
	int nError = FALSE;
	unsigned char c = 0;
	struct timeval strTv;
	fd_set strFds;

	while ( nBuffSize > 0 ) {

		c = RecvByte( nFd, &nError );
		if ( nError ) {
			fprintf( stderr, "Err: RecvByte() is failure.\n" );
			return -1;
		}

		*pszBuff = c;
		pszBuff ++;
		nCnt ++;
		nBuffSize --;

		/*------------ 接続先からの受信が終了したか確認 ------------*/
		FD_ZERO( &strFds );
		FD_SET( nFd, &strFds );
		strTv.tv_sec = 0;
		strTv.tv_usec = 10000; /* タイムアウト 10mS */
		nRtn = select( nFd+1, &strFds, NULL, NULL, &strTv );
		if( nRtn < 0 ){
			/* select()エラー */
			perror( "select()" );
			return -2;
		} else if( nRtn == 0 ){
			/* タイムアウト */
		}

		/* レディではなくなったらwhileループから抜ける */
		if( !FD_ISSET( nFd, &strFds ) ){
			break;
		}
		/*----------------------------------------------------------*/
	}

	return nCnt;
}

/*
 * 1バイト送信
 */
int SendByte( int nFd, const unsigned char *c )
{
	if ( write( nFd, c, 1 ) < 0 ) {
		perror( "write()" );
		return -1;
	}

	return 0;
}

/*
 * 文字列送信
 * (NULL終端されていること前提)
 */
static int SendString( int nFd, const char *pszBuff )
{
	int nCnt = 0;

	while ( *pszBuff ) {

		if ( SendByte( nFd, (unsigned char*)pszBuff ) < 0 ) {
			fprintf( stderr, "Err: SendByte() is failure.\n" );
			return -1;
		}

		pszBuff ++;
		nCnt ++;
	}

	return nCnt;
}

/*
 * バイト列送信
 * XMODEMで使用
 */
int SendSequence( int nFd, const unsigned char *pszBuff, size_t nBuffSize )
{
	int nCnt = 0;

	while ( nBuffSize > 0 ) {

		if ( SendByte( nFd, pszBuff ) < 0 ) {
			fprintf( stderr, "Err: SendByte() is failure.\n" );
			return -1;
		}

		pszBuff ++;
		nCnt ++;
		nBuffSize --;
	}

	return nCnt;
}

/*
 * 送受信
 */
static int RecvSend( int nFd )
{
	int nRtn = 0;
	int nRtnCk = 0;
	int nError = FALSE;
	int i = 0;
	unsigned char c =0;
	char szStdin[ SERIAL_COM_STDIN_BUFF_SIZE ];
	unsigned char szBuff[ SERIAL_COM_RECV_BUFF_SIZE ];
	fd_set stFds;

	while (1) {

		/* プロンプト出す */
		if ( !gnBuffFlag ) {
			fprintf( stdout, "> " );
			fflush( stdout );
			gnBuffFlag = TRUE;
		}

		/* 入力キャンセル中の場合 */
		if ( gnStdinCancel ) {
			fprintf( stdout, "> " );
			fprintf( stdout, "%s", szStdin );
			fflush( stdout );
			gnStdinCancel = FALSE;
		}

		FD_ZERO( &stFds );
		FD_SET( STDIN_FILENO, &stFds );
		FD_SET( nFd, &stFds );

		nRtn = select( nFd+1, &stFds, NULL, NULL, NULL );
		if ( nRtn < 0 ) {
			perror( "select()" );
		}

		/* 監視中のファイルディスクリプタがレディになった */

		/* 標準入力がレディ */
		if ( FD_ISSET( STDIN_FILENO, &stFds ) ) {

			c = RecvByte( STDIN_FILENO, &nError );
			if ( nError ) {
				fprintf( stderr,"Err: RecvByte() is failure.\n" );
				return -1;
			}

			if ( c == 0x7f ) {
				/* backspaceの処理 */
				fprintf( stdout, "\b \b\b \b" );	// bsのゴミ削除 環境依存かも...
				fflush( stdout );
				if ( gnPos > 0 ) {
					gnPos --;
					szStdin[ gnPos ] = 0x00;
					fprintf( stdout,"\b \b" );
					fflush( stdout );
				}
			} else {
				/* backspace以外の文字 */

				szStdin[ gnPos ] = (char)c;

				if ( szStdin[ gnPos ] != '\n' ) {
					/* LFでない場合バッファを継続 */

					gnBuffFlag = TRUE;

					if ( gnPos >= SERIAL_COM_STDIN_BUFF_SIZE -3 ) {
						fprintf( stderr,"Err: Buffer(stdin) is over.\n" );
						return -1;
					}

					gnPos ++;

				} else {
					/* LF来た 1lineを取り込み処理する */

					nRtnCk = CheckInnerCommand( szStdin );
					if ( nRtnCk ) {
						/* 内部コマンドである */

						if ( gnFlagExecSendXmodem ) {

							nRtn = SendXmodem( nFd, (unsigned char*)szStdin );
							if ( nRtn == -1 ) {
								fprintf( stderr,"Err: SendXmodem() is failure.\n" );
								gnFlagExecSendXmodem = FALSE;
								return -1;

							} else if ( nRtn == -2 ) {
								/* XMODEM送信処理エラー */
								fprintf( stderr,"Err: SendXmodem() is failure.\n" );
								gnFlagExecSendXmodem = FALSE;

							} else {
								/* 処理なし */
								gnFlagExecSendXmodem = FALSE;
							}
						}

 					} else {
						/*
						 * 内部コマンドではないので
						 * そのままシリアルで送信
						 */
						nRtn = SendString( nFd, szStdin );
						if ( nRtn < 0 ) {
							fprintf( stderr,"Err: SendString() is failure.\n" );
							return -1;
						}
					}

					memset( szStdin, 0x00, sizeof(szStdin) );
					gnPos = 0;
					gnBuffFlag = FALSE;

//					fprintf( stdout, "[%d]bytes sent.\n\n", gnPos +1 );
				}
			}

		}

		/* シリアルポートからの受信がレディ */
		if ( FD_ISSET( nFd, &stFds ) ) {

			memset( szBuff, 0x00, sizeof(szBuff) );
#if 0
			nRtn = ReadString( nFd, szBuff, sizeof(szBuff) );
			if ( nRtn < 0 ) {
				fprintf( stderr,"Err: ReadString() is failure.\n" );
				return -1;
			}
#endif
			nRtn = RecvSequence( nFd, szBuff, sizeof(szBuff) );
			if ( nRtn < 0 ) {
				fprintf( stderr,"Err: RecvSequence() is failure.\n" );
				return -1;
			}

			/* 入力中のものを消す(入力キャンセル) */
			if ( gnBuffFlag ) {
				gnStdinCancel = TRUE;
				fprintf( stdout, "\r" );
				for ( i = 0; i < gnPos +2; i ++ ) {		// +2 はプロンプト
					fprintf( stdout, " " );
				}
				fprintf( stdout, "\r" );
				fflush( stdout );
			}

			if ( !gnFlagDump ) {
				fprintf( stdout, "%s", szBuff );
			} else {
				Dumper( szBuff, nRtn, DUMP_PUTS_ASCII_ON );
			}

//			fprintf( stdout, "[%d]bytes received.\n\n", nRtn );

		}

	}

	return 0;
}

/*
 * 内部コマンド解析
 */
static int CheckInnerCommand( const char *pszComm )
{
	char szBuff[ SERIAL_COM_COMMAND_BUFF_SIZE ];

	memcpy( szBuff, pszComm, sizeof(szBuff) );

	/* 改行削除 */
	DeleteLF( szBuff );

	if (
		!memcmp( szBuff, COMMAND_XMODEM_SEND, strlen(COMMAND_XMODEM_SEND) ) &&
		strlen(COMMAND_XMODEM_SEND) == strlen(szBuff)
	) {
		gnFlagExecSendXmodem = TRUE;
		return TRUE;

	} else if (
		!memcmp( szBuff, COMMAND_DUMP_ON, strlen(COMMAND_DUMP_ON) ) &&
		strlen(COMMAND_DUMP_ON) == strlen(szBuff)
	) {
		gnFlagDump = TRUE;
		return TRUE;

	} else if (
		!memcmp( szBuff, COMMAND_DUMP_OFF, strlen(COMMAND_DUMP_OFF) ) &&
		strlen(COMMAND_DUMP_OFF) == strlen(szBuff)
	) {
		gnFlagDump = FALSE;
		return TRUE;

	} else {
		/* 処理なし */
		/* シリアルで送信する */
	}

	return FALSE;
}
