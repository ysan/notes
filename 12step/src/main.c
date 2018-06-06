#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "reg3067.h"
#include "defines.h"
#include "initialize.h"
#include "intc.h"
#include "sci.h"
#include "xmodem.h"
#include "timer.h"
#include "command.h"
#include "util.h"
#include "led.h"


/*
 * メイン
 */
int main( void )
{
	/* 初期化 */
	Init();

	StartTimer0(); /* LEDバー ダイナミック点灯 */
	StartTimer2(); /* LED(赤)輝度変化/XMODEM受信待機通知 */


	logPrint( " ---- %s ----\n", BOOTMSG );

	logPrint( "sizeof(DWORD) = [%d]\n", (DWORD)sizeof(DWORD) );
	logPrint( "sizeof(WORD) = [%d]\n", (DWORD)sizeof(WORD) );
	logPrint( "sizeof(BYTE) = [%d]\n", (DWORD)sizeof(BYTE) );

	logPrint( "NULL addr: [0x%x]\n", (DWORD)NULL );

	logPrint( "[%s]\n", "aaaa" );
	logPrint( "[%10s] [%5d] [%06x] [%05b]\n", "test", 10, 0xa5, 3 );


	while (1) {

//TODO このifいらないかも
		if (GetXmodemRecvState() == XMODEM_RECV_STATE_INIT) {
			/* 割り込み禁止すべき? */
			analyzeSci1RecvData();
		}

		AnalyzeXmodemRecv();

	}


}
