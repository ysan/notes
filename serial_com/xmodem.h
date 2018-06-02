#ifndef _XMODEM_H_
#define _XMODEM_H_


/*
 * 定数定義
 */
#define XMODEM_SOH					(0x01)
#define XMODEM_STX					(0x02)
#define XMODEM_EOT					(0x04)
#define XMODEM_ACK					(0x06)
#define XMODEM_NAK					(0x15)
#define XMODEM_CAN					(0x18)
#define XMODEM_EOF					(0x1a)

#define XMODEM_DATA_SIZE			(128)
#define XMODEM_SIZE					(1+1+1+XMODEM_DATA_SIZE+1)

#define XMODEM_SEND_RSLT_RETRY_CNT	(10)

#define XMODEM_READ_BUFF_SIZE		(65536)


/*
 * 型定義
 */
typedef struct xmodem_info {
    unsigned char cHeader;
    unsigned char cBlockNo;
    unsigned char cBlockNoNgtv;
    unsigned char szData[ XMODEM_DATA_SIZE ];
    unsigned char cCksum;
} ST_XMODEM_INFO;


/*
 * 外部宣言
 */
extern int SendXmodem( int nFdCom, unsigned char *pszBuff );


#endif
