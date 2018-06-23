#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>


int main (void)
{
	int nFd = open ("readwrite", O_RDWR);
	printf ("nFd=[%d]\n", nFd);
	if (nFd < 0) {
		perror ("open");
		exit (EXIT_FAILURE);
	}


	int nRtn = 0;
	struct timeval stTv;
	char szBuff[128] = {0};
	fd_set stFds;

	while (1) {
		FD_ZERO (&stFds);
		FD_SET (nFd, &stFds);
		stTv.tv_sec = 5;
		stTv.tv_usec = 0;

		nRtn = select (nFd+1, &stFds, NULL, NULL, &stTv);
		if (nRtn < 0) {
			perror ("select()");
			continue;

		} else if (nRtn == 0) {
			// timeout
			puts ("timeout");
			continue;
		}

		if (FD_ISSET(nFd, &stFds)) {
			memset (szBuff, 0x00, sizeof(szBuff));
			read (nFd, szBuff, sizeof(szBuff));
			printf ("%s", szBuff);
			fflush (stdout);
		}

	}

	close (nFd);

	exit (EXIT_SUCCESS);
}
