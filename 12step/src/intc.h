#ifndef _INTC_H_
#define _INTC_H_

#include "defines.h"

/*
 * 定数定義
 */
#define IPRA_IPRA0		(1<<0)	/* 16ビットタイマ2 割り込みプライオリティビット */
#define IPRA_IPRA1		(1<<1)	/* 16ビットタイマ1 割り込みプライオリティビット */
#define IPRA_IPRA2		(1<<2)	/* 16ビットタイマ0 割り込みプライオリティビット */
#define IPRA_IPRA3		(1<<3)	/* WDT,DRAM-IF,AD変換 割り込みプライオリティビット */
#define IPRA_IPRA4		(1<<4)	/* IRQ4,5 割り込みプライオリティビット */
#define IPRA_IPRA5		(1<<5)	/* IRQ2,3 割り込みプライオリティビット */
#define IPRA_IPRA6		(1<<6)	/* IRQ1 割り込みプライオリティビット */
#define IPRA_IPRA7		(1<<7)	/* IRQ0 割り込みプライオリティビット */

#define IPRB_IPRB0		(1<<0)	/* Reserve */
#define IPRB_IPRB1		(1<<1)	/* SCI2 割り込みプライオリティビット */
#define IPRB_IPRB2		(1<<2)	/* SCI1 割り込みプライオリティビット */
#define IPRB_IPRB3		(1<<3)	/* SCI0 割り込みプライオリティビット */
#define IPRB_IPRB4		(1<<4)	/* Reserve */
#define IPRB_IPRB5		(1<<5)	/* DMAC 割り込みプライオリティビット */
#define IPRB_IPRB6		(1<<6)	/* 8ビットタイマ2,3 割り込みプライオリティビット */
#define IPRB_IPRB7		(1<<7)	/* 8ビットタイマ0,1 割り込みプライオリティビット */

#define EI()	asm volatile ("andc.b #0x3f,ccr")	/* 割り込み許可 */
#define DI()	asm volatile ("orc.b #0xc0,ccr")	/* 割り込み禁止 */


/*
 * 型定義
 */
typedef void (*SOFTVEC_HANDLER)( WORD wSoftvecType, DWORD dwSp );


/*
 * 外部宣言
 */
extern void InitSoftvec( void );
extern void SetIntrSoftvec( WORD wSoftvecType, SOFTVEC_HANDLER pHandler );
extern void Interrupt( WORD wSoftvecType, DWORD dwSp );
extern void IntrHandlerRecvSci1( WORD wSoftvecType, DWORD dwSp );
extern void IntrHandlerTimer0( WORD wSoftvecType, DWORD dwSp );
extern void IntrHandlerTimer2( WORD wSoftvecType, DWORD dwSp );

#endif
