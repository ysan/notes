#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>


#include "mycommon.h"
#include "sslfunc.h"


SSL *gpstrSsl; /* SSLオブジェクト */
BIO *gpstrBio; /* BIOオブジェクト */


const INT_FUNC_INIT_SSL pFuncInitSsl[3] = {
	InitPlainBio,	// idx=0
	InitSslBio,		// idx=1
	InitSsl,		// 未使用
};


/*
 * SSL/TSL初期化
 * (ソケット作成後SSLオブジェクトにて通信)
 */
int InitSsl( int nFd )
{
	int nRtn = 0;
	SSL_CTX *pstrCtx = NULL;


	SSL_load_error_strings();
	ERR_load_SSL_strings();
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();


	/* SSL/TLS の初期化 */
	SSL_library_init();

	/* 使用するプロトコルを決める */
	pstrCtx = SSL_CTX_new( SSLv23_client_method() );
	if ( !pstrCtx ) {
		ERR_print_errors_fp( stderr );
		return -1;
	}

	/* SSLオブジェクトを生成 */
	gpstrSsl = SSL_new( pstrCtx );
	if ( !gpstrSsl ) {
		ERR_print_errors_fp( stderr );
		return -1;
	}

	/* SSLv2を禁止にするオプション */
	SSL_set_options( gpstrSsl, SSL_OP_NO_SSLv2 ); // DDDDDDDDDDDDD リターンコードチェック未

	/* ハンドシェイク要求をバックグラウンド処理するオプションを設定 */
	SSL_set_mode( gpstrSsl, SSL_MODE_AUTO_RETRY );


	/* ソケットとSSLオブジェクトを結びつける */
	nRtn = SSL_set_fd( gpstrSsl, nFd );
	if( !nRtn ){
		ERR_print_errors_fp( stderr );
		return -1;
	}

	/* サーバとハンドシェイクを実施 */
	nRtn = SSL_connect( gpstrSsl );
	if ( nRtn != 1 ) {
		ERR_print_errors_fp( stderr );
		return -1;
	}

	return 0;
}

/*
 * SSL/TSL初期化
 * (ソケット作成後BIOオブジェクトにて通信)
 */
int InitSslBio( int nFd )
{
	int nRtn = 0;
	SSL_CTX *pstrCtx = NULL;


	SSL_load_error_strings();
	ERR_load_BIO_strings();
	ERR_load_SSL_strings();
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();


	/* SSL/TLS の初期化 */
	SSL_library_init();

	/* 使用するプロトコルを決める */
	pstrCtx = SSL_CTX_new( SSLv23_client_method() );
	if ( !pstrCtx ) {
		ERR_print_errors_fp( stderr );
		return -1;
	}

	/* BIO オブジェクトをSSLオブジェクトのチェインとして生成 */
	gpstrBio = BIO_new_ssl_connect( pstrCtx );
		if ( !gpstrBio ) {
		ERR_print_errors_fp( stderr );
		return -1;
	}

	/* BIOオブジェクトからSSLオブジェクトへのポインターを取得する */
	BIO_get_ssl( gpstrBio, &gpstrSsl );


	/* SSLv2を禁止にするオプション */
	SSL_set_options( gpstrSsl, SSL_OP_NO_SSLv2 ); // DDDDDDDDDDDDDDD リターンコードチェック未

	/* ハンドシェイク要求をバックグラウンド処理するオプションを設定 */
	SSL_set_mode( gpstrSsl, SSL_MODE_AUTO_RETRY );


	/* ソケットとSSLオブジェクトを結びつける */
	nRtn = SSL_set_fd( gpstrSsl, nFd );
	if( !nRtn ){
		ERR_print_errors_fp( stderr );
		return -1;
	}

	/* サーバとハンドシェイクを実施 */
	nRtn = BIO_do_connect( gpstrBio );
	if ( nRtn != 1 ) {
		ERR_print_errors_fp( stderr );
		return -1;
	}

	return 0;
}

/*
 * ソケットとBIOオブジェクトを結びつける
 */
int InitPlainBio( int nFd )
{
	ERR_load_BIO_strings();

	gpstrBio = BIO_new_fd( nFd, BIO_NOCLOSE );
	if ( !gpstrBio ) {
		ERR_print_errors_fp( stderr );
		return -1;
	}

	return 0;
}

/*
 * BIO_read()
 * ソケットからの読み込み
 */
int RecvBio( BIO *pstrBio, unsigned char *pszBuff, size_t nBuffSize, int *pnFlagConn )
{
	int nFd = 0;
	int nRtn = 0;
	int nRecvSize = 0;
	int nDone = 0;
	struct timeval strTv;
	fd_set strFds;

	/* BIOオブジェクトからFDを取得 */
	nFd = BIO_get_fd( pstrBio, NULL );

	while(1){
		if( !nBuffSize ){
			/* バッファ残りサイズチェック */
			/* バッファサイズと同サイズが受信された場合もここに来る */
			fprintf( stderr, "Error: Buffer over.\n" );
			return -2;
		}

		nRecvSize = BIO_read( pstrBio, pszBuff, nBuffSize );
		if( nRecvSize < 0 ){
			perror( "recv()" );
			return -1;

		} else if( nRecvSize == 0 ){
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
			FD_ZERO( &strFds );
			FD_SET( nFd, &strFds );
			strTv.tv_sec = 0;
			strTv.tv_usec = 0; /* タイムアウト 0S */
			nRtn = select( nFd+1, &strFds, NULL, NULL, &strTv );
			if( nRtn < 0 ){
				/* select()エラー */
				perror( "select()" );
				return -3;
			} else if( nRtn == 0 ){
				/* タイムアウト */
			}

			/* レディではなくなったらwhileループから抜ける */
			if( !FD_ISSET( nFd, &strFds ) ){
				break;
			}
			/*----------------------------------------------------------*/
		}
	}

	return nDone;
}

/*
 * BIO_write()
 * ソケットへの書き込み
 */
int SendBio( BIO *pstrBio, const unsigned char *pszBuff, size_t nLen )
{
	int nSendSize = 0;
	int nDone = 0;

	while(1){
		nSendSize = BIO_write( pstrBio, pszBuff, nLen-nDone );
		if( nSendSize < 0 ){
			perror( "send()" );
			return -1;

		} else {
			/* send ok */
			pszBuff += nSendSize;
			nDone += nSendSize;

			/* 全て送信できたらループ抜ける */
			if( nLen == nDone ){
				break;
			}
		}
	}

	return nDone;
}
