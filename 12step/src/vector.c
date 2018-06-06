#include "defines.h"
#include "intr.h"

/*
 * 割込みベクタ設定
 * リンカスクリプトの定義によりROM先頭アドレスに配置される
 * (const付けるとうまく配置されない..?)
 */
VOID_FUNC vectors[ INTR_VECT_NUM_MAX ] = {
	start,			// [ 0] リセット
	NULL,			// [ 1] Reserve
	NULL,			// [ 2] Reserve
	NULL,			// [ 3] Reserve
	NULL,			// [ 4] Reserve
	NULL,			// [ 5] Reserve
	NULL,			// [ 6] Reserve
	NULL,			// [ 7] NMI
	NULL,			// [ 8] トラップ命令
	NULL,			// [ 9] トラップ命令
	NULL,			// [10] トラップ命令
	NULL,			// [11] トラップ命令
	NULL,			// [12] IRQ0
	NULL,			// [13] IRQ1
	NULL,			// [14] IRQ2
	NULL,			// [15] IRQ3
	NULL,			// [16] IRQ4
	NULL,			// [17] IRQ5
	NULL,			// [18] Reserve
	NULL,			// [19] Reserve
	NULL,			// [20] WOVI			WDT
	NULL,			// [21] CMI				DRAM IF
	NULL,			// [22] Reserve
	NULL,			// [23] ADI				A/D
	NULL,			// [24] IMIA0			16ビットタイマ0
	NULL,			// [25] IMIB0			16ビットタイマ0
	NULL,			// [26] OVI0			16ビットタイマ0
	NULL,			// [27] Reserve
	NULL,			// [28] IMIA1			16ビットタイマ1
	NULL,			// [29] IMIB1			16ビットタイマ1
	NULL,			// [30] OVI1			16ビットタイマ1
	NULL,			// [31] Reserve
	NULL,			// [32] IMIA2			16ビットタイマ2
	NULL,			// [33] IMIB2			16ビットタイマ2
	NULL,			// [34] OVI2			16ビットタイマ2
	NULL,			// [35] Reserve
	intr_timer0,	// [36] CMIA0			8ビットタイマ0/1
	NULL,			// [37] CMIB0			8ビットタイマ0/1
	NULL,			// [38] CMIA1/CMIB1		8ビットタイマ0/1
	NULL,			// [39] TOVI0/TOVI1		8ビットタイマ0/1
	intr_timer2,	// [40] CMIA2			8ビットタイマ2/3
	NULL,			// [41] CMIB2			8ビットタイマ2/3
	NULL,			// [42] CMIA3/CMIB3		8ビットタイマ2/3
	NULL,			// [43] TOVI2/TOVI3		8ビットタイマ2/3
	NULL,			// [44] DEND0A			DMAC
	NULL,			// [45] DEND0B			DMAC
	NULL,			// [46] DEND1A			DMAC
	NULL,			// [47] DEND0B			DMAC
	NULL,			// [48] Reserve
	NULL,			// [49] Reserve
	NULL,			// [50] Reserve
	NULL,			// [51] Reserve
	NULL,			// [52] ERI0			SCI0
	NULL,			// [53] RXI0			SCI0
	NULL,			// [54] TXI0			SCI0
	NULL,			// [55] TEI0			SCI0
	NULL,			// [56] ERI1			SCI1
	intr_sci1recv,	// [57] RXI1			SCI1
	NULL,			// [58] TXI1			SCI1
	NULL,			// [59] TEI1			SCI1
	NULL,			// [60] ERI2			SCI2
	NULL,			// [61] RXI2			SCI2
	NULL,			// [62] TXI2			SCI2
	NULL,			// [63] TEI2			SCI2
};
