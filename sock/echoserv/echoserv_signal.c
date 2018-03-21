#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "mycommon.h"
#include "echoserv_signal.h"
#include "echoserv_child_info.h"

/*
 * 変数宣言
 */
static volatile sig_atomic_t gSigVal;

/*
 * プロトタイプ宣言
 */
static void SigHandler( int );


/*
 * シグナルハンドラ関数
 */
static void SigHandler( int nSigno )
{
	/* シグナル種セット */
	gSigVal = nSigno;
	return;
}

/*
 * シグナル値格納変数初期化
 */
void InitSigVal( void )
{
	gSigVal = 0;
	return;
}

/*
 * シグナル値取得
 */
volatile sig_atomic_t GetSigVal( void )
{
	return gSigVal;
}

/*
 * シグナルハンドラ設定
 */
int SetSigHandle( int nSigkind )
{
	struct sigaction stSigact;

	memset( &stSigact, 0x00, sizeof(stSigact) );
	stSigact.sa_handler = SigHandler;

	if( sigaction( nSigkind, &stSigact, NULL ) < 0 ) {
		perror( "sigaction()" );
		return -1;
	}

	return 0;
}

/*
 * 無視シグナル設定
 */
int IgnoreSignal( int nSigkind )
{
	struct sigaction stSigact;

	memset( &stSigact, 0x00, sizeof(stSigact) );
	stSigact.sa_handler = SIG_IGN;

	if( sigaction( nSigkind, &stSigact, NULL ) < 0 ) {
		perror( "sigaction()" );
		return -1;
	}

	return 0;
}


/*------------ 以下 子プロセスで使用する ------------*/

/*
 * 変数宣言
 */
static volatile sig_atomic_t gSigValChild;
static int gSigInfoVal[ SIG_INFO_VAL_STACK_SIZE ];
static int gSigInfoValReadPos;
static int gSigInfoValWritePos;
static sigset_t gstSigset;

/*
 * プロトタイプ宣言
 */
static void SigHandlerSiginfo( int, siginfo_t*, void* );


/*
 * シグナルマスクセット
 */
void SetSigmask( void )
{
	sigemptyset( &gstSigset );
	sigaddset( &gstSigset, SIGRTMIN );
	return;
}

/*
 * 割り込み禁止
 * SIGRTMIN
 */
int DisableInterrupt( void )
{
	if ( sigprocmask( SIG_BLOCK, &gstSigset, NULL ) < 0 ) {
		perror( "sigprocmask()" );
		return -1;
	}

	return 0;
}

/*
 * 割り込み許可
 * SIGRTMIN
 */
int EnableInterrupt( void )
{
	if ( sigprocmask( SIG_UNBLOCK, &gstSigset, NULL ) < 0 ) {
		perror( "sigprocmask()" );
		return -1;
	}

	return 0;
}

/*
 * シグナルハンドラ関数
 * siginfo構造体
 */
static void SigHandlerSiginfo( int nSigno, siginfo_t *pstSiginfo, void *pUcontext )
{
	/* シグナル種セット */
	gSigValChild = nSigno;

	/* sigqueue()から送られたデータを取得(スタックする) */
	gSigInfoVal[ gSigInfoValWritePos ] = pstSiginfo->si_int;

	gSigInfoValWritePos ++;
	gSigInfoValWritePos &= SIG_INFO_VAL_STACK_SIZE -1;

	if ( gSigInfoValReadPos == gSigInfoValWritePos ) {
		/* バッファが一周した */
		/* error or warning */
		/*DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD*/
	}

	return;
}

/*
 * シグナル値格納変数初期化
 */
void InitSigValChild( void )
{
	gSigValChild = 0;
	return;
}

/*
 * シグナル値取得
 */
volatile sig_atomic_t GetSigValChild( void )
{
	return gSigValChild;
}

/*
 * sigqueue受信値格納変数初期化
 */
void InitSigInfoVal( void )
{
	memset( gSigInfoVal, 0x00, sizeof(gSigInfoVal) );
	gSigInfoValReadPos = 0;
	gSigInfoValWritePos = 0;
	return;
}

/*
 * sigqueue受信値取得
 */
volatile sig_atomic_t GetSigInfoVal( void )
{
	return gSigInfoVal[ gSigInfoValReadPos ];
}

/*
 * シグナルハンドラ設定
 * sigqueue()にてsiginfo構造体を受け取る
 */
int SetSigHandleSiginfo( int nSigkind )
{
	struct sigaction stSigact;

	memset( &stSigact, 0x00, sizeof(stSigact) );
	stSigact.sa_sigaction = SigHandlerSiginfo;
	stSigact.sa_flags = SA_SIGINFO;
	stSigact.sa_mask = gstSigset; /* ハンドラ中割り込み禁止するシグナルマスク */

	if( sigaction( nSigkind, &stSigact, NULL ) < 0 ) {
		perror( "sigaction()" );
		return -1;
	}

	return 0;
}

/*
 * sigqueue()
 */
int SendSigque( pid_t nPid, int nSigKind, int nSiVal )
{
	union sigval unSigval;

	memset( &unSigval, 0x00, sizeof(unSigval) );

	/* siguqeue()で送るデータをセット */
	unSigval.sival_int = nSiVal;

	if ( sigqueue( nPid, nSigKind, unSigval ) < 0 ) {
		perror( "sigqueue()" );
		return -1;
	}

	return 0;
}

/*
 * SigInfoValスタックデータを辿りコールバック関数を実行する
 */
void PursueSigInfoVal( int (*pFuncCallback)( int, void* ), void *pArg )
{
	/*
	 * 読み出し位置を書き込み位置まで回し
	 * コールバック関数を実行する
	 */
	while ( gSigInfoValReadPos != gSigInfoValWritePos ) {

		if ( pFuncCallback ) {
			pFuncCallback( gSigInfoVal[ gSigInfoValReadPos ], pArg );
		}

		gSigInfoValReadPos ++;
		gSigInfoValReadPos &= SIG_INFO_VAL_STACK_SIZE -1;
	}
}
