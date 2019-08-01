#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>



int main (void)
{
	int r = 0;
	int fdSockSv = 0;
	int fdSockCl = 0;
	char szBuff [128];
	uint16_t port = 0;
	struct sockaddr_in stAddrSv;
	struct sockaddr_in stAddrCl;
	socklen_t addrLenCl = sizeof(struct sockaddr_in);

	memset (szBuff, 0x00, sizeof(szBuff));
	memset (&stAddrSv, 0x00, sizeof(struct sockaddr_in));
	memset (&stAddrCl, 0x00, sizeof(struct sockaddr_in));

	port = 50000;
	stAddrSv.sin_family = AF_INET;
	stAddrSv.sin_addr.s_addr = htonl (INADDR_ANY);
	stAddrSv.sin_port = htons (port);


	if ((fdSockSv = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror ("socket()");
		return -1;
	}

	int optval = 1;
	r = setsockopt (fdSockSv, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (r < 0) {
		perror ("setsockopt()");
		close (fdSockSv);
		return -1;
	}

	if (bind(fdSockSv, (struct sockaddr*)&stAddrSv, sizeof(struct sockaddr_in)) < 0) {
		perror ("bind()");
		close (fdSockSv);
		return -1;
	}

	if (listen (fdSockSv, SOMAXCONN) < 0) {
		perror ("listen()");
		close (fdSockSv);
		return -1;
	}


	while (1) {
		printf ("accept blocking...\n");

		if ((fdSockCl = accept (fdSockSv, (struct sockaddr*)&stAddrCl, &addrLenCl)) < 0 ) {
			perror ("accept()");
			close (fdSockSv);
			return -1;
		}

		printf (
			"clientAddr:[%s] SocketFd:[%d] --- connected.\n",
			inet_ntoa(stAddrCl.sin_addr),
			fdSockCl
		);

//		usleep (100000);

		close (fdSockCl);
	}


	exit (EXIT_SUCCESS);
}
