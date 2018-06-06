#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "reg3067.h"
#include "defines.h"
#include "bsc.h"
#include "initialize.h"


/*
 * バスコン初期化
 * 裏面DRAM(HM5117805)使用設定
 */
void InitBsc( void )
{
	/* エリア2 8bitアクセス */
	ABWCR |= ABWCR_ABW2; 

	/* リフレッシュの設定 */
	RTCOR = 7;						// コンペアマッチ7回
	RTMCSR |= RTMCSR_CKS_PER2048;	// 20MHz/2048
	RTMCSR &= ~RTMCSR_CMIE;
	RTMCSR &= ~RTMCSR_CMF;

	/* DRAMコントロールB */
	DRCRB &= ~DRCRB_RLW;
	DRCRB &= ~DRCRB_RCW;
	DRCRB &= ~DRCRB_TPC;
	DRCRB |= DRCRB_RCYCE;
	DRCRB &= ~DRCRB_CSEL;
	DRCRB |= DRCRB_MXC_10;

	/* DRAMコントロールA */
	DRCRA &= ~DRCRA_RFSHE;
	DRCRA &= ~DRCRA_SRFMD;
	DRCRA &= ~DRCRA_RDM;
	DRCRA &= ~DRCRA_BE;
	DRCRA |= DRCRA_DRAS_001; // DRAMエリア CS2_

	/* IOポート出力設定 */
	P1DDR = 0xff; // A0~A7
	P2DDR = 0x07; // A8~A10 
	P8DDR = 0x04; // CS2_

	/* ウェイトコントロール */
	WCRL &= WCRL_W2_NON;

	/* アクセスステートコントロール */
	ASTCR &= ~ASTCR_AST2;

#if 0
	ABWCR  = 0xff;
	RTCOR  = 0x07;
	RTMCSR = 0x37;
	DRCRB  = 0x98;
	DRCRA  = 0x30;

	P1DDR  = 0xff;
	P2DDR  = 0x07;
	P8DDR  = 0xe4;
	/* *H8_3069F_PBDDR = ...; */

	/* H8_3069F_WCRH = ...; */
	WCRL = 0xcf;

	ASTCR = 0xfb;
#endif

	/* DRAMセクションクリア */
	ClearDramSection();
}
