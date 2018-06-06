#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "defines.h"
#include "xmodem.h"
#include "sci.h"
#include "intc.h"
#include "timer.h"


/*
 * 変数宣言
 */
static BYTE gbStateRecv;
static BYTE gbCntRecvIn;
static WORD gwBlockNo;
static WORD gwCntCopy; 
static ST_XMODEM_INFO gstXmodemInfo;

//static void *gpDestAddr = &ram_buff;
//static DWORD gdwDestSize = RAM_BUFF_SECTION_SIZE;
static void *gpDestAddr = &dram_start;
static DWORD gdwDestSize = DRAM_SECTION_SIZE;

/*
 * プロトタイプ宣言
 */
static BYTE StoreXmodemRecvData( BYTE *pbArg, DWORD dwArgSize );
static void RejectEof( BYTE *pbArg );


/*
 * XMODEM受信初期化
 */
void InitXmodemRecv( void )
{
	gbStateRecv = XMODEM_RECV_STATE_INIT;
	gbCntRecvIn = 0;
	gwBlockNo = 1; /* ブロックナンバーは1から開始 */
	gwCntCopy = 0;
	memset( &gstXmodemInfo, 0x00, XMODEM_SIZE );
}

/*
 * XMODEM受信待機状態セット
 */
void SetXmodemRecvStateWait( void )
{
	gbStateRecv = XMODEM_RECV_STATE_WAIT;
}

/*
 * XMODEM制御信号受信確認 (SOH,EOT,CAN)
 * 割り込み側で呼び出す
 */
void CheckXmodemRecvControlSignal( BYTE bArg )
{
	if (
		( gbStateRecv != XMODEM_RECV_STATE_WAIT ) &&
		( gbStateRecv != XMODEM_RECV_STATE_READY )
	) {
		return;
	}

	if ( bArg == XMODEM_SOH ) {
		/* SOHを受信 */

		gbStateRecv = XMODEM_RECV_STATE_LOAD;


	} else if ( bArg == XMODEM_EOT ) {
		/* EOTを受信 */

		sendByteSci1( XMODEM_ACK );

		/* 末尾EOF除去 */
		RejectEof( (BYTE*)gpDestAddr );

		InitXmodemRecv();

		/* 読み出し位置を更新 */
		gwReadPosBuffRecvSci1 += 1;

//StopTimer0();

	} else if ( bArg == XMODEM_CAN ) {
		/* CANを受信 */

		InitXmodemRecv();

		/* 読み出し位置を更新 */
		gwReadPosBuffRecvSci1 += 1;

//StopTimer0();

	} else {
		/* 処理なし */
	}
}

/*
 * 受信カウント
 * 割り込み側で呼び出す
 */
void CountXmodemRecv( void )
{
	if ( gbStateRecv != XMODEM_RECV_STATE_LOAD ) {
		return;
	}

	gbCntRecvIn ++;

	if ( gbCntRecvIn < XMODEM_SIZE ) {
		/* 受信中 */

	} else {
		/* 受信完了 */

		gbStateRecv = XMODEM_RECV_STATE_ANALYZE;
		gbCntRecvIn = 0;
	}
}

/*
 * XMODEM受信データ解析
 */
void AnalyzeXmodemRecv( void )
{
	BYTE i = 0;
	WORD wCksum = 0;
	WORD wWritePos = gwWritePosBuffRecvSci1; /* 現在の書き込み位置を取得 */

	if ( gbStateRecv != XMODEM_RECV_STATE_ANALYZE ) {
		return;
	}


	/* 受信データを内部領域にコピー */
	if ( ( SCI1_RECV_BUFF_SIZE - gwReadPosBuffRecvSci1 ) >= XMODEM_SIZE ) {

		memcpy( &gstXmodemInfo, &gbBuffRecvSci1[ gwReadPosBuffRecvSci1 ], XMODEM_SIZE );		

	} else {
		memcpy (
			&gstXmodemInfo,
			&gbBuffRecvSci1[ gwReadPosBuffRecvSci1 ],
			SCI1_RECV_BUFF_SIZE - gwReadPosBuffRecvSci1
		);
		memcpy (
			(BYTE*)( &gstXmodemInfo + ( SCI1_RECV_BUFF_SIZE - gwReadPosBuffRecvSci1 ) ),
			&gbBuffRecvSci1[ 0 ],
			XMODEM_SIZE - ( SCI1_RECV_BUFF_SIZE - gwReadPosBuffRecvSci1 )
		);
	}

	/* 読み出し位置を更新 */
	gwReadPosBuffRecvSci1 = wWritePos;


	/* ブロックナンバーチェック1 */
	if ( (BYTE)gwBlockNo != gstXmodemInfo.bBlockNo ) {
		/* エラー */
		sendByteSci1( XMODEM_NAK );
		gbStateRecv = XMODEM_RECV_STATE_READY;
		return;
	}

	/* ブロックナンバーチェック2 */
	if ( ( gstXmodemInfo.bBlockNo ^ gstXmodemInfo.bBlockNoNgtv ) != 0xff ) {
		/* エラー */
		sendByteSci1( XMODEM_NAK );
		gbStateRecv = XMODEM_RECV_STATE_READY;
		return;
	}

	/* データ部チェックサム */
	i = 0;
	wCksum = 0;
	while ( i < XMODEM_DATA_SIZE ) {
		wCksum += gstXmodemInfo.bData[ i ];
		i ++;
	}

	if ( ( wCksum & 0xff ) != gstXmodemInfo.bCksum ) {
		/* エラー */
		sendByteSci1( XMODEM_NAK );
		gbStateRecv = XMODEM_RECV_STATE_READY;
		return;
	}

	/* 解析済みデータを格納 */
	if ( !StoreXmodemRecvData( (BYTE*)gpDestAddr, gdwDestSize ) ) {
		/*
		 * エラー
		 * 格納先に空きが無い場合
		 */
		sendByteSci1( XMODEM_CAN );
		InitXmodemRecv();
//StopTimer0();
		return;
	}


	/* 1ブロック受信完了 */
	sendByteSci1( XMODEM_ACK );
	gbStateRecv = XMODEM_RECV_STATE_READY;
	gwBlockNo ++;
	gwBlockNo &= 0xff;
}

/*
 * 解析済みデータ格納
 */
static BYTE StoreXmodemRecvData( BYTE *pbArg, DWORD dwArgSize )
{
	/* 格納先の残りサイズ確認 */
	if ( dwArgSize < ( XMODEM_DATA_SIZE * gwCntCopy ) + XMODEM_DATA_SIZE ) {
		/* 残りサイズ足りない場合はエラー */
		return FALSE;
	}

	memcpy( pbArg + ( XMODEM_DATA_SIZE * gwCntCopy ), &gstXmodemInfo.bData, XMODEM_DATA_SIZE );
	memset( &gstXmodemInfo, 0x00, XMODEM_SIZE );

	gwCntCopy ++;

	return TRUE;
}

/*
 * 末尾EOF除去
 */
static void RejectEof( BYTE *pbArg )
{
	BYTE bCnt = 0;

	if ( !pbArg ) {
		return;
	}

	while ( *( pbArg + ( XMODEM_DATA_SIZE * gwCntCopy ) - bCnt -1 ) == (BYTE)XMODEM_EOF ) {

		*( pbArg + ( XMODEM_DATA_SIZE * gwCntCopy ) - bCnt -1 ) = 0x00;
		bCnt ++;
	}
}

/*
 * XMODEM受信状態取得
 */
BYTE GetXmodemRecvState( void )
{
	return gbStateRecv;
}
