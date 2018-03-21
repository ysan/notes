#ifndef _ECHOSERV_CHILD_PROC_H_
#define _ECHOSERV_CHILD_PROC_H_

#include <netinet/in.h>

/*
 * $BDj?tDj5A(B
 */

#define RECV_BUFF_SIZE		(1024)


/*
 * $B7?Dj5A(B
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
 * $B30It@k8@(B
 */
extern int ChildProcMain( ST_CLIENT_INFO* );

#endif
