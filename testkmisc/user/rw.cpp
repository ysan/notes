#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <iostream>
#include <string>
#include <cstdio>
#include <vector>
#include <sstream>
#include <fstream>
#include <thread>
#include <chrono>

#include "testkmisc.h"


static char gb[1024*1024];

struct th_args {
	int fd;
	char *buf;
	int sz;
};

static thread_local int thlocalid = 0;
static int gi = 0;

void * thfn (void *args)
{
	struct th_args targs = *(struct th_args*)args;
	for (int i = 0; i < 4; ++ i) {
		struct ioctl_testkmisc_gup itg = {
			.buf = targs.buf,
			.len = (size_t)targs.sz,
		};
		int r = ioctl (targs.fd, IOCTL_TESTKMISC_GUP, &itg);
		if (r < 0) {
			perror ("ioctl");
		}
	}
	return NULL;
}

std::string ltrim (const std::string &s) {
	size_t start = s.find_first_not_of(" \n\r\t\f\v");
	return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim (const std::string &s) {
	size_t end = s.find_last_not_of(" \n\r\t\f\v");
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim (const std::string &s) {
	return rtrim(ltrim(s));
}

std::vector<std::string> split(const std::string& s) {
	std::vector<std::string> r;
	std::stringstream ss(s);
	std::string token;
	while (ss >> token) {
		r.push_back(token);
	}
	return r;
}

bool rw (int fd, std::vector<std::string> &cmdlist) {
	std::string cmd = cmdlist[0];
	if (cmd == "gup_s") {
		// stack
		if (cmdlist.size() != 2) {
			printf("usage: %s size\n", cmd.c_str());
			return false;
		}
		int sz = strtoul(cmdlist[1].c_str(), 0, 0);
		if (sz <= 0) {
			printf("usage: %s size\n", cmd.c_str());
			return false;
		}
		char buf[sz];
		struct ioctl_testkmisc_gup itg = {
			.buf = buf,
			.len = (size_t)sz,
		};
		int r = ioctl (fd, IOCTL_TESTKMISC_GUP, &itg);
		if (r < 0) {
			perror ("ioctl");
			return false;
		}

	} else if (cmd == "gup_g") {
		// global
		if (cmdlist.size() != 2) {
			printf("usage: %s size\n", cmd.c_str());
			return false;
		}
		int sz = strtoul(cmdlist[1].c_str(), 0, 0);
		if (sz <= 0) {
			printf("usage: %s size\n", cmd.c_str());
			return false;
		}
		struct ioctl_testkmisc_gup itg = {
			.buf = gb,
			.len = (size_t)sz,
		};
		int r = ioctl (fd, IOCTL_TESTKMISC_GUP, &itg);
		if (r < 0) {
			perror ("ioctl");
			return false;
		}

	} else if (cmd == "gup_m") {
		// malloc
		if (cmdlist.size() != 2) {
			printf("usage: %s size\n", cmd.c_str());
			return false;
		}
		int sz = strtoul(cmdlist[1].c_str(), 0, 0);
		if (sz <= 0) {
			printf("usage: %s size\n", cmd.c_str());
			return false;
		}
		char *buf = (char*)malloc(sz);
		if (!buf) {
			perror ("malloc");
			return false;
		}
//		memset(buf, 0, sz);
		struct ioctl_testkmisc_gup itg = {
			.buf = buf,
			.len = (size_t)sz,
		};
		int r = ioctl (fd, IOCTL_TESTKMISC_GUP, &itg);
		if (r < 0) {
			perror ("ioctl");
			free(buf);
			return false;
		}
		free(buf);

	} else if (cmd == "gup_pm") {
		// posix_memalign
		if (cmdlist.size() != 2) {
			printf("usage: %s size\n", cmd.c_str());
			return false;
		}
		int sz = strtoul(cmdlist[1].c_str(), 0, 0);
		if (sz <= 0) {
			printf("usage: %s size\n", cmd.c_str());
			return false;
		}
		char *buf = NULL;
		posix_memalign((void **)&buf, 4096, sz + 4096);
		if (!buf) {
			perror ("posix_memalign");
			return false;
		}
		struct ioctl_testkmisc_gup itg = {
			.buf = buf,
			.len = (size_t)sz,
		};
		int r = ioctl (fd, IOCTL_TESTKMISC_GUP, &itg);
		if (r < 0) {
			perror ("ioctl");
			free(buf);
			return false;
		}
		free(buf);

	} else if (cmd == "gup_th") {
		// malloc
		if (cmdlist.size() != 2) {
			printf("usage: %s size\n", cmd.c_str());
			return false;
		}
		int sz = strtoul(cmdlist[1].c_str(), 0, 0);
		if (sz <= 0) {
			printf("usage: %s size\n", cmd.c_str());
			return false;
		}
		char *buf = (char*)malloc(sz);
		if (!buf) {
			perror ("malloc");
			return false;
		}
//		memset(buf, 0, sz);
		pthread_t th0;
		pthread_t th1;
		struct th_args targs = {fd, buf, sz};
		pthread_create(&th0, NULL, thfn, &targs);
		pthread_create(&th1, NULL, thfn, &targs);
		pthread_join(th0, NULL);
		pthread_join(th1, NULL);
		free(buf);

	} else if (cmd == "vd") {
		int lv = 0;
		if (cmdlist.size() > 2) {
			printf("usage: %s [v]\n", cmd.c_str());
			return false;
		}
		if (cmdlist.size() == 2) {
			if (cmdlist[1] == "v") {
				lv = 1;
			} else {
				printf("usage: %s [v]\n", cmd.c_str());
				return false;
			}
		}
		int r = ioctl (fd, IOCTL_TESTKMISC_VMADUMP, &lv);
		if (r < 0) {
			perror ("ioctl");
			return false;
		}

	} else if (cmd == "fr") {
		if (cmdlist.size() != 2) {
			printf("usage: %s path\n", cmd.c_str());
			return false;
		}
		const char *path = cmdlist[1].c_str();
		int r = ioctl (fd, IOCTL_TESTKMISC_FILEREAD, path);
		if (r < 0) {
			perror ("ioctl");
			return false;
		}

	} else if (cmd == "pg") {
		if (cmdlist.size() != 2) {
			printf("usage: %s path\n", cmd.c_str());
			return false;
		}
		const char *path = cmdlist[1].c_str();
		int r = ioctl (fd, IOCTL_TESTKMISC_PAGECACHE, path);
		if (r < 0) {
			perror ("ioctl");
			return false;
		}

	} else if (cmd == "va") {
		if (cmdlist.size() != 2) {
			printf("usage: %s size\n", cmd.c_str());
			return false;
		}
		int sz = strtoul(cmdlist[1].c_str(), 0, 0);
		if (sz <= 0) {
			printf("usage: %s size\n", cmd.c_str());
			return false;
		}
		int r = ioctl (fd, IOCTL_TESTKMISC_VADDR, &sz);
		if (r < 0) {
			perror ("ioctl");
			return false;
		}

	} else if (cmd == "kthrun") {
		if (cmdlist.size() != 1) {
			printf("usage: %s\n", cmd.c_str());
			return false;
		}
		auto start = std::chrono::high_resolution_clock::now();
		int r = ioctl (fd, IOCTL_TESTKMISC_KTHREAD_RUN, 0);
		auto end = std::chrono::high_resolution_clock::now();
		if (r < 0) {
			perror ("ioctl");
			return false;
		}

		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
		printf("elapsed time %lu nsec\n", duration.count());

	} else if (cmd == "help") {
			printf("commands:\n");
			printf("  gup_s\n");
			printf("  gup_g\n");
			printf("  gup_m\n");
			printf("  gup_pm\n");
			printf("  gup_th\n");
			printf("  vd\n");
			printf("  fr\n");
			printf("  pg\n");
			printf("  kthrun\n");
	} else {
		return false;
	}

	return true;
}

int main (int argc, char **argv)
{
	if (argc != 2) {
		fprintf (stderr, "\nusage: %s <cdev>\n\n", argv[0]);
		exit(1);
	}

	char *cdev = strdup(argv[1]);
//	off_t pagesize = sysconf(_SC_PAGESIZE);
	int fd = 0;
	if ((fd = open(cdev, O_RDWR)) == -1) {
		perror("open");
		exit(1);
	}

	printf("thlocalid %d %p\n", thlocalid, &thlocalid);
	printf("gi %d %p\n", gi, &gi);

	std::string in;
	while (true) {
		std::cout << "> ";
		std::getline(std::cin, in);
		std::string t = trim(in);
		if (t.empty()) {
			continue;
		}
		auto cmd = split(t);
		if (!rw(fd, cmd)) {
			std::cout << "invalid command..." << std::endl;
		}
	}

	close(fd);

	return 0;
}
