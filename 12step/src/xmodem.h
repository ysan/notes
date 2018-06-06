#ifndef _XMODEM_H_
#define _XMODEM_H_

/*
 * 定数定義
 */
#define XMODEM_SOH			(0x01)
#define XMODEM_STX			(0x02)
#define XMODEM_EOT			(0x04)
#define XMODEM_ACK			(0x06)
#define XMODEM_NAK			(0x15)
#define XMODEM_CAN			(0x18)
#define XMODEM_EOF			(0x1a)

#define XMODEM_DATA_SIZE	(128)
#define XMODEM_SIZE			(1+1+1+XMODEM_DATA_SIZE+1)

enum {
	XMODEM_RECV_STATE_INIT = 0,
	XMODEM_RECV_STATE_WAIT,
	XMODEM_RECV_STATE_READY,
	XMODEM_RECV_STATE_LOAD,
	XMODEM_RECV_STATE_ANALYZE,
};


/*
 * 型定義
 */
typedef struct xmodem_info {
	BYTE bHeader;
	BYTE bBlockNo;
	BYTE bBlockNoNgtv;
	BYTE bData[ XMODEM_DATA_SIZE ];
	BYTE bCksum;
} ST_XMODEM_INFO;


/*
 * 外部宣言
 */
extern void InitXmodemRecv( void );
extern void SetXmodemRecvStateWait( void );
extern void CheckXmodemRecvControlSignal( BYTE bArg );
extern void CountXmodemRecv( void );
extern void AnalyzeXmodemRecv( void );
extern BYTE GetXmodemRecvState( void );

#endif
