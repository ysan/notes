#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>


static sigset_t gnSigset;


void handler( int nSigno )
{
	printf( "Interrupt occurred. [%s]\n", strsignal( nSigno ) );
	return;
}

void _DI( void )
{
	sigprocmask( SIG_BLOCK, &gnSigset, NULL);
}

void _EI( void )
{
	sigprocmask( SIG_UNBLOCK, &gnSigset, NULL);
}

int main()
{
	unsigned char szBuff[128];
	struct sigaction strSigact;


	sigemptyset( &gnSigset );
	sigaddset( &gnSigset, SIGRTMIN );


	memset( &strSigact, 0x00, sizeof(strSigact) );
	strSigact.sa_handler = handler;
	strSigact.sa_mask = gnSigset;
	sigaction( SIGRTMIN, &strSigact, NULL );


	while(1){

		_EI();

		read( STDIN_FILENO, szBuff, sizeof(szBuff) );

		_DI();

		printf( "return read()...\n" );
		sleep(5);

	}


	exit( EXIT_SUCCESS );
}
