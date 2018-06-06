#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "reg3067.h"
#include "defines.h"
#include "initialize.h"
#include "sci.h"
#include "intc.h"
#include "timer.h"
#include "bsc.h"
#include "xmodem.h"
#include "led.h"


/*
 * プロトタイプ宣言
 */
static void InitStaticValues( void );

/*
 * 初期化処理 
 */
void Init( void )
{
	/* 割り込み禁止 */
	DI();

	/* 静的変数の初期化 */
	InitStaticValues();

	/* ソフトウェア割り込みベクタ初期化 */
	InitSoftvec();


	/* バスコン初期化 */
	InitBsc();

	/* SCI1初期化 */
	initSci1();

	/* 8bitTimer初期化 */
	InitTimer0();
	InitTimer2();


	/* XMODEM受信初期化 */
	InitXmodemRecv();

	/* LEDバー初期化 */
	initLedBar();


	/* 割り込み許可 */
	EI();
}

/*
 * 静的変数の初期化
 */
static void InitStaticValues( void )
{
	/* ROM上の静的変数の初期値をRAM上にコピー */
	memcpy( &data_start, &rodata_end, (DWORD)&data_end - (DWORD)&data_start );

	/* bssセクションを0クリア */
	memset( &bss_start, 0x00, (DWORD)&bss_end - (DWORD)&bss_start );

	/* ram_buffセクションを0クリア */
	ClearRamBuffSection();
}

/*
 * ram_buffセクションクリア
 */
void ClearRamBuffSection( void )
{
	memset( &ram_buff, 0x00, RAM_BUFF_SECTION_SIZE );
}

/*
 * dramセクションクリア
 */
void ClearDramSection( void )
{
	memset( &dram_start, 0x00, DRAM_SECTION_SIZE );
}
