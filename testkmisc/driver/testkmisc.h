#ifndef _TESTKMISC_H_
#define _TESTKMISC_H_

#include <linux/limits.h>
#include <linux/ioctl.h>


struct ioctl_testkmisc_gup {
	void * buf;
	size_t len;
};



#define IOCTL_TESTKMISC_GUP             _IOW('m', 0, struct ioctl_testkmisc_gup *)
#define IOCTL_TESTKMISC_VMADUMP         _IOW('m', 1, int *)
#define IOCTL_TESTKMISC_FILEREAD        _IOW('m', 2, char *) // path
#define IOCTL_TESTKMISC_PAGECACHE       _IOW('m', 3, char *) // path
#define IOCTL_TESTKMISC_VADDR           _IOW('m', 4, int *)
#define IOCTL_TESTKMISC_KTHREAD_RUN     _IOW('m', 5, int *)

#endif /* _TESTKMISC_H_ */
