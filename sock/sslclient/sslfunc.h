#ifndef _SSLFUNC_H_
#define _SSLFUNC_H_

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>


typedef int (*INT_FUNC_INIT_SSL)( int );


extern SSL *gpstrSsl;
extern BIO *gpstrBio;

extern const INT_FUNC_INIT_SSL pFuncInitSsl[3];

extern int InitSsl( int );
extern int InitSslBio( int );
extern int InitPlainBio( int );
extern int RecvBio( BIO*, unsigned char*, size_t, int* );
extern int SendBio( BIO*, const unsigned char*, size_t );


#endif
