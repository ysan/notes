#ifndef __HSERV_H
#define __HSERV_H


#define SERVER_PORT 20001

#define CONTENTS_BASE_DIR	"./contents/"
#define CGI_BASE_DIR		"./cgi-bin/"

#define CHK_REQMSG_LINE 64

#define SP		" "
#define LF		"\n"
#define CRLF	"\r\n"

#define LFLF		"\n\n"
#define CRLFCRLF	"\r\n\r\n"

#define C_LF	'\n'
#define C_CR	'\r'


#define R 0
#define W 1


struct client_info {
	int m_nFdSockCl;
	struct sockaddr_in m_strAddrCl;
};

struct request_msg {
	char m_szMethod[32];
	char m_szPath[256];
	char m_szHttpVer[32];
	char m_szHost[256];
	char m_szConnection[32];
	int m_nContentLength;
	char *m_pszReqBdy;
};

struct response_element {
	int m_nContentType;
	int m_nCharset;
	int m_nConnection;
	int m_nKeepAlive;
	int m_nContentLength;
	int m_szResStat;
};


pthread_mutex_t g_nMutex;


int ExecCgi( const char*, const char*, char*, size_t );
int GetCgiBodyLength( const char* );
int GetPostMethod( const char*, int, int, char**, int* );
int ReturnContentType( const char*, int );
int OpenFile( char*, int );
void CreateResponseMessage( int, const struct response_element*, char*, size_t );
int CompUpperString( const char*, const char* );
int GetHeaderField( const char*, char* );
int CheckRequestMessage( const char*, int, struct request_msg* );
int RecvSend( int, int* );
void *WorkerThread( void* );
int CreateServSock( unsigned short );

#endif
