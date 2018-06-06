#ifndef _BSC_H_
#define _BSC_H_


/*
 * 定数定義
 */
#define ABWCR_ABW0			(1<<0)	/* エリア0 バス幅コントロール 0:16bit 1:8bit */
#define ABWCR_ABW1			(1<<1)	/* エリア1 バス幅コントロール 0:16bit 1:8bit */
#define ABWCR_ABW2			(1<<2)	/* エリア2 バス幅コントロール 0:16bit 1:8bit */
#define ABWCR_ABW3			(1<<3)	/* エリア3 バス幅コントロール 0:16bit 1:8bit */
#define ABWCR_ABW4			(1<<4)	/* エリア4 バス幅コントロール 0:16bit 1:8bit */
#define ABWCR_ABW5			(1<<5)	/* エリア5 バス幅コントロール 0:16bit 1:8bit */
#define ABWCR_ABW6			(1<<6)	/* エリア6 バス幅コントロール 0:16bit 1:8bit */
#define ABWCR_ABW7			(1<<7)	/* エリア7 バス幅コントロール 0:16bit 1:8bit */

#define ASTCR_AST0			(1<<0)	/* エリア0 アクセスステートコントロール 0:2state 1:3state  */
#define ASTCR_AST1			(1<<1)	/* エリア1 アクセスステートコントロール 0:2state 1:3state  */
#define ASTCR_AST2			(1<<2)	/* エリア2 アクセスステートコントロール 0:2state 1:3state  */
#define ASTCR_AST3			(1<<3)	/* エリア3 アクセスステートコントロール 0:2state 1:3state  */
#define ASTCR_AST4			(1<<4)	/* エリア4 アクセスステートコントロール 0:2state 1:3state  */
#define ASTCR_AST5			(1<<5)	/* エリア5 アクセスステートコントロール 0:2state 1:3state  */
#define ASTCR_AST6			(1<<6)	/* エリア6 アクセスステートコントロール 0:2state 1:3state  */
#define ASTCR_AST7			(1<<7)	/* エリア7 アクセスステートコントロール 0:2state 1:3state  */

#define WCRH_W4_NON			(0<<0)	/* エリア4 ウェイトコントロール */
#define WCRH_W4_1STATE		(1<<0)	/* エリア4 ウェイトコントロール */
#define WCRH_W4_2STATE		(2<<0)	/* エリア4 ウェイトコントロール */
#define WCRH_W4_3STATE		(3<<0)	/* エリア4 ウェイトコントロール */
#define WCRH_W5_NON			(0<<2)	/* エリア5 ウェイトコントロール */
#define WCRH_W5_1STATE		(1<<2)	/* エリア5 ウェイトコントロール */
#define WCRH_W5_2STATE		(2<<2)	/* エリア5 ウェイトコントロール */
#define WCRH_W5_3STATE		(3<<2)	/* エリア5 ウェイトコントロール */
#define WCRH_W6_NON			(0<<4)	/* エリア6 ウェイトコントロール */
#define WCRH_W6_1STATE		(1<<4)	/* エリア6 ウェイトコントロール */
#define WCRH_W6_2STATE		(2<<4)	/* エリア6 ウェイトコントロール */
#define WCRH_W6_3STATE		(3<<4)	/* エリア6 ウェイトコントロール */
#define WCRH_W7_NON			(0<<6)	/* エリア7 ウェイトコントロール */
#define WCRH_W7_1STATE		(1<<6)	/* エリア7 ウェイトコントロール */
#define WCRH_W7_2STATE		(2<<6)	/* エリア7 ウェイトコントロール */
#define WCRH_W7_3STATE		(3<<6)	/* エリア7 ウェイトコントロール */

#define WCRL_W0_NON			(0<<0)	/* エリア0 ウェイトコントロール */
#define WCRL_W0_1STATE		(1<<0)	/* エリア0 ウェイトコントロール */
#define WCRL_W0_2STATE		(2<<0)	/* エリア0 ウェイトコントロール */
#define WCRL_W0_3STATE		(3<<0)	/* エリア0 ウェイトコントロール */
#define WCRL_W1_NON			(0<<2)	/* エリア1 ウェイトコントロール */
#define WCRL_W1_1STATE		(1<<2)	/* エリア1 ウェイトコントロール */
#define WCRL_W1_2STATE		(2<<2)	/* エリア1 ウェイトコントロール */
#define WCRL_W1_3STATE		(3<<2)	/* エリア1 ウェイトコントロール */
#define WCRL_W2_NON			(0<<4)	/* エリア2 ウェイトコントロール */
#define WCRL_W2_1STATE		(1<<4)	/* エリア2 ウェイトコントロール */
#define WCRL_W2_2STATE		(2<<4)	/* エリア2 ウェイトコントロール */
#define WCRL_W2_3STATE		(3<<4)	/* エリア2 ウェイトコントロール */
#define WCRL_W3_NON			(0<<6)	/* エリア3 ウェイトコントロール */
#define WCRL_W3_1STATE		(1<<6)	/* エリア3 ウェイトコントロール */
#define WCRL_W3_2STATE		(2<<6)	/* エリア3 ウェイトコントロール */
#define WCRL_W3_3STATE		(3<<6)	/* エリア3 ウェイトコントロール */

#define BRCR_BRLE			(1<<0)	/* バスリリースイネーブル 1:バス権開放 */
#define BRCR_A20E			(1<<4)	/* アドレス20イネーブル 0enable */
#define BRCR_A21E			(1<<5)	/* アドレス21イネーブル 0enable */
#define BRCR_A22E			(1<<6)	/* アドレス22イネーブル 0enable */
#define BRCR_A23E			(1<<7)	/* アドレス23イネーブル 0enable */

#define BCR_WAITE			(1<<0)	/* WAIT端子イネーブル 1enable */
#define BCR_RDEA			(1<<1)	/* エリア分割単位選択 */
#define BCR_EMC				(1<<2)	/* 拡張メモリマップコントロール */
#define BCR_BRSTS0			(1<<3)	/* バーストサイクルセレクト0 */
#define BCR_BRSTS1			(1<<4)	/* バーストサイクルセレクト1 */
#define BCR_BROME			(1<<5)	/* バーストROMイネーブル0 */
#define BCR_ICIS0			(1<<6)	/* アイドルサイクル0 */
#define BCR_ICIS1			(1<<7)	/* アイドルサイクル1 */

#define CSCR_CS4E			(1<<4)	/* チップセレクト4イネーブル */
#define CSCR_CS5E			(1<<5)	/* チップセレクト5イネーブル */
#define CSCR_CS6E			(1<<6)	/* チップセレクト6イネーブル */
#define CSCR_CS7E			(1<<7)	/* チップセレクト7イネーブル */

#define DRCRA_RFSHE			(1<<0)	/* リフレッシュ端子イネーブル 1enable */
#define DRCRA_SRFMD			(1<<1)	/* セルフリフレッシュモード */
#define DRCRA_RDM			(1<<2)	/* RASダウンモード */
#define DRCRA_BE			(1<<3)	/* バーストアクセスイネーブル */
/* DRAMエリアセレクト */
#define DRCRA_DRAS_000		(0<<5)	/* エリア5:通常           4:通常           3:通常           2:通常           */
#define DRCRA_DRAS_001		(1<<5)	/* エリア5:通常           4:通常           3:通常           2:DRAM空間(CS2_) */
#define DRCRA_DRAS_010		(2<<5)	/* エリア5:通常           4:通常           3:DRAM空間(CS3_) 2:DRAM空間(CS2_) */
#define DRCRA_DRAS_011		(3<<5)	/* エリア5:通常           4:通常           3:-->            2:DRAM空間(CS2_) */
#define DRCRA_DRAS_100		(4<<5)	/* エリア5:通常           4:DRAM空間(CS4_) 3:DRAM空間(CS3_) 2:DRAM空間(CS2_) */
#define DRCRA_DRAS_101		(5<<5)	/* エリア5:DRAM空間(CS5_) 4:DRAM空間(CS4_) 3:DRAM空間(CS3_) 2:DRAM空間(CS2_) */
#define DRCRA_DRAS_110		(6<<5)	/* エリア5:-->            4:DRAM空間(CS4_) 3:-->            2:DRAM空間(CS2_) */
#define DRCRA_DRAS_111		(7<<5)	/* エリア5:-->            4:-->            3:-->            2:DRAM空間(CS2_) */

#define DRCRB_RLW			(1<<0)	/* リフレッシュサイクルウェイトコントロール */
#define DRCRB_RCW			(1<<1)	/* RAS_ CAS_ 間ウェイト */
#define DRCRB_TPC			(1<<2)	/* TPサイクルコントロール */
#define DRCRB_RCYCE			(1<<4)	/* リフレッシュサイクルイネーブル 1enable */
#define DRCRB_CSEL			(1<<5)	/* CAS_出力端子選択 1enable */
#define DRCRB_MXC_00		(0<<6)	/* マルチプレクスコントロール 8bit */
#define DRCRB_MXC_01		(1<<6)	/* マルチプレクスコントロール 9bit */
#define DRCRB_MXC_10		(2<<6)	/* マルチプレクスコントロール 10bit */

#define RTMCSR_CKS_STOP		(0<<3)	/* リフレッシュカウンタクロックセレクト 停止 */
#define RTMCSR_CKS_PER2		(1<<3)	/* リフレッシュカウンタクロックセレクト φ/2 */
#define RTMCSR_CKS_PER8		(2<<3)	/* リフレッシュカウンタクロックセレクト φ/8 */
#define RTMCSR_CKS_PER32	(3<<3)	/* リフレッシュカウンタクロックセレクト φ/32 */
#define RTMCSR_CKS_PER128	(4<<3)	/* リフレッシュカウンタクロックセレクト φ/128 */
#define RTMCSR_CKS_PER512	(5<<3)	/* リフレッシュカウンタクロックセレクト φ/512 */
#define RTMCSR_CKS_PER2048	(6<<3)	/* リフレッシュカウンタクロックセレクト φ/2048 */
#define RTMCSR_CKS_PER4096	(7<<3)	/* リフレッシュカウンタクロックセレクト φ/4096 */
#define RTMCSR_CMIE			(1<<6)	/* コンペマッチ割り込みイネーブル 1enable */
#define RTMCSR_CMF			(1<<7)	/* コンペマッチフラグ */

#define ADRCR_ADRCL			(1<<0)	/* アドレスコントロールビット */


/*
 * 外部宣言
 */
extern void InitBsc( void );

#endif
