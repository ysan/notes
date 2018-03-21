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


#define STR_BUFF_SIZE				(1024)
#define STR_ITEM_IDX_MAX			(64)

typedef enum {
	EN_STR_ITEM_STATE_NOUSE = 0,
	EN_STR_ITEM_STATE_READY,
	EN_STR_ITEM_STATE_USE
} EN_STR_ITEM_STATE;

typedef struct str_item_info {
	int nIdx;
	EN_CUTL_DISP_STR_KIND enKind;
	int nSetPosH;
	int nSetPosW;
	char szBuff[ STR_BUFF_SIZE ];
	EN_CUTL_SCROLL_SPEED enSpd;;
} ST_STR_ITEM_INFO;

typedef void (*PFUNC)( int );


/*
 * Prototypes
 */
static int GetIdxString( void );
static void FixedString ( int nIdx );
static void ScrollString ( int nIdx );

/*
 * Variables
 */
static EN_STR_ITEM_STATE g_enStrItemState[ STR_ITEM_IDX_MAX ];
static ST_STR_ITEM_INFO g_stStrItemInfo[ STR_ITEM_IDX_MAX ];
static const PFUNC pFunc[ EN_CUTL_DISP_STR_KIND_MAX ] = {
	FixedString,
	ScrollString
};


/**
 *
 *
 */
void CheckUpdateStringItem( void )
{
	int nIdx = 0;

	for ( nIdx=0; nIdx<STR_ITEM_IDX_MAX; nIdx++ ) {
		if (g_enStrItemState[ nIdx ] != EN_STR_ITEM_STATE_NOUSE) {
			if (pFunc[ nIdx ]) {
				pFunc[ nIdx ]( g_stStrItemInfo[ nIdx ].enKind );
			}
		}
	}

}

/**
 *
 *
 */
static int GetIdxString( void )
{
	int nIdx = 0;
	for ( nIdx=0; nIdx<STR_ITEM_IDX_MAX; nIdx++ ) {
		if (g_enStrItemState[ nIdx ] == EN_STR_ITEM_STATE_NOUSE) {

			/* state更新 */
			g_enStrItemState[ nIdx ] = EN_STR_ITEM_STATE_READY;

			return nIdx;
		}
	}

	/* 全部つかってる */
	LOG_E( "idx using all.\n" );
	return -1;
}

/**
 *
 *
 */
void ClearIdxStringCursUtil( int nIdx )
{
	/* 問答無用でクリア */
	g_enStrItemState[ nIdx ] = EN_STR_ITEM_STATE_NOUSE;
	memset( &g_stStrItemInfo[ nIdx ], 0x00, sizeof(ST_STR_ITEM_INFO) );
}

/**
 *
 *
 */
void ClearAllIdxStringCursUtil( void )
{
	int nIdx = 0;
	for ( nIdx=0; nIdx<STR_ITEM_IDX_MAX; nIdx++ ) {
		ClearIdxStringCursUtil( nIdx );
	}
}

/**
 *
 *
 */
int SetStringItemCursUtil (
	EN_CUTL_DISP_STR_KIND enKind,
	void *pOpt,
	int nSetPosH,
	int nSetPosW,
	char *pszFmt,
	...
)
{
	if (enKind < EN_CUTL_DISP_STR_KIND_FIXED) {
		LOG_E( "kind is invalid.\n" );
		return -1;
	} else if (enKind >= EN_CUTL_DISP_STR_KIND_MAX) {
		LOG_E( "kind is invalid.\n" );
		return -1;
	}

	if (nSetPosH < 0) {
		nSetPosH = 0;
	}

	if (nSetPosW < 0) {
		nSetPosW = 0;
	}

	int nIdx = GetIdxString();
	if (nIdx < 0) {
		LOG_E( "GetIdxString() is failure.\n" );
		return -1;
	}

	g_stStrItemInfo[ nIdx ].nIdx = nIdx;

	g_stStrItemInfo[ nIdx ].enKind = enKind;
	g_stStrItemInfo[ nIdx ].nSetPosH = nSetPosH;
	g_stStrItemInfo[ nIdx ].nSetPosW = nSetPosW;

	/* 可変引数セット */
	memset( g_stStrItemInfo[ nIdx ].szBuff, 0x00, STR_BUFF_SIZE );
	va_list va;
	va_start( va, pszFmt );
	vsnprintf( g_stStrItemInfo[ nIdx ].szBuff, STR_BUFF_SIZE, pszFmt, va );
	va_end( va );

	/* option引数 */
	if (enKind == EN_CUTL_DISP_STR_KIND_SCROLL) {
		if (pOpt) {
			g_stStrItemInfo[ nIdx ].enSpd = *((EN_CUTL_DISP_STR_KIND*)pOpt);
		} else {
			/* NULLだったらDEFでセット */
			g_stStrItemInfo[ nIdx ].enSpd = EN_CUTL_SCROLL_SPEED_DEF;
		}
	} else {
		g_stStrItemInfo[ nIdx ].enSpd = 0; /* 使わない */
	}

	return nIdx;
}

/**
 *
 *
 */
void ModStringCursUtil( int nIdx, char *pszFmt, ... )
{
	if ((nIdx < 0)||(nIdx >= STR_ITEM_IDX_MAX)) {
		return;
	}

	/* 可変引数セット */
	memset( g_stStrItemInfo[ nIdx ].szBuff, 0x00, STR_BUFF_SIZE );
	va_list va;
	va_start( va, pszFmt );
	vsnprintf( g_stStrItemInfo[ nIdx ].szBuff, STR_BUFF_SIZE, pszFmt, va );
	va_end( va );
}

/**
 *
 *
 */
void ModPosCursUtil( int nIdx, int nSetPosH, int nSetPosW )
{
	if ((nIdx < 0)||(nIdx >= STR_ITEM_IDX_MAX)) {
		return;
	}

	if (nSetPosH < 0) {
		nSetPosH = 0;
	}

	if (nSetPosW < 0) {
		nSetPosW = 0;
	}

	g_stStrItemInfo[ nIdx ].nSetPosH = nSetPosH;
	g_stStrItemInfo[ nIdx ].nSetPosW = nSetPosW;
}

/**
 *
 *
 */
static void FixedString( int nIdx )
{
	int nGetW;
	int nGetH;
	ST_STR_ITEM_INFO *pInfo = &g_stStrItemInfo[ nIdx ];
	WINDOW *pWin = GetWindow();


	if ((int)strlen(pInfo->szBuff) == 0) {
		ClearIdxStringCursUtil( nIdx );
		LOG_E( "Clear internal area for the length 0. [idx:%d]\n", nIdx );
		return;
	}

	if (g_enStrItemState[ nIdx ] == EN_STR_ITEM_STATE_READY) {
		/* state更新 */
		g_enStrItemState[ nIdx ] = EN_STR_ITEM_STATE_USE;
	}

//TODO
	getmaxyx( pWin, nGetH, nGetW );

	if ( pInfo->nSetPosH > nGetH ) {
		return;
	}

	if ( pInfo->nSetPosW > nGetW ) {
		return;
	}

	/* 表示位置からの文字列長が横幅を越えた場合 */
	if ( nGetW < pInfo->nSetPosW+(int)strlen(pInfo->szBuff) ) {
		pInfo->szBuff[ nGetW - pInfo->nSetPosW ] = 0x00;
		pInfo->szBuff[ nGetW - pInfo->nSetPosW -1 ] = '>';
	}

	mvwaddnstr( pWin, pInfo->nSetPosH, pInfo->nSetPosW, pInfo->szBuff, (int)strlen(pInfo->szBuff) );

}

/**
 *
 *
 */
static void ScrollString ( int nIdx )
{
	int nGetW;
	int nGetH;
	int nWriteLen;
	BOOL isWrap = FALSE;
	ST_STR_ITEM_INFO *pInfo = &g_stStrItemInfo[ nIdx ];
	WINDOW *pWin = GetWindow();


	/* static values */
//TODO 整理したい
	static int n[ STR_ITEM_IDX_MAX ] = { 0 };
	static int nNowPosW[ STR_ITEM_IDX_MAX ] = { 0 };


	if ((int)strlen(pInfo->szBuff) == 0) {
		ClearIdxStringCursUtil( nIdx );
		LOG_E( "Clear internal area for the length 0. [idx:%d]\n", nIdx );
		return;
	}

	if (g_enStrItemState[ nIdx ] == EN_STR_ITEM_STATE_READY) {
		/* 初回のみ受け取る */
		nNowPosW[ nIdx ] = pInfo->nSetPosW;

		/* state更新 */
		g_enStrItemState[ nIdx ] = EN_STR_ITEM_STATE_USE;
	}

//TODO
	getmaxyx( pWin, nGetH, nGetW );

	if ( pInfo->nSetPosH > nGetH ) {
		return;
	}

	if (pInfo->enSpd < EN_CUTL_SCROLL_SPEED_VSLOW) {
		pInfo->enSpd = EN_CUTL_SCROLL_SPEED_VSLOW;
	} else if (pInfo->enSpd > EN_CUTL_SCROLL_SPEED_VFAST) {
		pInfo->enSpd = EN_CUTL_SCROLL_SPEED_VFAST;
	}

	/* 文字列長が横幅を越えたら切り詰める */
	if ( nGetW < (int)strlen(pInfo->szBuff) ) {
		memset( pInfo->szBuff+nGetW, 0x00, STR_BUFF_SIZE-(int)strlen(pInfo->szBuff) );
	}

	/* 折り返しチェック */
	if ( nGetW - nNowPosW[ nIdx ] >= (int)strlen(pInfo->szBuff) ) {
		/* 折り返し無 */
		nWriteLen = (int)strlen(pInfo->szBuff);
		isWrap = FALSE;
	} else {
		/* 折り返し有 */
		nWriteLen = nGetW - nNowPosW[ nIdx ];
		isWrap = TRUE;
	}

	mvwaddnstr( pWin, pInfo->nSetPosH, nNowPosW[ nIdx ], pInfo->szBuff, nWriteLen );

	if (isWrap) {
		mvwaddnstr( pWin, pInfo->nSetPosH, 0, pInfo->szBuff+nWriteLen, (int)strlen(pInfo->szBuff)-nWriteLen );
	}

	/* scroll speed */
	n[ nIdx ] --;
	if (n[ nIdx ] < pInfo->enSpd) {
		if (pInfo->enSpd > 0) {
			nNowPosW[ nIdx ] += pInfo->enSpd +1;
		} else {
			nNowPosW[ nIdx ] ++;
		}
		n[ nIdx ] = 0;
	}

	if (nGetW < nNowPosW[ nIdx ]) {
		/* 一周移動した */
		nNowPosW[ nIdx ] = 0;
	}
}
