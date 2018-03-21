#ifndef _ECHOSERV_ACCEPT_H_
#define _ECHOSERV_ACCEPT_H_

/*
 * 定数定義
 */
#define TIMEOUT_SEC_VAL			(2)
#define TIMEOUT_USEC_VAL		(500000)

/*
 * 型定義
 */


/*
 * 外部宣言
 */
extern int SelectAcceptFork( int nFdSockSv );

#endif
