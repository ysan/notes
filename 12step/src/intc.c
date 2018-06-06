#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "defines.h"
#include "intr.h"
#include "intc.h"
#include "sci.h"
#include "timer.h"

/*
 * ソフトウエア割込みベクタ初期化
 */
void InitSoftvec( void )
{
	WORD wSoftvecType;

	for ( wSoftvecType = 0; wSoftvecType < SOFTVEC_TYPE_NUM; wSoftvecType ++ ) {
		SetIntrSoftvec( wSoftvecType, NULL );
	}
}

/*
 * ソフトウエア割込みベクタセット
 */
void SetIntrSoftvec( WORD wSoftvecType, SOFTVEC_HANDLER pHandler )
{
	/* ポインタのポインタになっているので注意 */
	SOFTVEC_HANDLER *pSoftvec = (SOFTVEC_HANDLER*)&softvec;

	/*
	 * RAM上のソフトウエア割り込みベクタの
	 * アドレス(softvec)にハンドラ関数を割り当てる
	 * (第1引数のタイプでベクタアドレスを選択)
	 */
	*(pSoftvec + wSoftvecType) = pHandler;
}

/*
 * 共通割込みハンドラ
 * ソフトウエア割込みベクタにより各ハンドラに分岐する
 */
void Interrupt( WORD wSoftvecType, DWORD dwSp )
{
	SOFTVEC_HANDLER pHandler;

	/* ポインタのポインタになっているので注意 */
	SOFTVEC_HANDLER *pSoftvec = (SOFTVEC_HANDLER*)&softvec;


	/* 以下第1引数のタイプに割り当てられたハンドラ関数を実行する */

	pHandler = *(pSoftvec + wSoftvecType);

	if ( pHandler ) {
		pHandler( wSoftvecType, dwSp );
	}
}

//TODO ここにあるのはなんかおかしい
// 各モジュールのソースに移動か
/*
 * SCI1 割り込み受信ハンドラ
 */
void IntrHandlerRecvSci1( WORD wSoftvecType, DWORD dwSp )
{
	intrRecvByteSci1();
}

/*
 * 8bitタイマ0 割り込みハンドラ
 */
void IntrHandlerTimer0( WORD wSoftvecType, DWORD dwSp )
{
	IntrTimer0();
}

/*
 * 8bitタイマ2 割り込みハンドラ
 */
void IntrHandlerTimer2( WORD wSoftvecType, DWORD dwSp )
{
	IntrTimer2();
}

