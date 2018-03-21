#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ncurses.h>
#include <stdarg.h>
#include <pthread.h>

#include "mycommon.h"
#include "cursutil.h"
#include "cursutil_string.h"


#define REFRESH_RATE_USEC			(100000)

typedef enum {
	EN_STATE_INIT = 0,
	EN_STATE_READY,
	EN_STATE_START,
	EN_STATE_END
} EN_STATE;


/*
 * Variables
 */
static WINDOW *g_pWin;
static EN_STATE g_enState = EN_STATE_INIT; // $B=i4|2=$7$F@k8@(B
static pthread_mutex_t gMutex;
static pthread_cond_t gCond;
static pthread_t g_nThId;


/*
 * Prototypes
 */
static void *CursUtilThread( void *pThArg );
static BOOL CreateThread( void );


WINDOW *GetWindow( void )
{
	return g_pWin;
}

static void *CursUtilThread( void *pThArg )
{
	int nW;
	int nH;


	/* state update $B$3$N0LCV$G(B */
	g_enState = EN_STATE_START;


	/* thread wait */
	pthread_mutex_lock( &gMutex );
	pthread_cond_wait( &gCond, &gMutex );
	pthread_mutex_unlock( &gMutex );


	initscr();
	noecho();
	cbreak();

//TODO $B<:GT$9$k$H$-$"$k$N$+ITL@(B
	g_pWin = newwin( LINES, COLS-1, 0, 0 );


//TODO $B8=>u%&%#%s%I%&%5%$%:(B=$B%&%#%s%I%&OH$N$_(B
	while (1) {
		if (g_enState == EN_STATE_END) {
			/* thread end */
			break;
		}

		werase( g_pWin );

getmaxyx( g_pWin, nH, nW );
mvwprintw( g_pWin, nH-1, nW-8, "%d %d", nH, nW );
mvwaddnstr( g_pWin, nH/2, nW/2, "test", 4 );


		CheckUpdateStringItem();


		wrefresh( g_pWin );
		usleep( REFRESH_RATE_USEC );
	}

	endwin();

	return NULL;
}

static BOOL CreateThread( void )
{
	if ( pthread_create( &g_nThId, NULL, CursUtilThread, NULL ) != 0 ) {
		PERROR( "pthread_create()" );
		return FALSE;
	}

	return TRUE;
}

BOOL InitCursUtil( void )
{
	if (g_enState != EN_STATE_INIT) {
		/* 1$B2s$7$+8F$Y$^$;$s(B */
		LOG_W( "Already been executed." );
		return FALSE;
	}

	/* init static values */
	g_pWin = NULL;
	g_nThId = 0;
	pthread_mutex_init( &gMutex, NULL );
	pthread_cond_init( &gCond, NULL );


	if (!CreateThread()) {
		LOG_E( "CreateThread() is failure.\n" );

		/* state$B$r(BINIT$B$K$7$F=*$k(B */
		g_enState = EN_STATE_INIT;

		return FALSE;
	}


	/* state update */
	g_enState = EN_STATE_READY;

	return TRUE;
}

void StartCursUtil( void )
{
	if (g_enState != EN_STATE_READY) {
		LOG_W( "It is not a state can be executed. [state:%d]\n", g_enState );
	}

	while (g_enState != EN_STATE_START) {
		/*
		 * TODO
		 * $B:GE,2=$5$l$F>C$($F$7$^$&$N$G(B $B;CDj$G%m%0$r=P$7$F$^$9(B
		 * $B$b$7$/$O%3%s%Q%$%k:GE,2=%l%Y%k$rJQ$($k$+(B
		 */
		LOG_I( "wait thread start. [state:%d]\n", g_enState );
	}

	/* thread start */
	pthread_cond_signal( &gCond );
}

void FinalizeCursUtil( void )
{
	if (g_enState != EN_STATE_START) {
		LOG_W( "It is not a state can be executed. [state:%d]\n", g_enState );
		return;
	}

	/* $B%9%l%C%I=*$o$i$;$k(B */
	g_enState = EN_STATE_END;

	if (pthread_join( g_nThId, NULL ) != 0) {
		PERROR( "pthread_join()" );
	}

	g_enState = EN_STATE_INIT;
}
