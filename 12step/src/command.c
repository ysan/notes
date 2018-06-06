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
#include "elf_loader.h"
#include "led.h"
#include "util.h"


/*
 * 変数宣言
 */
static BYTE gbCommand[ COMMAND_SIZE_MAX ];

/*
 * プロトタイプ宣言
 */
static void analyzeCommand( const BYTE *pbComm, WORD wLenComm );


/**
 * コマンド解析
 */
static void analyzeCommand( const BYTE *pbComm, WORD wLenComm )
{
	/* 引数チェック */
	if ( ( !pbComm ) || ( wLenComm == 0 ) ) {
		return;
	}

	if (
		!memcmp( pbComm, (void*)"echo", strlen("echo") ) &&
		strlen("echo") == strlen((char*)pbComm)
	) {
		logPrint( "H83069 echo.\n" );

	} else if (
		!memcmp( pbComm, (void*)"start timer0", strlen("start timer0") ) &&
		strlen("start timer0") == strlen((char*)pbComm)
	) {
		StartTimer0();

	} else if (
		!memcmp( pbComm, (void*)"boot", strlen("boot") ) &&
		strlen("boot") == strlen((char*)pbComm)
	) {
//TODO
		if ( Boot( (const void*)&dram_start ) ) {

		}

	} else if (
		!memcmp( pbComm, (void*)"stop timer0", strlen("stop timer0") ) &&
		strlen("stop timer0") == strlen((char*)pbComm)
	) {
		StopTimer0();

	} else if (
		!memcmp( pbComm, (void*)"start timer2", strlen("start timer2") ) &&
		strlen("start timer2") == strlen((char*)pbComm)
	) {
		StartTimer2();

	} else if (
		!memcmp( pbComm, (void*)"stop timer2", strlen("stop timer2") ) &&
		strlen("stop timer2") == strlen((char*)pbComm)
	) {
		StopTimer2();

	} else if (
		!memcmp( pbComm, (void*)"XMODEM recv", strlen("XMODEM recv") ) &&
		strlen("XMODEM recv") == strlen((char*)pbComm)
	) {
		SetXmodemRecvStateWait();

	} else if (
		!memcmp( pbComm, (void*)"refer ram_buff", strlen("refer ram_buff") ) &&
		strlen("refer ram_buff") == strlen((char*)pbComm)
	) {
//TODO hexdump形式にしたい
		sendSequenceSci1( (BYTE*)&ram_buff, RAM_BUFF_SECTION_SIZE );

	} else if (
		!memcmp( pbComm, (void*)"refer dram", strlen("refer dram") ) &&
		strlen("refer dram") == strlen((char*)pbComm)
	) {
//TODO hexdump形式にしたい
// サイズ暫定
		sendSequenceSci1( (BYTE*)&dram_start, 0x0400 );

	} else if (
		!memcmp( pbComm, (void*)"set rate", strlen("set rate") )
	) {
//TODO 
		/* LEDバー ダイナミック点灯周期切り替え */
		if        ( *(pbComm+strlen("set rate")+1) == '1' ) {
			TCORA0_8 = 50;
		} else if ( *(pbComm+strlen("set rate")+1) == '2' ) {
			TCORA0_8 = 100;
		} else if ( *(pbComm+strlen("set rate")+1) == '3' ) {
			TCORA0_8 = 200;
		} else {
			// def
			TCORA0_8 = 1;
		}

	} else if (
		!memcmp( pbComm, (void*)"dd", strlen("dd") ) &&
		strlen("dd") == strlen((char*)pbComm)
	) {
		ddddd();

	} else {
		/* 無効なコマンド */
		logPrint( "Invalid command...\n" );
	}

}

/**
 * SCI1受信データ解析
 * 文字列(改行LF)であること
 */
void analyzeSci1RecvData( void )
{
	WORD wLen = 0;
	WORD wLenTmp1 = 0;
	WORD wLenTmp2 = 0;
	BYTE *pbAddr = NULL;
	WORD wWritePos = gwWritePosBuffRecvSci1; /* 現在書き込み位置取得 */


	/* 新たな受信データが無ければ戻る */
	if ( wWritePos == gwReadPosBuffRecvSci1 ) {
		return;
	}

	memset( gbCommand, 0x00, COMMAND_SIZE_MAX );

	if ( wWritePos == 0 ) {
		/* 書き込み位置が0の場合 */

		if ( gbBuffRecvSci1[ SCI1_RECV_BUFF_SIZE -1 ] == '\n' ) {

			pbAddr = &gbBuffRecvSci1[ gwReadPosBuffRecvSci1 ];
			wLen = SCI1_RECV_BUFF_SIZE - gwReadPosBuffRecvSci1;

			/* コマンドを内部領域にコピー */
			memcpy( gbCommand, pbAddr, wLen-1 > COMMAND_SIZE_MAX ? COMMAND_SIZE_MAX : wLen-1 );

			/* コマンド解析 */
			analyzeCommand( gbCommand, wLen-1 );

//logString( pbAddr ); /*echo*/

			/* 読み出し位置更新 */
			gwReadPosBuffRecvSci1 = wWritePos;

//TODO wWritePos == wReadPosの場合の評価

		} else {
			/* LFで終わってない無視 */
		}

	} else {
		/* 書き込み位置が0以外の場合 */

		if ( gbBuffRecvSci1[ wWritePos -1 ] == '\n' ) {

			if ( wWritePos > gwReadPosBuffRecvSci1 ) {

				pbAddr = &gbBuffRecvSci1[ gwReadPosBuffRecvSci1 ];
				wLen = wWritePos - gwReadPosBuffRecvSci1;

				/* コマンドを内部領域にコピー */
				memcpy( gbCommand, pbAddr, wLen-1 > COMMAND_SIZE_MAX ? COMMAND_SIZE_MAX : wLen-1 );

				/* コマンド解析 */
				analyzeCommand( gbCommand, wLen-1 );

//logString( pbAddr ); /*echo*/

				/* 読み出し位置更新 */
				gwReadPosBuffRecvSci1 = wWritePos;

			} else if ( wWritePos < gwReadPosBuffRecvSci1 ) {

				pbAddr = &gbBuffRecvSci1[ gwReadPosBuffRecvSci1 ];
				wLen = SCI1_RECV_BUFF_SIZE - gwReadPosBuffRecvSci1;

				/* コマンドを内部領域にコピー */
				wLenTmp1 = wLen > COMMAND_SIZE_MAX ? COMMAND_SIZE_MAX : wLen;
				memcpy( gbCommand, pbAddr, wLenTmp1 );

//logString( pbAddr ); /*echo*/


				/* 受信データ続き */
				wLen = wWritePos +1;
				pbAddr = &gbBuffRecvSci1[ 0 ];

				/* コマンドを内部領域にコピー */
				wLenTmp2 = wLen-1 > (COMMAND_SIZE_MAX-wLenTmp1) ? (COMMAND_SIZE_MAX-wLenTmp1) : wLen-1;
				memcpy( gbCommand+wLenTmp1, pbAddr, wLenTmp2 );

				/* コマンド解析 */
				analyzeCommand( gbCommand, wLenTmp1 + wLenTmp2 );

//logString( pbAddr ); /*echo*/

				/* 読み出し位置更新 */
				gwReadPosBuffRecvSci1 = wWritePos;

			} else {
//TODO wWritePos == wReadPosの場合
			}
		} else {
			/* LFで終わってない場合無視 */
		}
	}
}
