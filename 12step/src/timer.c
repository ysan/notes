#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "defines.h"
#include "reg3067.h"
#include "timer.h"
#include "intr.h"
#include "intc.h"
#include "xmodem.h"
#include "sci.h"
#include "led.h"


/*
 * 変数宣言
 */
static WORD gwCntIntrTimer2_0;
static WORD gwCntIntrTimer2_1;
static BOOL gIsTcsr;


/*
 * 8bitタイマ0 初期化
 */
void InitTimer0( void )
{
//	/* out High */
//	PBDDR |= 1<<0;
//	PBDR |= 1<<0;

	/* カウンタをクリア */
	TCNT0_8 = 0x00;

	/* カウンタクリア要因をコンペアマッチAに設定 */
	TCR0_8 = TCR_8_CCLR_CMA;

	/*
	 * コンスタントレジスタA0を設定
	 * 1/(20MHz/8192) = 約0.4mS
	 */
	TCORA0_8 = 1;

	/* 割り込みハンドラセット */
	SetIntrSoftvec( SOFTVEC_TYPE_TIMER0, IntrHandlerTimer0 );

	/* コンペアマッチA 割り込み要求を許可 */
	TCR0_8 |= TCR_8_CMIEA;
}

/*
 * 8bitタイマ2 初期化
 */
void InitTimer2( void )
{
	gwCntIntrTimer2_0 = 0;
	gwCntIntrTimer2_1 = 0;
	gIsTcsr = FALSE;

	/* out High */
	PBDDR |= 1<<2;
	PBDR |= 1<<2;

	/* カウンタをクリア */
	TCNT2_8 = 0;

	/* カウンタクリア要因をコンペアマッチAに設定 */
	TCR2_8 = TCR_8_CCLR_CMA;

	/*
	 * コンスタントレジスタA2及びB2を設定
	 * 1/(20MHz/8192) *50 = 約20mS
	 */
	TCORA2_8 = 50;
	TCORB2_8 = 0; /* 割り込み側で変化させる */

	/* 割り込みハンドラセット */
	SetIntrSoftvec( SOFTVEC_TYPE_TIMER2, IntrHandlerTimer2 );

	/* コンペアマッチA 割り込み要求を許可 */
	TCR2_8 |= TCR_8_CMIEA;
}

/*
 * 8bitタイマ0 スタート
 */
void StartTimer0( void )
{
//	/*
//	 * コントロール/ステータスレジスタ
//	 * タイマ出力 コンペアマッチAでトグル出力
//	 */
//	TCSR0_8 = TCSR_8_A_OS_OUT_TGL;

	/*
	 * カウンタクロックをφ/8192に設定
	 * カウントスタート
	 */
	TCR0_8 |= TCR_8_CKS_PER8192;
}

/*
 * 8bitタイマ2 スタート
 */
void StartTimer2( void )
{
	/*
	 * コントロール/ステータスレジスタ
	 * タイマ出力 コンペアマッチAでL出力 コンペアマッチBでH出力
	 */
	TCSR2_8 = TCSR_8_A_OS_OUT_L | TCSR_8_B_OS_OUT_H;

	/*
	 * カウンタクロックをφ/8192に設定
	 * カウントスタート
	 */
	TCR2_8 |= TCR_8_CKS_PER8192;
}

/*
 * 8bitタイマ0 停止
 */
void StopTimer0( void )
{
	TCR0_8 &= ~TCR_8_CKS_PER8192;
//	TCSR0_8 &= ~TCSR_8_A_OS_OUT_TGL;

//	/* out High */
//	PBDDR |= 1<<0;
//	PBDR |= 1<<0;
}

/*
 * 8bitタイマ2 停止
 */
void StopTimer2( void )
{
	TCR2_8 &= ~TCR_8_CKS_PER8192;
	TCSR2_8 &= ~( TCSR_8_A_OS_OUT_L | TCSR_8_B_OS_OUT_H );

	/* out High */
	PBDDR |= 1<<2;
	PBDR |= 1<<2;
}

/*
 * 8bitタイマ0 割り込み
 * インターバル約0.4mS
 */
void IntrTimer0( void )
{
	/*---------- LEDバー点灯 ----------*/

	/* 次グループをセット */
	setNextGroupLedBar();

	/* データ生成 */
	createDataLedBar();

	checkTurnOffLedBar();

	/* ダイナミック点灯切り替え */
	setDynamicLedBar();



	/* コンペアマッチフラグAを0クリア */
	TCSR0_8 &= ~TCSR_8_CMFA;
}

/*
 * 8bitタイマ2 割り込み
 * インターバル約20mS
 */
void IntrTimer2( void )
{
	WORD wCnt = gwCntIntrTimer2_0 > 0xff ? 0xff : gwCntIntrTimer2_0;

	/*---------- LED 輝度変化 ----------*/

	/* コンスタントレジスタB0を書き換える */
	TCORB2_8 = (BYTE)(wCnt);

	/* 約1Secで一周 */
	if ( gwCntIntrTimer2_0 >= 50 ) {
		gwCntIntrTimer2_0 = 0;

		if ( gIsTcsr ) {
			TCSR2_8 &= ~TCSR_8_A_OS_OUT_H;
			TCSR2_8 &= ~TCSR_8_B_OS_OUT_L;
			TCSR2_8 |= TCSR_8_A_OS_OUT_L | TCSR_8_B_OS_OUT_H;
			gIsTcsr = FALSE;
		} else {
			TCSR2_8 &= ~TCSR_8_A_OS_OUT_L;
			TCSR2_8 &= ~TCSR_8_B_OS_OUT_H;
			TCSR2_8 |= TCSR_8_A_OS_OUT_H | TCSR_8_B_OS_OUT_L; /* 初期化値 */
			gIsTcsr = TRUE;
		}
	}

	gwCntIntrTimer2_0 ++;


	/*---------- XMODEM受信待機通知 ----------*/

	if ( gwCntIntrTimer2_1 >= 1000 ) {

		/* 約20Secごとに通知 */
		if ( GetXmodemRecvState() == XMODEM_RECV_STATE_WAIT ) {
			sendByteSci1( XMODEM_NAK );
		}

		gwCntIntrTimer2_1 = 0;
	}

	gwCntIntrTimer2_1 ++;


	/* コンペアマッチフラグAを0クリア */
	TCSR2_8 &= ~TCSR_8_CMFA;
}
