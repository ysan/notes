#ifndef _TIMER_H_
#define _TIMER_H_

/*
 * 定数定義
 */
#define TCR_8_CKS_PER1			(0<<0)	/* クロック入力禁止 */
#define TCR_8_CKS_PER8			(1<<0)	/* 内部クロックφ/8 立ち上りでカウント */
#define TCR_8_CKS_PER64			(2<<0)	/* 内部クロックφ/64 立ち上りでカウント */
#define TCR_8_CKS_PER8192		(3<<0)	/* 内部クロックφ/8192 立ち上りでカウント */
#define TCR_8_CKS_CASCADE		(4<<0)	/* チャネル間カスケード接続 */
#define TCR_8_CKS_EXTCK_RZ		(5<<0)	/* 外部クロック立ち上りでカウント */
#define TCR_8_CKS_EXTCK_FL		(6<<0)	/* 外部クロック立ち下りでカウント */
#define TCR_8_CKS_EXTCK_RZFL	(7<<0)	/* 外部クロック両エッジでカウント */
#define TCR_8_CCLR_PROHIBIT		(0<<3)	/* カウンタクリア禁止 */
#define TCR_8_CCLR_CMA			(1<<3)	/* コンペアマッチAでカウンタクリア */
#define TCR_8_CCLR_CMB_ICB		(2<<3)	/* コンペアマッチB/インプットキャプチャBでカウンタクリア */
#define TCR_8_CCLR_ICB			(3<<3)	/* インプットキャプチャBでカウンタクリア */
#define TCR_8_OVIE				(1<<5)	/* オーバーフロー割り込みイネーブル 1:enable */
#define TCR_8_CMIEA				(1<<6)	/* コンペアマッチA割り込みイネーブル 1:enable */
#define TCR_8_CMIEB				(1<<7)	/* コンペアマッチB割り込みイネーブル 1:enable */

#define TCSR_8_A_OS_OUT_NO		(0<<0)	/* コンペアマッチAで変化しない */
#define TCSR_8_A_OS_OUT_L		(1<<0)	/* コンペアマッチAでL出力 */
#define TCSR_8_A_OS_OUT_H		(2<<0)	/* コンペアマッチAでH出力 */
#define TCSR_8_A_OS_OUT_TGL		(3<<0)	/* コンペアマッチAでトグル出力 */
#define TCSR_8_B_OS_OUT_NO		(0<<2)	/* コンペアマッチBで変化しない */			/*ICEビット0*/
#define TCSR_8_B_OS_OUT_L		(1<<2)	/* コンペアマッチBでL出力 */				/*ICEビット0*/
#define TCSR_8_B_OS_OUT_H		(2<<2)	/* コンペアマッチBでH出力 */				/*ICEビット0*/
#define TCSR_8_B_OS_OUT_TGL		(3<<2)	/* コンペアマッチBでトグル出力 */			/*ICEビット0*/
#if 0
#define TCSR_8_B_IS_RZ			(0<<2)	/* インプットキャプチャB 立ち上りエッジ */	/*ICEビット1*/
#define TCSR_8_B_IS_FL			(1<<2)	/* インプットキャプチャB 立ち下りエッジ */	/*ICEビット1*/
#define TCSR_8_B_OS_RZFL		(2<<2)	/* インプットキャプチャB 両エッジ */		/*ICEビット1*/
#define TCSR_8_B_OS_RZFL		(3<<2)	/* インプットキャプチャB 両エッジ */		/*ICEビット1*/
#endif
#define TCSR_8_ADTE_ICE			(1<<4)	/* ADトリガ/インプットキャプイネーブル */
#define TCSR_8_OVF				(1<<5)	/* オーバーフローフラグ */
#define TCSR_8_CMFA				(1<<6)	/* コンペアマッチフラグA */
#define TCSR_8_CMFB				(1<<7)	/* コンペアマッチ/インプットキャプチャフラグB */


/*
 * 外部宣言
 */
extern void InitTimer0( void );
extern void InitTimer2( void );
extern void StartTimer0( void );
extern void StartTimer2( void );
extern void StopTimer0( void );
extern void StopTimer2( void );
extern void IntrTimer0( void );
extern void IntrTimer2( void );

#endif
