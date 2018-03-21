#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "mycommon.h"
#include "pcap_main.h"
#include "pcap_analyze.h"
#include "pcap_addr2name.h"
#include "pcap_signal.h"
#include "pcap_proc.h"
#include "pcap_socket.h"


/*
 * メイン
 */
int main( int argc, char **argv )
{
	int nRtn = 0;
	int nFdSock = 0;
	pid_t nPid[2];


	if ( argc != 2 ) {
		fprintf( stderr, "Usage: %s <NetworkInterface>\n", argv[0] );
		exit( EXIT_FAILURE );
	}

	/* メッセージキューID */
	gnMsqid = 0;


	/* シグナルマスク設定 */
	if ( SetSignalMask() < 0 ) {
		fprintf( stderr, "Err: SetSignalMask()\n" );
		exit( EXIT_FAILURE );
	}


	/* ソケット作成 */
	if ( ( nFdSock = CreateRawSocket( argv[1], ETH_P_ALL ) ) < 0 ) {
		fprintf( stderr, "Err: CreateRawSocket()\n" );
		close( nFdSock );
		exit( EXIT_FAILURE );
	}

	/* メッセージキュー作成 */
	if ( CreateMsgQue() < 0 ){
		fprintf( stderr, "Err: CreateMsgQue()\n" );
		exit( EXIT_FAILURE );
	}


	/* パケット受信プロセス生成 */
	nPid[0] = fork();
	if ( nPid[0] < 0 ) {
		perror( "fork()" );
		exit( EXIT_FAILURE );

	} else if ( nPid[0] == 0 ) {
		/* 子プロセス */
		fprintf( stdout, "Start RecvPacketProcess. Pid:[%d]\n", getpid() );
		nRtn = RecvPacketProc( nFdSock, gnMsqid );
		if ( nRtn < 0 ) {
			fprintf( stderr, "Err: RecvPacketProc()\n" );
			exit( EXIT_FAILURE );
		}

		exit( EXIT_SUCCESS );
	}

	usleep(10);

	/* パケット解析プロセス生成 */
	nPid[1] = fork();
	if ( nPid[1] < 0 ) {
		perror( "fork()" );
		exit( EXIT_FAILURE );

	} else if ( nPid[1] == 0 ) {
		/* 子プロセス */
		fprintf( stdout, "Start AnalyzePacketProcess. Pid:[%d]\n", getpid() );
		nRtn = AnalyzePacketProc( gnMsqid );
		if ( nRtn < 0 ) {
			fprintf( stderr, "Err: AnalyzePacketProc()\n" );
			exit( EXIT_FAILURE );
		}

		exit( EXIT_SUCCESS );
	}


	/* シグナル受信待ち */
	if ( WaitSignal( nPid ) < 0 ) {
		fprintf( stderr, "Err: WaitSignal()\n" );
		close( nFdSock );

		/* メッセージキュー破棄 */
		if ( DeleteMsgQue() < 0 ) {
			fprintf( stderr, "Err: DeleteMsgQue()\n" );
			exit( EXIT_FAILURE );
		}

		exit( EXIT_FAILURE );
	}


	close( nFdSock );


	/* メッセージキュー破棄 */
	if ( DeleteMsgQue() < 0 ) {
		fprintf( stderr, "Err: DeleteMsgQue()\n" );
		exit( EXIT_FAILURE );
	}


	fprintf( stdout, "main() normal end.\n" );


	exit( EXIT_SUCCESS );
}
