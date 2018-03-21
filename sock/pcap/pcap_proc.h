#ifndef __PCAP_PROC_H
#define __PCAP_PROC_H


#define MSG_QUE_KEY				((key_t)0x01)
#define MSG_QUE_MODE			(0666)
#define MSG_TYPE_NO				(1)
#define MSGSND_RETRY_COUNT		(5)
#define MSGSND_RETRY_TIME		(5)

struct msg_packet_info {
	long m_nType;
	int m_nPacketSize;
	unsigned char m_szPacketData[2048];
	char m_szCapTime[4+1+2+1+2+1+2+1+2+1+2+1+3+1];
};

extern int gnMsqid;

extern int CreateMsgQue( void );
extern int DeleteMsgQue( void );
extern int RecvPacketProc( int, int );
extern int AnalyzePacketProc( int );
extern int CheckWaitpid( void );

#endif
