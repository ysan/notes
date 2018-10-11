#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>


/*
 * constant
 */
#define DEVICE_NAME		"/dev/hidraw1"

/*
 * Ugly hack to work around failing compilation on systems that don't
 * yet populate new version of hidraw.h to userspace.
 */
#ifndef HIDIOCSFEATURE
#warning Please have your distro update the userspace kernel headers
#define HIDIOCSFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x06, len)
#define HIDIOCGFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x07, len)
#endif


/*
 * values
 */
unsigned char gszCommand[4];

const char *getBusType (int nType)
{
	switch (nType) {
	case BUS_USB:
		return "USB";
		break;
	case BUS_HIL:
		return "HIL";
		break;
	case BUS_BLUETOOTH:
		return "Bluetooth";
		break;
	case BUS_VIRTUAL:
		return "Virtual";
		break;
	default:
		return "Other";
		break;
	}
}

/*
 * open hid
 *
 * reference:
 *   kernel/samples/hidraw/hid-example.c
 */
int openDevice (char *pszDev)
{
	if ((!pszDev) || (strlen(pszDev) == 0)) {
		return -1;
	}

	int i = 0;
	int nFd = 0;
	int nRtn = 0;
	int nDescSize = 0;
	char szBuff[ 256 ];
	struct hidraw_report_descriptor stReportDesc;
	struct hidraw_devinfo stDevInfo;

	memset (&stReportDesc, 0x00, sizeof(stReportDesc));
	memset (&stDevInfo, 0x00, sizeof(stDevInfo));
	memset (szBuff, 0x00, sizeof(szBuff));

	/* device open */
	nFd = open (pszDev, O_RDWR);
	if (nFd < 0) {
		perror ("open()");
		return -1;
	}

	/* Get Report Descriptor Size */
	nRtn = ioctl (nFd, HIDIOCGRDESCSIZE, &nDescSize);
	if (nRtn < 0) {
		perror ("ioctl() HIDIOCGRDESCSIZE");
	}
	printf("Report Descriptor Size: %d\n", nDescSize);

	/* Get Report Descriptor */
	stReportDesc.size = nDescSize;
	nRtn = ioctl (nFd, HIDIOCGRDESC, &stReportDesc);
	if (nRtn < 0) {
		perror ("ioctl() HIDIOCGRDESC");
	}
	printf("Report Descriptor:\n");
	for (i = 0; i < stReportDesc.size; i ++) {
		printf ("%x ", stReportDesc.value[i]);
	}
	printf("\n\n");

	/* Get Raw Name */
	nRtn = ioctl (nFd, HIDIOCGRAWNAME(256), szBuff);
	if (nRtn < 0) {
		perror ("ioctl() HIDIOCGRAWNAME");
	}
	printf ("Raw Name: %s\n", szBuff);

	/* Get Physical Location */
	nRtn = ioctl (nFd, HIDIOCGRAWPHYS(256), szBuff);
	if (nRtn < 0) {
		perror ("ioctl() HIDIOCGRAWPHYS");
	}
	printf ("Raw Phys: %s\n", szBuff);

	/* Get Raw Info */
	nRtn = ioctl (nFd, HIDIOCGRAWINFO, &stDevInfo);
	if (nRtn < 0) {
		perror ("ioctl() HIDIOCGRAWINFO");
	}
	printf ("Raw Info:\n");
	printf ("  bustype: %d (%s)\n", stDevInfo.bustype, getBusType(stDevInfo.bustype));
	printf ("  vendor: 0x%04x\n", stDevInfo.vendor);
	printf ("  product: 0x%04x\n", stDevInfo.product);

#if 0
	/* Set Feature */
	szBuff[0] = 0x9; /* Report Number */
	szBuff[1] = 0xff;
	szBuff[2] = 0xff;
	szBuff[3] = 0xff;
	nRtn = ioctl (nFd, HIDIOCSFEATURE(4), szBuff);
	if (nRtn < 0) {
		perror ("ioctl() HIDIOCSFEATURE");
	}
	printf ("ioctl() HIDIOCGFEATURE returned: %d\n", nRtn);

	/* Get Feature */
	szBuff[0] = 0x9; /* Report Number */
	nRtn = ioctl (nFd, HIDIOCGFEATURE(256), szBuff);
	if (nRtn < 0) {
		perror ("ioctl() HIDIOCGFEATURE");
	}
	printf ("ioctl() HIDIOCGFEATURE returned: %d\n", nRtn);
	printf ("Report data (not containing the report number):\n");
	printf (" "); 
	for (i = 0; i < nRtn; i ++) {
		printf ("%x ", szBuff[i]);
	}
	printf ("\n\n");
#endif

	return nFd;
}

void closeDevice (int nFd)
{
	close (nFd);
}

/*
 * 4byteづつ送らないとwriteエラーになる (hid)
 */
void sendToHid (int nFd, unsigned char cData)
{
	/* usb io (AKI) コマンド */
	gszCommand[0] = 0x00;
	gszCommand[1] = 0x20;
	gszCommand[2] = 0x01; // io port1
	gszCommand[3] = cData;

	if (write (nFd, gszCommand, sizeof(gszCommand)) < 0) {
		perror ("write()");
	}
}

void sendStream (int nFd, unsigned char *pszBuff, int nLen)
{
	if (!pszBuff || (nLen <= 0)) {
		return;
	}

	while (nLen > 0) {
		sendToHid (nFd, *pszBuff);
		pszBuff ++;
		nLen --;
	}

}

int main (void)
{
	int nFd = 0;

	nFd = openDevice ("DEVICE_NAME");
	if (nFd < 0) {
		goto END;
	}



END:
	closeDevice (nFd);

	exit (EXIT_SUCCESS);
}
