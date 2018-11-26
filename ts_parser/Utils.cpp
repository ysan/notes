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

void CUtils::getStrEpoch (time_t tx, const char *format, char *pszout, int outsize)
{
	struct tm *tl;
	struct tm stm;
	tl = localtime_r (&tx, &stm);
	strftime(pszout, outsize - 1, format, tl);
}

void CUtils::getStrSecond (int second, char *pszout, int outsize)
{
	int hh = second / 3600;
	int mm = (second % 3600) / 60;
	int ss = (second % 3600) % 60;
	snprintf (pszout, outsize, "%02d:%02d:%02d", hh, mm, ss);
}

time_t CUtils::getEpochFromMJD (const uint8_t *mjd)
{
	if (!mjd) {
		return 0;
	}

	int tnum,yy,mm,dd;
	char buf[10];
	time_t l_time ;
	time_t end_time ;
	struct tm tl ;
	struct tm *endtl ;
	char cendtime[32];
	char cmjd[32];

	tnum = (mjd[0] & 0xFF) << 8 | (mjd[1] & 0xFF);

	yy = (tnum - 15078.2) / 365.25;
	mm = ((tnum - 14956.1) - (int)(yy * 365.25)) / 30.6001;
	dd = (tnum - 14956) - (int)(yy * 365.25) - (int)(mm * 30.6001);

	if(mm == 14 || mm == 15) {
		yy += 1;
		mm = mm - 1 - (1 * 12);
	} else {
		mm = mm - 1;
	}

	tl.tm_year = yy;
	tl.tm_mon = mm - 1;
	tl.tm_mday = dd;
	memset(buf, '\0', sizeof(buf));
	sprintf(buf, "%x", mjd[2]);
	tl.tm_hour = atoi(buf);
	memset(buf, '\0', sizeof(buf));
	sprintf(buf, "%x", mjd[3]);
	tl.tm_min = atoi(buf);
	memset(buf, '\0', sizeof(buf));
	sprintf(buf, "%x", mjd[4]);
	tl.tm_sec = atoi(buf);

	tl.tm_wday = 0;
	tl.tm_isdst = 0;
	tl.tm_yday = 0;

	l_time = mktime(&tl);
	return l_time;
}

int CUtils::getSecFromBCD (const uint8_t *bcd)
{
	if (!bcd) {
		return -1;
	}

	int hh,mm,ss;
	char buf[24];

	if((bcd[0] == 0xFF) && (bcd[1] == 0xFF) && (bcd[2] == 0xFF)){
		// 終了未定
		hh = mm = ss = 0;
		ss = -1;
	}else{
		memset(buf, '\0', sizeof(buf));
		sprintf(buf, "%x", bcd[0]);
		hh = atoi(buf)*3600;
		memset(buf, '\0', sizeof(buf));
		sprintf(buf, "%x", bcd[1]);
		mm = atoi(buf)*60;
		memset(buf, '\0', sizeof(buf));
		sprintf(buf, "%x", bcd[2]);
		ss = atoi(buf);
	}

	return hh+mm+ss;
}

