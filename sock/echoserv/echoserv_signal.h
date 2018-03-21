#ifndef _ECHOSERV_SIGNAL_H_
#define _ECHOSERV_SIGNAL_H_

/*
 * $BDj?tDj5A(B
 */
#define SIG_INFO_VAL_STACK_SIZE		(256) /* 2$B$N$Y$->h$NCM$K$9$k$3$H(B */

/*
 * $B7?Dj5A(B
 */
typedef struct send_sigque_args {
	pid_t nPid;
	int nSigKind;
	int nVal;
} ST_SEND_SIGQUE_ARGS;

/*
 * $B30It@k8@(B
 */
extern void InitSigVal( void );
extern volatile sig_atomic_t GetSigVal( void );
extern int SetSigHandle( int nSigkind );
extern int IgnoreSignal( int nSigkind );

extern void SetSigmask( void );
extern int DisableInterrupt( void );
extern int EnableInterrupt( void );
extern void InitSigValChild( void );
extern volatile sig_atomic_t GetSigValChild( void );
extern void InitSigInfoVal( void );
extern volatile sig_atomic_t GetSigInfoVal( void );
extern int SetSigHandleSiginfo( int nSigkind );
extern int SendSigque( pid_t nPid, int nSigKind, int nSiVal );
extern void PursueSigInfoVal( int (*pFuncCallback)( int, void* ), void *pArg );

#endif
