#ifndef _SERIAL_COM_H_
#define _SERIAL_COM_H_

/*
 * 定数定義
 */
#define BAUDRATE						(9600)

#define SERIAL_COM_STDIN_BUFF_SIZE		(1024)
#define SERIAL_COM_RECV_BUFF_SIZE		(65536)
#define SERIAL_COM_COMMAND_BUFF_SIZE	(32)

#define COMMAND_XMODEM_SEND				"XMODEM send"
#define COMMAND_DUMP_ON					"dump on"
#define COMMAND_DUMP_OFF				"dump off"


/*
 * 外部宣言
 */
extern unsigned char RecvByte( int nFd, int *pnFlagError );
extern int SendByte( int nFd, const unsigned char *c );
extern int SendSequence( int nFd, const unsigned char *pszBuff, size_t nBuffSize );

#endif
