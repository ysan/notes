#ifndef _INTR_H_
#define _INTR_H_

/*
 * アセンブラソースヘッダのため
 * defineのみ使用可
 * enumは使えません
 */

/*
 * 定数定義
 */
#define SOFTVEC_TYPE_NUM		(5)		/* ソフトウエア割込みベクタ数 */ 

#define SOFTVEC_TYPE_SOFTERR	(0)
#define SOFTVEC_TYPE_SYSCALL	(1)
#define SOFTVEC_TYPE_SCI1RECV	(2)
#define SOFTVEC_TYPE_TIMER0		(3)
#define SOFTVEC_TYPE_TIMER2		(4)


/* アセンブラ関数の外部宣言は defines.h で */

#endif
