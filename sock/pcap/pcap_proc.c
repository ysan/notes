#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/msg.h>

#include "mycommon.h"
#include "pcap_signal.h"
#include "pcap_proc.h"
#include "pcap_main.h"
#include "pcap_analyze.h"
#include "pcap_addr2name.h"


int gnMsqid; /* メインプロセス内で使用する */

/*
 * メッセージキュー作成
 * (メインプロセス内で使用する)
 */
int CreateMsgQue( void )
{
	if ( ( gnMsqid = msgget( MSG_QUE_KEY, MSG_QUE_MODE | IPC_CREAT ) ) < 0 ) {
		perror( "msgget()" );
		return -1;
	}

	return 0;
}

/*
 * メッセージキュー削除
 * (メインプロセス内で使用する)
 */
int DeleteMsgQue( void )
{
	if ( msgctl( gnMsqid, IPC_RMID, NULL ) < 0 ) {
		perror( "msgctl()" );
		return -1;
	}

	return 0;
}

/*
 * パケット受信プロセス
 */
int RecvPacketProc( int nFdSock, int nMsqid )
{
	int nRtn = 0;
	struct msg_packet_info strMsgPacketInfo;

	if ( SetSigHandle( SIGUSR1 ) < 0 ) {
		fprintf( stderr, "Err: SetSigHandle()\n" );
		return -1;
	}

	strMsgPacketInfo.m_nType = MSG_TYPE_NO;

	while (1) {
		strMsgPacketInfo.m_nPacketSize = 0;
		memset( strMsgPacketInfo.m_szCapTime, 0x00, sizeof(strMsgPacketInfo.m_szCapTime) );
		memset( strMsgPacketInfo.m_szPacketData, 0x00, sizeof(strMsgPacketInfo.m_szPacketData) );

		/* パケット受信 */
		strMsgPacketInfo.m_nPacketSize = recv( nFdSock, strMsgPacketInfo.m_szPacketData,
													sizeof(strMsgPacketInfo.m_szPacketData), 0 );
		if ( strMsgPacketInfo.m_nPacketSize < 0 ) {
			if( errno == EINTR ){
				/* 割り込み発生 */
				fprintf( stdout, "recv(): Interrupt occurs.\n" );
				goto RECV_INTERRUPT;
				
			} else {
				perror( "recv()" );
				return -1;
			}
		}

		/* パケット受信時間取得 */
		nRtn = GetRecvTime( nFdSock, strMsgPacketInfo.m_szCapTime,
										sizeof(strMsgPacketInfo.m_szCapTime) );
		if ( nRtn < 0 ) {
			fprintf( stderr, "Err: GetRecvTime()\n" );
			return -1;
		}

		/*
		 * キューのデフォルトの最大サイズ MSGMNB : 16384 bytes
		 * msgctl()でキューのサイズをMSGMNB よりも大きい値に変更可能。(root権限要)
		 * (参考 1メッセージの最大サイズ MSGMAX : 8192 bytes )
		 */
		/* キューにパケットデータを送信 (キューがフルであればブロックする) */
		if ( msgsnd( nMsqid, &strMsgPacketInfo, sizeof(strMsgPacketInfo), 0 ) < 0 ) {
			if ( errno == EINTR ) {
				/* 割り込み発生 */
				fprintf( stdout, "recv(): Interrupt occurs.\n" );
				goto RECV_INTERRUPT;
				
			} else {
				perror( "msgsnd()" );
				return -1;

			}
		}

		RECV_INTERRUPT:
		if ( gCatchSignal ) {
			/* メインプロセスからの停止指示 (SIGUSR1が送信された) */
			fprintf( stdout, "RecvPacketProc() to exit. [%s]\n", sys_siglist[gCatchSignal] );
			gCatchSignal = 0;
			break;
		}

	}

	return 0;
}

/*
 * パケット解析プロセス
 */
int AnalyzePacketProc( int nMsqid )
{
	int nRtn = 0;
	struct msg_packet_info strMsgPacketInfo;

	/* アドレス変換リストテーブル初期化 */
	InitNameList();

	if ( SetSigHandle( SIGUSR1 ) < 0 ) {
		fprintf( stderr, "Err: SetSigHandle()\n" );
		return -1;
	}

	if ( SetSigHandle( SIGHUP ) < 0 ) {
		fprintf( stderr, "Err: SetSigHandle()\n" );
		return -1;
	}

	while (1) {

		memset( &strMsgPacketInfo, 0x00, sizeof(strMsgPacketInfo) );

		/* キューからパケットデータを取り出す (無ければブロックする) */
		if ( msgrcv( nMsqid, &strMsgPacketInfo, sizeof(strMsgPacketInfo), MSG_TYPE_NO, 0 ) < 0 ) {
			if ( errno == EINTR ) {
				/* 割り込み発生 */
				fprintf( stdout, "msgrcv(): Interrupt occurs.\n" );
				goto MSGRCV_INTERRUPT;

			} else {
				perror( "msgrcv()" );
				return -1;
			}
		}

		fprintf( stdout, "\n%s [%d]bytes captured...\n",
							strMsgPacketInfo.m_szCapTime, strMsgPacketInfo.m_nPacketSize );

		/* パケットの解析実行 */
		nRtn = AnalyzePacket( strMsgPacketInfo.m_szPacketData, strMsgPacketInfo.m_nPacketSize );
		if ( nRtn < 0 ) {
			fprintf( stdout, "Warn: Irregular packet...\n" );
		}


		/* 16進数ダンプ表示+ascii  */
		Dumper (
			strMsgPacketInfo.m_szPacketData,
			strMsgPacketInfo.m_nPacketSize,
			DUMP_PUTS_ASCII_ON
		);


		MSGRCV_INTERRUPT:
		if ( gCatchSignal == SIGUSR1 ) {
			/* メインプロセスからの停止指示 (SIGUSR1が送信された) */
			fprintf( stdout, "AnalyzePacketProc() to exit. [%s]\n", sys_siglist[gCatchSignal] );

			RefNameList();
			FreeAllMallocList();
			gCatchSignal = 0;
			break;

		} else if (gCatchSignal == SIGHUP ) {

			RefNameList();
			gCatchSignal = 0;
			continue;
		}

	}

	return 0;
}

/*
 * ゾンビプロセスの削除
 */
int CheckWaitpid( void )
{
	pid_t nPidWait;

	while(1){

		/* ゾンビプロセスが見つからなければ処理を返す(WNOHANG:ブロックしない) */
		nPidWait = waitpid( (pid_t)-1, NULL, WNOHANG );
		if( nPidWait < 0 ){
			if( errno == ECHILD ){
				/* ゾンビプロセスなし */
				fprintf( stdout, "Zombie is none.(ECHILD)\n" );
				break;

			} else {
				perror( "waitpid()" );
				return -1;
			}

		} else if( nPidWait == 0 ){
			/* ゾンビプロセスなし */
			fprintf( stdout, "Zombie is none.\n" );
			break;

		} else {
			/* ゾンビプロセスを削除した */
			fprintf( stdout, "Zombie is reject. Pid:[%d]\n", nPidWait );

		}
	}

	return 0;
}
