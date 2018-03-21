#ifndef _CURSUTIL_STRING_H_
#define _CURSUTIL_STRING_H_

#include <ncurses.h>
#include "mycommon.h"

/*
 * Constant define
 */


/*
 * Type define
 */
typedef enum {
	EN_CUTL_DISP_STR_KIND_FIXED = 0,
	EN_CUTL_DISP_STR_KIND_SCROLL,
	EN_CUTL_DISP_STR_KIND_MAX
} EN_CUTL_DISP_STR_KIND;

typedef enum {
	EN_CUTL_SCROLL_SPEED_VSLOW = -2,
	EN_CUTL_SCROLL_SPEED_SLOW,
	EN_CUTL_SCROLL_SPEED_DEF,
	EN_CUTL_SCROLL_SPEED_FAST,
	EN_CUTL_SCROLL_SPEED_VFAST
} EN_CUTL_SCROLL_SPEED;


/*
 * External
 */
extern void CheckUpdateStringItem( void );
extern void ClearIdxStringCursUtil( int nIdx );
extern void ClearAllIdxStringCursUtil( void );
extern int SetStringItemCursUtil (
	EN_CUTL_DISP_STR_KIND enKind,
	void *pOpt,
	int nSetPosH,
	int nSetPosW,
	char *pszFmt,
	...
);
extern void ModStringCursUtil( int nIdx, char *pszFmt, ... );
extern void ModPosCursUtil( int nIdx, int nSetPosH, int nSetPosW );


#endif
