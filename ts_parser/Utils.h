#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "Defs.h"

class CUtils {
public:
	static int readFile (int fd, uint8_t *pBuff, size_t nSize);
	static void dumper (const uint8_t *pSrc, int nSrcLen, bool isAddAscii=true);
	static void byte2bitString (uint8_t nByte, char *pszDst, size_t nDstSize);
	static char * getStrEpoch (time_t tx, const char *format);
	static char * getStrSecond (int sec);


	// ARIB specific
	static time_t getEpochFromMJD (const uint8_t *mjd);
	static int getSecFromBCD (const uint8_t *bcd);

};

#endif
