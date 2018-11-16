#ifndef _UTILS_H_
#define _UTILS_H_

#include "Defs.h"

class CUtils {
public:
	static int readFile (int fd, uint8_t *pBuff, size_t nSize);
	static void dumper (const uint8_t *pSrc, int nSrcLen, bool isAddAscii=true);
	static void byte2bitString (uint8_t nByte, char *pszDst, size_t nDstSize);

};

#endif
