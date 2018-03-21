#ifndef _ECHOSERV_CHILD_PROC_H_
#define _ECHOSERV_CHILD_PROC_H_

#include <netinet/in.h>

/*
 * 定数定義
 */

#define RECV_BUFF_SIZE		(1024)


/*
 * 型定義
 */
typedef struct client_info {
	int nFdSockSv;
	int nFdSockCl;
	struct sockaddr_in stAddrCl;
	int nShmIdMsgInfo;
	int nShmIdChildInfoTbl;
	int nShmIdChildInfoTblPtr;
	int nSemIdMsgInfo;
} ST_CLIENT_INFO;


/*
 * 外部宣言
 */
extern int ChildProcMain( ST_CLIENT_INFO* );

#endif
