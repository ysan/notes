#ifndef _IMM_SOCKET_SERVICE_COMMON_H_
#define _IMM_SOCKET_SERVICE_COMMON_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "Utils.h"

using namespace std;


namespace ImmSocketService {


extern void setLogLevel (EN_LOG_LEVEL enLvl);
extern EN_LOG_LEVEL getLogLevel (void);

/**
 * log macro
 */
#ifndef _ANDROID_BUILD

// --- Information ---
#ifndef _LOG_ADD_FILE_INFO
#define _ISS_LOG_I(fmt, ...) {\
	CUtils::putsLogLW (stdout, getLogLevel(), EN_LOG_LEVEL_I, fmt, ##__VA_ARGS__);\
}
#else
#define _ISS_LOG_I(fmt, ...) {\
	CUtils::putsLog (stdout, getLogLevel(), EN_LOG_LEVEL_I, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}
#endif

// --- Notice ---
#ifndef _LOG_ADD_FILE_INFO
#define _ISS_LOG_N(fmt, ...) {\
	CUtils::putsLogLW (stdout, getLogLevel(), EN_LOG_LEVEL_N, fmt, ##__VA_ARGS__);\
}
#else
#define _ISS_LOG_N(fmt, ...) {\
    CUtils::putsLog (stdout, getLogLevel(), EN_LOG_LEVEL_N, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}
#endif

// --- Warning ---
#ifndef _LOG_ADD_FILE_INFO
#define _ISS_LOG_W(fmt, ...) {\
	CUtils::putsLogLW (stdout, getLogLevel(), EN_LOG_LEVEL_W, fmt, ##__VA_ARGS__);\
}
#else
#define _ISS_LOG_W(fmt, ...) {\
    CUtils::putsLog (stdout, getLogLevel(), EN_LOG_LEVEL_W, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}
#endif

// --- Error ---
#ifndef _LOG_ADD_FILE_INFO
#define _ISS_LOG_E(fmt, ...) {\
	CUtils::putsLogLW (stdout, getLogLevel(), EN_LOG_LEVEL_E, fmt, ##__VA_ARGS__);\
}
#else
#define _ISS_LOG_E(fmt, ...) {\
    CUtils::putsLog (stdout, getLogLevel(), EN_LOG_LEVEL_E, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}
#endif

// --- perror ---
#ifndef _LOG_ADD_FILE_INFO
#define _ISS_PERROR(fmt, ...) {\
	CUtils::putsLogLW (stdout, getLogLevel(), EN_LOG_LEVEL_PE, fmt, ##__VA_ARGS__);\
}
#else
#define _ISS_PERROR(fmt, ...) {\
	CUtils::putsLog (stdout, getLogLevel(), EN_LOG_LEVEL_PE, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}
#endif

#else // _ANDROID_BUILD

// --- Information ---
#define _ISS_LOG_I(fmt, ...) do {\
	__android_log_print (ANDROID_LOG_DEBUG, __func__, fmt, ##__VA_ARGS__); \
} while (0)

// --- Notice ---
#define _ISS_LOG_N(fmt, ...) do {\
	__android_log_print (ANDROID_LOG_INFO, __func__, fmt, ##__VA_ARGS__); \
} while (0)

// --- Warning ---
#define _ISS_LOG_W(fmt, ...) do {\
	__android_log_print (ANDROID_LOG_WARN, __func__, fmt, ##__VA_ARGS__); \
} while (0)

// --- Error ---
#define _ISS_LOG_E(fmt, ...) do {\
	__android_log_print (ANDROID_LOG_ERROR, __func__, fmt, ##__VA_ARGS__); \
} while (0)

// --- perror ---
#define _ISS_PERROR(fmt, ...) do {\
	__android_log_print (ANDROID_LOG_ERROR, __func__, fmt, ##__VA_ARGS__); \
	char szPerror[32]; \
	strerror_r(errno, szPerror, sizeof (szPerror)); \
	__android_log_print (ANDROID_LOG_ERROR, __func__, "%s", szPerror); \
} while (0)

#endif

} // namespace ImmSocketService

#endif
