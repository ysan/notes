#ifndef _CURSUTIL_H_
#define _CURSUTIL_H_

#include <ncurses.h>
#include "mycommon.h"

/*
 * Constant define
 */


/*
 * Type define
 */


/*
 * External
 */
extern WINDOW *GetWindow( void );
extern BOOL InitCursUtil( void );
extern void StartCursUtil( void );
extern void FinalizeCursUtil( void );


#endif
