#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>


#define R 0
#define W 1

int main()
{
	int nRtn = 0;
	int nPipeC2P[2];
	int nPipeP2C[2];
	int nRtnFcntl = 0;
	int nStatus = 0;
	char szBuff[65536];
	pid_t nPid;
	pid_t nPidWait;


	nRtn = pipe( nPipeP2C );
	if( nRtn < 0 ){
		perror( "pipe()" );
		exit( EXIT_FAILURE );
	}

	nRtn = pipe( nPipeC2P );
	if( nRtn < 0 ){
		perror( "pipe()" );
		exit( EXIT_FAILURE );
	}

/***
	if(( nRtnFcntl = fcntl( nPipeP2C[0], F_GETFL, NULL ) ) < 0 ){
		perror( "fcntl()" );
		exit( EXIT_FAILURE );
	}

	nRtnFcntl |= O_NONBLOCK;

	if( fcntl( nPipeP2C[0], F_SETFL, nRtnFcntl ) < 0 ){
		perror( "fcntl()" );
		exit( EXIT_FAILURE );
	}

	if(( nRtnFcntl = fcntl( nPipeP2C[1], F_GETFL, NULL ) ) < 0 ){
		perror( "fcntl()" );
		exit( EXIT_FAILURE );
	}

	nRtnFcntl |= O_NONBLOCK;

	if( fcntl( nPipeP2C[1], F_SETFL, nRtnFcntl ) < 0 ){
		perror( "fcntl()" );
		exit( EXIT_FAILURE );
	}

	if(( nRtnFcntl = fcntl( nPipeC2P[0], F_GETFL, NULL ) ) < 0 ){
		perror( "fcntl()" );
		exit( EXIT_FAILURE );
	}

	nRtnFcntl |= O_NONBLOCK;

	if( fcntl( nPipeC2P[0], F_SETFL, nRtnFcntl ) < 0 ){
		perror( "fcntl()" );
		exit( EXIT_FAILURE );
	}

	if(( nRtnFcntl = fcntl( nPipeC2P[1], F_GETFL, NULL ) ) < 0 ){
		perror( "fcntl()" );
		exit( EXIT_FAILURE );
	}

	nRtnFcntl |= O_NONBLOCK;

	if( fcntl( nPipeC2P[1], F_SETFL, nRtnFcntl ) < 0 ){
		perror( "fcntl()" );
		exit( EXIT_FAILURE );
	}
***/




	nPid = fork();
	if( nPid == -1 ){
		/* error */

		perror( "fork()" );
		exit( EXIT_FAILURE );

	} else if( nPid == 0 ){
		/* child */

		close( nPipeC2P[R] );
		close( nPipeP2C[W] );
		
		dup2( nPipeP2C[R], STDIN_FILENO );
		dup2( nPipeC2P[W], STDOUT_FILENO );
		close( nPipeP2C[R] );
		close( nPipeC2P[W] );

sleep(2);
//		nRtn = execlp( "telnet", "telnet", "me-simul", "80", NULL );
		nRtn = execlp( "tr", "tr", "[:upper:]", "[:lower:]", NULL );
//		nRtn = execlp( "echo", "echo", "test", NULL );
//		nRtn = execlp( "./tcom", "./tcom", NULL );
//		nRtn = execlp( "nkf", "nkf", "-MB", NULL );
		if( nRtn < 0 ){
			perror( "execlp()" );
			exit( EXIT_FAILURE );
		}

	} else {
		/* parent */

		close( nPipeC2P[W] );
		close( nPipeP2C[R] );

		nRtn = write( nPipeP2C[W], "GET / HTTP/1.0\n\n", 16 );
		close( nPipeP2C[W] );
		printf("write:[%d]\n",nRtn);

		memset( szBuff, 0x00, sizeof(szBuff) );
		nRtn = read( nPipeC2P[R], szBuff, sizeof(szBuff) );
		printf("read:[%d]\n",nRtn);
		puts( szBuff );
//		close( nPipeC2P[R] );

puts("-----------------------");
		memset( szBuff, 0x00, sizeof(szBuff) );
		nRtn = read( nPipeC2P[R], szBuff, sizeof(szBuff) );
		printf("read:[%d]\n",nRtn);
		puts( szBuff );

puts("-----------------------");
		memset( szBuff, 0x00, sizeof(szBuff) );
		nRtn = read( nPipeC2P[R], szBuff, sizeof(szBuff) );
		printf("read:[%d]\n",nRtn);
		puts( szBuff );

puts("-----------------------");
		memset( szBuff, 0x00, sizeof(szBuff) );
		nRtn = read( nPipeC2P[R], szBuff, sizeof(szBuff) );
		printf("read:[%d]\n",nRtn);
		puts( szBuff );


		/* 子プロセスのPIDを指定して 終了を待つ(ブロックする) */
		nPidWait = waitpid( nPid, &nStatus, 0 );
		if( nPidWait < 0 ){
			perror(" waitpid()" );
			exit( EXIT_FAILURE );
		}

		if( WIFEXITED(nStatus) ){
			/* 子プロセス正常終了 */
	        fprintf( stdout, "child is successful.\n" );

		} else {
			/* 子プロセス異常終了 */
			fprintf( stderr, "child is failure.\n" );
		}


	}

	exit( EXIT_SUCCESS );
}
