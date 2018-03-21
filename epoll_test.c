#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <fcntl.h>


#define FD_SIZE 100
#define MAX_EVENTS 10

int main (void)
{
	int epfd = epoll_create (FD_SIZE);
	struct epoll_event ev;

	memset (&ev, 0x00, sizeof(ev));
	ev.events = EPOLLIN; // 入力を待つ
	ev.data.fd = STDIN_FILENO;
	epoll_ctl (epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev); //epollインスタンスにfdを追加し、イベントをfdに関連付ける。


//	int val;
//	if ((val = fcntl(STDIN_FILENO, F_GETFL, 0)) < 0) {
//		perror ("fcntl F_GETFL");
//		exit (EXIT_FAILURE);
//	}
//
//	val |= O_NONBLOCK;  /* turn on flags */
//
//	if (fcntl(STDIN_FILENO, F_SETFL, val) < 0) {
//		perror ("fcntl F_SETFL");
//		exit (EXIT_FAILURE);
//	}

	int i = 0;
	int nfd = 0;
	char buf[128];
	struct epoll_event events [FD_SIZE];

	while (1) {
//		memset (buf, 0x00, sizeof(buf));
//		read (STDIN_FILENO, buf, sizeof(buf));

		puts ("epoll_wait");
		memset (events, 0x00, sizeof(events));
		nfd = epoll_wait (epfd, events, MAX_EVENTS, 5000); // epoll_waitは準備ができているファイルディスクリプタの数を返す
		if (nfd == 0) {
			puts ("timeout");
			continue;
		}

		for (i = 0; i < nfd; i++) {
			if (events[i].data.fd == STDIN_FILENO) {
				memset (buf, 0x00, sizeof(buf));
				read (STDIN_FILENO, buf, sizeof(buf));
				printf ("[%s]", buf);
			}
		}
	}


	exit (EXIT_SUCCESS);
}
