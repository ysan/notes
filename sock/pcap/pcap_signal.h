#ifndef __PCAP_SIGNAL_H
#define __PCAP_SIGNAL_H


extern volatile sig_atomic_t gCatchSignal;

extern int SetSignalMask( void );
extern int SetSigHandle( int );
extern int WaitSignal( pid_t* );

#endif
