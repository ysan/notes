#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/wait.h>
#include <linux/limits.h>


#include "mycommon.h"
#include "http_defines.h"
#include "hserv.h"


/*
 * cgiプログラム実行
 */
int ExecCgi( const char *pszExecCgi, const char *pszStdin, char *pszSendBuff, size_t nBuffSize )
{
	int nRtn = 0;
	int	nPipeC2P[2];
	int nPipeP2C[2];
	int nReadSize = 0;
	int nStatus = 0;
/*	int nFcntlVal = 0;*/
	pid_t nPid;
	pid_t nPidWait;


	/* パイプを2つ作成 */
	nRtn = pipe( nPipeP2C );
	if( nRtn < 0 ){
		perror( "pipe()" );
		return -1;
	}

	nRtn = pipe( nPipeC2P );
	if( nRtn < 0 ){
		perror( "pipe()" );
		close( nPipeP2C[R] );
		close( nPipeP2C[W] );
		return -1;
	}


	/* ノンブロック設定 */
/***
	if(( nFcntlVal = fcntl( nPipeC2P[R], F_GETFL, 0 ) ) < 0 ){
		perror( "fcntl()" );
		close( nPipeP2C[R] );
		close( nPipeP2C[W] );
		return -1;
	}

	nFcntlVal |= O_NONBLOCK;

	if( fcntl( nPipeC2P[R], F_SETFL, nFcntlVal ) < 0 ){
		perror( "fcntl()" );
		close( nPipeP2C[R] );
		close( nPipeP2C[W] );
		return -1;
	}
***/

	/* forkして子プロセスでcgi実行(成り代わる) 親プロセスで標準出力結果を受け取る */
	nPid = fork();
	if( nPid == -1 ){
		/* エラー */
		perror( "fork()" );
		close( nPipeP2C[R] );
		close( nPipeP2C[W] );
		close( nPipeC2P[R] );
		close( nPipeC2P[W] );
		return -1;

	} else if( nPid == 0 ){
		/* 子プロセス */

		close( nPipeC2P[R] );
		close( nPipeP2C[W] );

		dup2( nPipeP2C[R], STDIN_FILENO );
		dup2( nPipeC2P[W], STDOUT_FILENO );
		close( nPipeP2C[R] );
		close( nPipeC2P[W] );

		nRtn = execl( pszExecCgi, pszExecCgi, NULL );
		if( nRtn < 0 ){
			perror( "execl()" );
			exit( EXIT_FAILURE );
		}

	} else {
		/* 親プロセス */

		/* 子プロセスへの標準入力 */
		if( pszStdin ){
			nRtn = WriteData( nPipeP2C[W], (unsigned char*)pszStdin, strlen(pszStdin) );
			if( nRtn < 0 ){
				fprintf( stderr, "Error: SendData() is failure.\n" );
				close( nPipeP2C[R] );
				close( nPipeP2C[W] );
				close( nPipeC2P[R] );
				close( nPipeC2P[W] );
				return -1;
			}
		}

		/* 子プロセスの標準出力を受け取る */
		nReadSize = ReadDataPipe( nPipeC2P[R], (unsigned char*)pszSendBuff, nBuffSize );
		if( nReadSize < 0 ){
			fprintf( stderr, "Error: ReadData() is failure. nReadSize=[%d]\n", nReadSize );
			close( nPipeP2C[R] );
			close( nPipeP2C[W] );
			close( nPipeC2P[R] );
			close( nPipeC2P[W] );
			return -1;
		}

		/* 子プロセスpidを指定して 終了を待つ(ブロックする) */
		nPidWait = waitpid( nPid, &nStatus, 0 );
		if( nPidWait < 0 ){
			perror( "waitpid()" );
			close( nPipeP2C[R] );
			close( nPipeP2C[W] );
			close( nPipeC2P[R] );
			close( nPipeC2P[W] );
			return -1;
		}

		if( WIFEXITED(nStatus) ){
			/* 子プロセス正常終了 */
			fprintf( stdout, "Child is successful.\n" );

		} else {
			/* 子プロセス異常終了 */
			fprintf( stderr, "Error: Child is failure.\n" );
			close( nPipeP2C[R] );
			close( nPipeP2C[W] );
			close( nPipeC2P[R] );
			close( nPipeC2P[W] );
			return -1;
		}
	}

	close( nPipeP2C[R] );
	close( nPipeP2C[W] );
	close( nPipeC2P[R] );
	close( nPipeC2P[W] );

	return 0;
}

/*
 * cgiで出力されたHTMLボディ長を取得
 */
int GetCgiBodyLength( const char *pszBuff )
{
	const char *pszBodyStat = NULL;

	/* ボディの先頭アドレスを探す */

	/* 改行コードが CRLFの場合 */
	if( ( pszBodyStat = strstr( pszBuff, CRLFCRLF ) ) ){
		pszBodyStat += 4;
	} else {
		/* 改行コードが LFの場合 */
		if( ( pszBodyStat = strstr(pszBuff, LFLF ) ) ){
			pszBodyStat += 2;
		} else {
			/* みつからない */
			pszBodyStat = pszBuff;
		}
	}

	/* NULL終端されていること あくまで文字列であること限定 */
	return strlen( pszBodyStat );
}
