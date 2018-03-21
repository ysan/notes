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
#include "starttls.h"


/*
 * STARTTLS SMTP
 * モード遷移
 */
int StartTlsSmtp( int nFdSockCl )
{
	int nRtn = 0;
	int nFlag = 0;
	unsigned char szTmp[1024];


	fprintf( stdout, "START TLS sequence...\n" );


	memset( szTmp, 0x00, sizeof(szTmp) );

	/* connect後 サーバからのレスポンスを受信 */
	nRtn = RecvData( nFdSockCl, szTmp, sizeof(szTmp), &nFlag );
	if ( nRtn < 0 ) {
		fprintf( stderr,"RecvData() is failure.\n" );
		return -1;
	}

	fprintf( stdout, "recv: %s", (char*)szTmp );

	if ( nFlag ) {
		/* 0 byte received */
		fprintf( stdout, "Connection with a partner went out...\n" );
		return -1;
	}

	/* SMTP HELOコマンド送信 */
	nRtn = SendData( nFdSockCl, (unsigned char*)COM_SMTP_HELO, strlen(COM_SMTP_HELO) );
	if ( nRtn < 0 ) {
		fprintf( stderr,"SendData() is failure.\n" );
		return -1;
	}

	fprintf( stdout, "send: %s", COM_SMTP_HELO );


	memset( szTmp, 0x00, sizeof(szTmp) );

	/* HELOコマンド レスポンス受信 */
	nRtn = RecvData( nFdSockCl, szTmp, sizeof(szTmp), &nFlag );
	if ( nRtn < 0 ) {
		fprintf( stderr,"RecvData() is failure.\n" );
		return -1;
	}

	/* レスポンスコード チェック */
	if ( ( szTmp[0] != '2' ) || (szTmp[1] != '5') || (szTmp[2] != '0') ) {
		fprintf( stderr, "SMTP response code is bad.\n" );
		return -1;
	}

	fprintf( stdout, "recv: %s", (char*)szTmp );

	if ( nFlag ) {
		/* 0 byte received */
		fprintf( stdout, "Connection with a partner went out...\n" );
		return -1;
	}

	/* SMTP STARTTLSコマンド送信 */
	nRtn = SendData( nFdSockCl, (unsigned char*)COM_SMTP_STARTTLS, strlen(COM_SMTP_STARTTLS) );
	if ( nRtn < 0 ) {
		fprintf( stderr,"SendData() is failure.\n" );
		return -1;
	}

	fprintf( stdout, "send: %s", COM_SMTP_STARTTLS );


	memset( szTmp, 0x00, sizeof(szTmp) );

	/* STARTTLSコマンド レスポンス受信 */
	nRtn = RecvData( nFdSockCl, szTmp, sizeof(szTmp), &nFlag );
	if ( nRtn < 0 ) {
		fprintf( stderr,"RecvData() is failure.\n" );
		return -1;
	}

	/* レスポンスコード チェック */
	if ( ( szTmp[0] != '2' ) || (szTmp[1] != '2') || (szTmp[2] != '0') ) {
		fprintf( stderr, "SMTP response code is bad.\n" );
		return -1;
	}

	fprintf( stdout, "recv: %s", (char*)szTmp );

	if ( nFlag ) {
		/* 0 byte received */
		fprintf( stdout, "Connection with a partner went out...\n" );
		return -1;
	}


	fprintf( stdout, "START TLS sequence is complete.\n" );

	return 0;
}
