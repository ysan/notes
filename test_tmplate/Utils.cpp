#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>

#include "Utils.h"


#define SYSTIME_STRING_SIZE			(2+1+2+1+2+1+2+1+2 +1)
#define THREAD_NAME_STRING_SIZE		(15+1)
#define LOG_STRING_SIZE				(128)
#define BACKTRACE_BUFF_SIZE			(20)


#ifdef _ANDROID_BUILD
#include <stdint.h>
#include <unwind.h>
#include <cxxabi.h>

struct UnwindData {
	void **array;
	int current;
	int size;
};

static _Unwind_Reason_Code unwind_callback (struct _Unwind_Context* context, void* _data)
{
	UnwindData *data = static_cast<UnwindData*> (_data);
	uintptr_t pc = _Unwind_GetIP (context);
	if (pc) {
		if (data->current < data->size) {
			data->array[data->current] = (void *)pc;
			++ data->current;
		} else {
			return _URC_END_OF_STACK;
		}
	}

	return _URC_NO_REASON;
}

int bionic_backtrace (void **array, int size)
{
	UnwindData data = {array, 0, size};
	_Unwind_Backtrace (unwind_callback, &data);

	return data.current;
}

char **bionic_backtrace_symbols (void *const *array, int size)
{
	const int line_size = 256;
	void *buff = malloc((sizeof(void *) * size) + (line_size * size));
	char **line_addr = (char **)buff;
	char *ptr = (char *)buff + sizeof(void *) * size;

	for (int i = 0; i < size; ++ i) {
		line_addr[i] = ptr;
		const void *addr = array[i];
		const char *symbol = "";
		const char *fname  = "";
		unsigned int offset = 0;

		Dl_info info;
		if (dladdr(addr, &info) && info.dli_sname) {
			int status = 0;
			symbol = info.dli_sname;
			char *demangled = __cxxabiv1::__cxa_demangle (symbol, 0, 0, &status);
			if (demangled) {
				symbol = demangled;
			}
			fname = info.dli_fname;
			offset = (unsigned int)addr;
			offset -= (unsigned int)info.dli_saddr;
			snprintf (line_addr[i], line_size, "[0x%08X] %s <%s +0x%X>", (unsigned int)addr, fname, symbol, (unsigned int)offset);
			if (demangled) {
				free (demangled);
			}
		} else {
			snprintf (line_addr[i], line_size, "[0x%08X]", (unsigned int)addr);
		}
		ptr += line_size;
	}

	return line_addr;
}

#endif


/**
 * システム現在時刻を取得
 * MM/dd HH:mm:ss形式
 */
void CUtils::getSysTime (char *pszOut, size_t nSize)
{
	time_t timer;
	struct tm *pstTmLocal = NULL;
	struct tm stTmLocalTmp;

	timer = time (NULL);
	pstTmLocal = localtime_r (&timer, &stTmLocalTmp); /* スレッドセーフ */

	snprintf (
		pszOut,
		nSize,
		"%02d/%02d %02d:%02d:%02d",
		pstTmLocal->tm_mon+1,
		pstTmLocal->tm_mday,
		pstTmLocal->tm_hour,
		pstTmLocal->tm_min,
		pstTmLocal->tm_sec
	);
}

/**
 * setThreadName
 * thread名をセット  pthread_setname_np
 */
void CUtils::setThreadName (char *p)
{
	if (!p) {
		return;
	}

	char szName [THREAD_NAME_STRING_SIZE];
	memset (szName, 0x00, sizeof(szName));
	strncpy (szName, p, sizeof(szName) -1);
	pthread_setname_np (pthread_self(), szName);
}

/**
 * pthread名称を取得
 */
void CUtils::getThreadName (char *pszOut, size_t nSize)
{
	char szName[16];
	memset (szName, 0x00, sizeof(szName));
//	pthread_getname_np (pthread_self(), szName, sizeof(szName));
	prctl(PR_GET_NAME, szName);

	strncpy (pszOut, szName, nSize -1);
	int len = strlen(pszOut);
	if (len < THREAD_NAME_STRING_SIZE -1) {
		int i = 0;
		for (i = 0; i < (THREAD_NAME_STRING_SIZE -1 -len); ++ i) {
			*(pszOut + len + i) = ' ';
		}
	}
}

/**
 * getTimeOfDay wrapper
 * gettimeofdayはスレッドセーフ
 */
void CUtils::getTimeOfDay (struct timeval *p)
{
	if (!p) {
		return ;
	}

	gettimeofday (p, NULL);
}

/**
 * putsLog
 * ログ出力 本体
 */
void CUtils::putsLog (
	FILE *pFp,
	EN_LOG_LEVEL enLogLevel,
	const char *pszFile,
	const char *pszFunc,
	int nLine,
	const char *pszFormat,
	va_list va
)
{
	char szBufVa [LOG_STRING_SIZE];
	char szTime [SYSTIME_STRING_SIZE];
    char szThreadName [THREAD_NAME_STRING_SIZE];
	char type;
	char szPerror[32];
#ifndef _ANDROID_BUILD
	char *pszPerror = NULL;
#endif

	memset (szBufVa, 0x00, sizeof (szBufVa));
	memset (szTime, 0x00, sizeof (szTime));
    memset (szThreadName, 0x00, sizeof (szThreadName));
	memset (szPerror, 0x00, sizeof (szPerror));

	switch (enLogLevel) {
	case EN_LOG_LEVEL_I:
		type = 'I';
		break;

	case EN_LOG_LEVEL_N:
		type = 'N';
		fprintf (stdout, _UTL_TEXT_CYAN);
		break;

	case EN_LOG_LEVEL_W:
		type = 'W';
		fprintf (stdout, _UTL_TEXT_BOLD_TYPE);
		fprintf (stdout, _UTL_TEXT_YELLOW);
		break;

	case EN_LOG_LEVEL_E:
		type = 'E';
		fprintf (stdout, _UTL_TEXT_UNDER_LINE);
		fprintf (stdout, _UTL_TEXT_BOLD_TYPE);
		fprintf (stdout, _UTL_TEXT_RED);
		break;

	case EN_LOG_LEVEL_PE:
		type = 'E';
		fprintf (stdout, _UTL_TEXT_REVERSE);
		fprintf (stdout, _UTL_TEXT_BOLD_TYPE);
		fprintf (stdout, _UTL_TEXT_MAGENTA);
#ifndef _ANDROID_BUILD
		pszPerror = strerror_r(errno, szPerror, sizeof (szPerror));
#else
		strerror_r(errno, szPerror, sizeof (szPerror));
#endif
		break;

	default:
		type = 'I';
		break;
	}

	vsnprintf (szBufVa, sizeof(szBufVa), pszFormat, va);

	getSysTime (szTime, SYSTIME_STRING_SIZE);
	getThreadName (szThreadName, THREAD_NAME_STRING_SIZE);

	deleteLF (szBufVa);

	switch (enLogLevel) {
	case EN_LOG_LEVEL_PE:
		fprintf (
			pFp,
			"[%s] %c %s  %s: %s   src=[%s %s()] line=[%d]\n",
			szThreadName,
			type,
			szTime,
			szBufVa,
#ifndef _ANDROID_BUILD
			pszPerror,
#else
			szPerror,
#endif
			pszFile,
			pszFunc,
			nLine
		);
		break;

	case EN_LOG_LEVEL_I:
	case EN_LOG_LEVEL_N:
	case EN_LOG_LEVEL_W:
	case EN_LOG_LEVEL_E:
	default:
		fprintf (
			pFp,
			"[%s] %c %s  %s   src=[%s %s()] line=[%d]\n",
			szThreadName,
			type,
			szTime,
			szBufVa,
			pszFile,
			pszFunc,
			nLine
		);
		break;
	}

	fflush (pFp);

	fprintf (stdout, _UTL_TEXT_ATTR_RESET);
	fflush (pFp);
}

/**
 * putsLog
 * ログ出力 loglevel判定なし
 */
void CUtils::putsLog (
	FILE *pFp,
	EN_LOG_LEVEL enLogLevel,
	const char *pszFile,
	const char *pszFunc,
	int nLine,
	const char *pszFormat,
	...
)
{
	va_list va;
	va_start (va, pszFormat);
	putsLog (pFp, enLogLevel, pszFile, pszFunc, nLine, pszFormat, va);
	va_end (va);
}

/**
 * putsLog
 * ログ出力 loglevel判定あり
 */
void CUtils::putsLog (
	FILE *pFp,
	EN_LOG_LEVEL enCurLogLevel,
	EN_LOG_LEVEL enLogLevel,
	const char *pszFile,
	const char *pszFunc,
	int nLine,
	const char *pszFormat,
	...
)
{
	if (enCurLogLevel > enLogLevel) {
		return ;
	}

	va_list va;
	va_start (va, pszFormat);
	putsLog (pFp, enLogLevel, pszFile, pszFunc, nLine, pszFormat, va);
	va_end (va);
}

/**
 * putsLW
 * ログ出力 src,lineなし 本体
 */
void CUtils::putsLogLW (
	FILE *pFp,
	EN_LOG_LEVEL enLogLevel,
	const char *pszFormat,
	va_list va
)
{
	char szBufVa [LOG_STRING_SIZE];
	char szTime [SYSTIME_STRING_SIZE];
    char szThreadName [THREAD_NAME_STRING_SIZE];
	char type;
	char szPerror[32];
#ifndef _ANDROID_BUILD
	char *pszPerror = NULL;
#endif

	memset (szBufVa, 0x00, sizeof (szBufVa));
	memset (szTime, 0x00, sizeof (szTime));
    memset (szThreadName, 0x00, sizeof (szThreadName));
	memset (szPerror, 0x00, sizeof (szPerror));

	switch (enLogLevel) {
	case EN_LOG_LEVEL_I:
		type = 'I';
		break;

	case EN_LOG_LEVEL_N:
		type = 'N';
		fprintf (stdout, _UTL_TEXT_CYAN);
		break;

	case EN_LOG_LEVEL_W:
		type = 'W';
		fprintf (stdout, _UTL_TEXT_BOLD_TYPE);
		fprintf (stdout, _UTL_TEXT_YELLOW);
		break;

	case EN_LOG_LEVEL_E:
		type = 'E';
		fprintf (stdout, _UTL_TEXT_UNDER_LINE);
		fprintf (stdout, _UTL_TEXT_BOLD_TYPE);
		fprintf (stdout, _UTL_TEXT_RED);
		break;

	case EN_LOG_LEVEL_PE:
		type = 'E';
		fprintf (stdout, _UTL_TEXT_REVERSE);
		fprintf (stdout, _UTL_TEXT_BOLD_TYPE);
		fprintf (stdout, _UTL_TEXT_MAGENTA);
#ifndef _ANDROID_BUILD
		pszPerror = strerror_r(errno, szPerror, sizeof (szPerror));
#else
		strerror_r(errno, szPerror, sizeof (szPerror));
#endif
		break;

	default:
		type = 'I';
		break;
	}

	vsnprintf (szBufVa, sizeof(szBufVa), pszFormat, va);

	getSysTime (szTime, SYSTIME_STRING_SIZE);
	getThreadName (szThreadName, THREAD_NAME_STRING_SIZE);

	deleteLF (szBufVa);

	switch (enLogLevel) {
	case EN_LOG_LEVEL_PE:
		fprintf (
			pFp,
			"[%s] %c %s  %s: %s\n",
			szThreadName,
			type,
			szTime,
			szBufVa,
#ifndef _ANDROID_BUILD
			pszPerror
#else
			szPerror
#endif
		);
		break;

	case EN_LOG_LEVEL_I:
	case EN_LOG_LEVEL_N:
	case EN_LOG_LEVEL_W:
	case EN_LOG_LEVEL_E:
	default:
		fprintf (
			pFp,
			"[%s] %c %s  %s\n",
			szThreadName,
			type,
			szTime,
			szBufVa
		);
		break;
	}

	fflush (pFp);

	fprintf (stdout, _UTL_TEXT_ATTR_RESET);
	fflush (pFp);
}

/**
 * putsLog
 * ログ出力 src,lineなし loglevel判定なし
 */
void CUtils::putsLogLW (
	FILE *pFp,
	EN_LOG_LEVEL enLogLevel,
	const char *pszFormat,
	...
)
{
	va_list va;
	va_start (va, pszFormat);
	putsLogLW (pFp, enLogLevel, pszFormat, va);
	va_end (va);
}

/**
 * putsLog
 * ログ出力 src,lineなし loglevel判定あり
 */
void CUtils::putsLogLW (
	FILE *pFp,
	EN_LOG_LEVEL enCurLogLevel,
	EN_LOG_LEVEL enLogLevel,
	const char *pszFormat,
	...
)
{
	if (enCurLogLevel > enLogLevel) {
		return ;
	}

	va_list va;
	va_start (va, pszFormat);
	putsLogLW (pFp, enLogLevel, pszFormat, va);
	va_end (va);
}

/**
 * deleteLF
 * 改行コードLFを削除  (文字列末尾についてるものだけです)
 * CRLFの場合 CRも削除する
 */
void CUtils::deleteLF (char *p)
{
	if ((!p) || ((int)strlen(p) == 0)) {
		return;
	}

	int len = (int)strlen (p);

	while (len > 0) {
		if ((*(p + len -1) == '\n') || (*(p + len -1) == '\r')) {
			*(p + len -1) = '\0';
		} else {
			break;
		}

		-- len;
	}
}

/**
 * putsBackTrace
 * libc とbionicはコンパイルスイッチで切り替え
 */
void CUtils::putsBackTrace (void)
{
	int i;
	int n;
	void *pBuff [BACKTRACE_BUFF_SIZE];
	char **pRtn;

#ifdef _ANDROID_BUILD
	n = bionic_backtrace (pBuff, BACKTRACE_BUFF_SIZE);
	pRtn = bionic_backtrace_symbols (pBuff, n);
#else
	if (!backtrace) {
		_UTL_LOG_W ("============================================================\n");
		_UTL_LOG_W ("----- pid=%d tid=%ld ----- backtrace symbol not found\n", getpid(), syscall(SYS_gettid));
		_UTL_LOG_W ("============================================================\n");
		return ;
	}

	// libc
	n = backtrace (pBuff, BACKTRACE_BUFF_SIZE);
	pRtn = backtrace_symbols (pBuff, n);
#endif
	if (!pRtn) {
		_UTL_PERROR ("backtrace_symbols()");
		return;
	}

	_UTL_LOG_W ("============================================================\n");
	_UTL_LOG_W ("----- pid=%d tid=%ld -----\n", getpid(), syscall(SYS_gettid));
	for (i = 0; i < n; ++ i) {
		_UTL_LOG_W ("%s\n", pRtn[i]);
	}
	_UTL_LOG_W ("============================================================\n");
	free (pRtn);
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

	char szWk [128];
	char* pszWk = &szWk[0];
	memset (szWk, 0x00, sizeof(szWk));
	int rtn = 0;
	int size = sizeof(szWk);


	while (nSrcLen >= 16) {
		// clear for snprintf
		pszWk = &szWk[0];
		memset (szWk, 0x00, sizeof(szWk));
		size = sizeof(szWk);


//		fprintf (stdout, "%s0x%08x: ", DUMP_PUTS_OFFSET, i);
		rtn = snprintf (pszWk, size, "%s0x%08x: ", DUMP_PUTS_OFFSET, i);
		pszWk += rtn;
		size -= rtn;

		// 16進dump
//		fprintf (
//			stdout,
//			"%02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
//			*(pSrc+ 0), *(pSrc+ 1), *(pSrc+ 2), *(pSrc+ 3), *(pSrc+ 4), *(pSrc+ 5), *(pSrc+ 6), *(pSrc+ 7),
//			*(pSrc+ 8), *(pSrc+ 9), *(pSrc+10), *(pSrc+11), *(pSrc+12), *(pSrc+13), *(pSrc+14), *(pSrc+15)
//		);
		rtn = snprintf (
			pszWk,
			size,
			"%02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
			*(pSrc+ 0), *(pSrc+ 1), *(pSrc+ 2), *(pSrc+ 3), *(pSrc+ 4), *(pSrc+ 5), *(pSrc+ 6), *(pSrc+ 7),
			*(pSrc+ 8), *(pSrc+ 9), *(pSrc+10), *(pSrc+11), *(pSrc+12), *(pSrc+13), *(pSrc+14), *(pSrc+15)
		);
		pszWk += rtn;
		size -= rtn;

		// ascii文字表示
		if (isAddAscii) {
//			fprintf (stdout, "  |");
			rtn = snprintf (pszWk, size, "  |");
			pszWk += rtn;
			size -= rtn;

			k = 0;
			while (k < 16) {
				// 制御コード系は'.'で代替
//				fprintf (
//					stdout,
//					"%c",
//					(*(pSrc+k)>0x1f) && (*(pSrc+k)<0x7f) ? *(pSrc+k) : '.'
//				);
				rtn = snprintf (
					pszWk,
					size,
					"%c",
					(*(pSrc+k)>0x1f) && (*(pSrc+k)<0x7f) ? *(pSrc+k) : '.'
				);
				pszWk += rtn;
				size -= rtn;

				++ k;
			}
		}

//		fprintf (stdout, "|\n");
		rtn = snprintf (pszWk, size, "|\n");
		pszWk += rtn;
		size -= rtn;

		_UTL_LOG_I ("%s", szWk);

		pSrc += 16;
		i += 16;
		nSrcLen -= 16;
	}

	// clear for snprintf
	pszWk = &szWk[0];
	memset (szWk, 0x00, sizeof(szWk));
	size = sizeof(szWk);

	// 余り分(16byte満たない分)
	if (nSrcLen) {

		// 16進dump
//		fprintf (stdout, "%s0x%08x: ", DUMP_PUTS_OFFSET, i);
		rtn = snprintf (pszWk, size, "%s0x%08x: ", DUMP_PUTS_OFFSET, i);
		pszWk += rtn;
		size -= rtn;

		while (j < 16) {
			if (j < nSrcLen) {
//				fprintf (stdout, "%02x", *(pSrc+j));
				rtn = snprintf (pszWk, size, "%02x", *(pSrc+j));
				pszWk += rtn;
				size -= rtn;

				if (j == 7) {
//					fprintf (stdout, "  ");
					rtn = snprintf (pszWk, size, "  ");
					pszWk += rtn;
					size -= rtn;
				} else if (j == 15) {

				} else {
//					fprintf (stdout, " ");
					rtn = snprintf (pszWk, size, " ");
					pszWk += rtn;
					size -= rtn;
				}

			} else {
//				fprintf (stdout, "  ");
				rtn = snprintf (pszWk, size, "  ");
				pszWk += rtn;
				size -= rtn;

				if (j == 7) {
//					fprintf (stdout, "  ");
					rtn = snprintf (pszWk, size, "  ");
					pszWk += rtn;
					size -= rtn;
				} else if (j == 15) {

				} else {
//					fprintf (stdout, " ");
					rtn = snprintf (pszWk, size, " ");
					pszWk += rtn;
					size -= rtn;
				}
			}

			++ j;
		}

		// ascii文字表示
		if (isAddAscii) {
//			fprintf (stdout, "  |");
			rtn = snprintf (pszWk, size, "  |");
			pszWk += rtn;
			size -= rtn;

			k = 0;
			while (k < nSrcLen) {
				// 制御コード系は'.'で代替
//				fprintf (stdout, "%c", (*(pSrc+k)>0x20) && (*(pSrc+k)<0x7f) ? *(pSrc+k) : '.');
				rtn = snprintf (pszWk, size, "%c", (*(pSrc+k)>0x20) && (*(pSrc+k)<0x7f) ? *(pSrc+k) : '.');
				pszWk += rtn;
				size -= rtn;

				++ k;
			}
			for (int i = 0; i < (16 - nSrcLen); ++ i) {
//				fprintf (stdout, " ");
				rtn = snprintf (pszWk, size, " ");
				pszWk += rtn;
				size -= rtn;
			}
		}

//		fprintf (stdout, "|\n");
		rtn = snprintf (pszWk, size, "|\n");
		pszWk += rtn;
		size -= rtn;

		_UTL_LOG_I ("%s", szWk);
	}
}
