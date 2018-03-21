#ifndef _STARTTLS_H_
#define _STARTTLS_H_


#define COM_SMTP_HELO		"EHLO localhost\r\n"
#define COM_SMTP_STARTTLS	"STARTTLS\r\n"


extern int StartTlsSmtp( int );

#endif
