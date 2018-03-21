#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

int main()
{
	int nSigno = 0;
	sigset_t nSigset;

	sigemptyset( &nSigset );
	sigaddset( &nSigset, SIGINT );
	sigaddset( &nSigset, SIGUSR1 );
	sigaddset( &nSigset, SIGRTMIN );
	sigaddset( &nSigset, SIGRTMIN+1 );


	if( sigprocmask( SIG_BLOCK, &nSigset, NULL) < 0 ){
		perror( "sigprocmask()" );
		exit( EXIT_FAILURE );
	}

struct sigaction strSigact;
memset(&strSigact,0x00,sizeof(strSigact));
strSigact.sa_handler = SIG_IGN;
sigaction(SIGCHLD,&strSigact,NULL);
sigaction(SIGPIPE,&strSigact,NULL);

FILE *fp = NULL;
fp = popen("ls","w");
char buff[128] = {0};
fgets(buff,sizeof(buff),fp);
puts(buff);

	while(1){

		sleep(3);

		puts( "sigwait blocking..." );
		if( !sigwait( &nSigset, &nSigno ) ){

			printf("catch Signal. [%s]\n", strsignal(nSigno) );

		} else {
			perror( "sigwait()" );
			exit( EXIT_FAILURE );
		}
	}


	exit( EXIT_SUCCESS );
}
