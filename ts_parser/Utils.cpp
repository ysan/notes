#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "Utils.h"
#include "Defs.h"


int CUtils::readFile (int fd, uint8_t *pBuff, size_t nSize)
{
	if ((!pBuff) || (nSize == 0)) {
		return -1;
	}

	int nReadSize = 0;
	int nDone = 0;

	while (1) {
		nReadSize = read (fd, pBuff, nSize);
		if (nReadSize < 0) {
			perror ("read()");
			return -1;

		} else if (nReadSize == 0) {
			// file end
			break;

		} else {
			// read done
			pBuff += nReadSize;
			nSize -= nReadSize;
			nDone += nReadSize;

			if (nSize == 0) {
				break;
			}
		}
	}

	return nDone;
}

#define DUMP_PUTS_OFFSET	"  "
void CUtils::dumper (const uint8_t *pSrc, int nSrcLen, bool isAddAscii)
{
	if ((!pSrc) || (nSrcLen <= 0)) {
		return;
	}

	int i = 0;
	int j = 0;
	int k = 0;

	while (nSrcLen >= 16) {

		fprintf (stdout, "%s0x%08x: ", DUMP_PUTS_OFFSET, i);

		// 16進dump
		fprintf (
			stdout,
			"%02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
			*(pSrc+ 0), *(pSrc+ 1), *(pSrc+ 2), *(pSrc+ 3), *(pSrc+ 4), *(pSrc+ 5), *(pSrc+ 6), *(pSrc+ 7),
			*(pSrc+ 8), *(pSrc+ 9), *(pSrc+10), *(pSrc+11), *(pSrc+12), *(pSrc+13), *(pSrc+14), *(pSrc+15)
		);

		// ascii文字表示
		if (isAddAscii) {
			fprintf (stdout, "  |");
			k = 0;
			while (k < 16) {
				// 制御コード系は'.'で代替
				fprintf (
					stdout,
					"%c",
					(*(pSrc+k)>0x1f) && (*(pSrc+k)<0x7f) ? *(pSrc+k) : '.'
				);
				++ k;
			}
		}

		fprintf (stdout, "|\n");

		pSrc += 16;
		i += 16;
		nSrcLen -= 16;
	}

	// 余り分(16byte満たない分)
	if (nSrcLen) {

		// 16進dump
		fprintf (stdout, "%s0x%08x: ", DUMP_PUTS_OFFSET, i);
		while (j < 16) {
			if (j < nSrcLen) {
				fprintf (stdout, "%02x", *(pSrc+j));
				if (j == 7) {
					fprintf (stdout, "  ");
				} else if (j == 15) {

				} else {
					fprintf (stdout, " ");
				}

			} else {
				fprintf (stdout, "  ");
				if (j == 7) {
					fprintf (stdout, "  ");
				} else if (j == 15) {

				} else {
					fprintf (stdout, " ");
				}
			}

			++ j;
		}

		// ascii文字表示
		if (isAddAscii) {
			fprintf (stdout, "  |");
			k = 0;
			while (k < nSrcLen) {
				// 制御コード系は'.'で代替
				fprintf (stdout, "%c", (*(pSrc+k)>0x20) && (*(pSrc+k)<0x7f) ? *(pSrc+k) : '.');
				++ k;
			}
			for (int i = 0; i < (16 - nSrcLen); ++ i) {
				fprintf (stdout, " ");
			}
		}

		fprintf (stdout, "|\n");
	}
}

void CUtils::byte2bitString (uint8_t nByte, char *pszDst, size_t nDstSize)
{



}
