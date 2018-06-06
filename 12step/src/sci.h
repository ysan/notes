#ifndef _SCI_H_
#define _SCI_H_

/*
 * 定数定義
 */
#define SMR_CKS_PER1			(0<<0)
#define SMR_CKS_PER4			(1<<0)
#define SMR_CKS_PER16			(2<<0)
#define SMR_CKS_PER64			(3<<0)
#define SMR_MP					(1<<2)
#define SMR_STOP				(1<<3)
#define SMR_OE 					(1<<4)
#define SMR_PE 					(1<<5)
#define SMR_CHR  				(1<<6)
#define SMR_CA					(1<<7)

#define SCR_CKE0				(1<<0)
#define SCR_CKE1				(1<<1)
#define SCR_TEIE				(1<<2)
#define SCR_MPIE				(1<<3)
#define SCR_RE					(1<<4)	/* 受信有効 */
#define SCR_TE					(1<<5)	/* 送信有効 */
#define SCR_RIE					(1<<6)	/* 受信割込み有効 */
#define SCR_TIE					(1<<7)	/* 送信割込み有効 */

#define SSR_MPBT				(1<<0)
#define SSR_MPB					(1<<1)
#define SSR_TEND				(1<<2)
#define SSR_PER					(1<<3)	/* パリティエラー */
#define SSR_FERERS				(1<<4)	/* フレーミングエラー エラーシグナルステータス */
#define SSR_ORER				(1<<5)	/* オーバーランエラー */
#define SSR_RDRF				(1<<6)	/* 受信完了 */
#define SSR_TDRE				(1<<7)	/* 送信完了 */


#define SCI1_SENDSEQ_SIZE_MAX	(256)
#define SCI1_RECV_BUFF_SIZE		(512)


/*
 * 外部宣言
 */
extern BYTE gbBuffRecvSci1[ SCI1_RECV_BUFF_SIZE ];
extern WORD gwWritePosBuffRecvSci1;
extern WORD gwReadPosBuffRecvSci1;

extern void initSci1( void );
extern void sendByteSci1( BYTE bVal );
extern void sendSequenceSci1( const BYTE *pbVal, WORD wlen );
extern BYTE recvByteSci1( void );
extern void intrRecvByteSci1( void );

#endif
