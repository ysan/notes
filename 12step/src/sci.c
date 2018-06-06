#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "defines.h"
#include "reg3067.h"
#include "sci.h"
#include "intr.h"
#include "intc.h"
#include "xmodem.h"


/*
 * 変数宣言
 */
BYTE gbBuffRecvSci1[ SCI1_RECV_BUFF_SIZE ];
WORD gwWritePosBuffRecvSci1;
WORD gwReadPosBuffRecvSci1;


/**
 * SCI1 初期化
 */
void initSci1( void )
{
	/* 一旦 送受信無効化 */
	SCR1 = 0x00;

	/* 分周なし */
	SMR1 = 0x00;

	/* 20MHzのクロックから9600bpsを生成(25MHzの場合は80にする) */
	/* (水晶20MHz 分周なし) */
	BRR1 = 64;

	/* 送受信許可 */
	SCR1 = SCR_RE | SCR_TE;

	/* ステータス クリア */
	SSR1 = 0x00;


	/* 割り込み受信用バッファ クリア */
	memset( gbBuffRecvSci1, 0x00, sizeof(gbBuffRecvSci1) );
	gwWritePosBuffRecvSci1 = 0;
	gwReadPosBuffRecvSci1 = 0;


	/* 割り込みハンドラセット */
	SetIntrSoftvec( SOFTVEC_TYPE_SCI1RECV, IntrHandlerRecvSci1 );

	/* 受信割り込み要求許可 */
	SCR1 |= SCR_RIE;

	/* 割り込み優先度レベル1 */
	IPRB |= IPRB_IPRB2;
}

/**
 * SCI1 送信
 */
void sendByteSci1( BYTE bVal )
{
	/*
	 * 送信可能になるまで待つ
	 * (TDREが0であれば待つ)
	 */
	while ( !( SSR1 & SSR_TDRE ) ) {
	}

	/* 送信する1byteをTDRに格納 */
	TDR1 = bVal;

	/*
	 * TDREを0にセット
	 * TDRからTSRに転送送信
	 */
	SSR1 &= ~SSR_TDRE;
}

/**
 * SCI1 バイト列送信
 */
void sendSequenceSci1( const BYTE *pbVal, WORD wLen )
{
	WORD wCnt = 0;

	/* 引数チェック */
	if ( ( !pbVal ) || ( wLen == 0 ) ) {
		return;
	}

	/* 送信サイズ切り詰め */
//	wlen = ( wLen > SCI1_SENDSEQ_SIZE_MAX ) ? SCI1_SENDSEQ_SIZE_MAX : wLen;

	/* byte送信 */
	while ( wCnt < wLen ) {

		sendByteSci1( *pbVal );

		wCnt ++;
		pbVal ++;
	}
}

/**
 * SCI1 受信
 */
BYTE recvByteSci1( void )
{
	/*
	 * RDRに受信データが格納されるまで待つ
	 * (RDRFが0であれば待つ)
	 */
	while( !( SSR1 & SSR_RDRF ) ) {
	}

	/* RDRFクリア */
	SSR1 &= ~SSR_RDRF;

	/* 受信データを返す */
	return RDR1;
}

/**
 * SCI1 割り込み受信
 */
void intrRecvByteSci1( void )
{
	/* エラー判定 */
	if (
		!( SSR1 & SSR_PER ) && 
		!( SSR1 & SSR_FERERS ) && 
		!( SSR1 & SSR_ORER )
	) {
		/* 受信正常時 */

		/* 受信データを格納 */
		gbBuffRecvSci1[ gwWritePosBuffRecvSci1 ] = RDR1;
		gwWritePosBuffRecvSci1 ++;
		gwWritePosBuffRecvSci1 &= SCI1_RECV_BUFF_SIZE -1;

		/* XMODEM受信確認 */
		CheckXmodemRecvControlSignal( RDR1 );
		CountXmodemRecv();


		/* RDRFクリア */
		SSR1 &= ~SSR_RDRF;

// TODO
		/*DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD*/
		/* バッファ一周したばあいの確認 */

	} else {
		/* 受信エラー */
// TODO

	}
}
