#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "defines.h"
#include "reg3067.h"
#include "timer.h"
#include "sci.h"
#include "led.h"
#include "util.h"


typedef struct led_blink_info {
	WORD *pwPat;
	WORD wRowNum;
	WORD wIntrCnt;
	WORD wLoopCnt;
} ST_LED_BLINK_INFO;


/*
 * $BJQ?t@k8@(B
 */
static DWORD gdwIntrCnt;
static DWORD gdwRowCnt;
static DWORD gdwLoopCnt;
static BOOL gIsToggle;
static BOOL gIsNext;
static WORD gwIdx;
static BYTE gbCol_9_5;
static BYTE gbCol_4_0;
static ST_LED_BLINK_INFO *gpstNowGroup;
static WORD gwNowPtn;
static WORD gwBeforePtn;
static WORD gwCntIntr[10];
static WORD gwCntIntrTmp[10];


static WORD gwLedBarPtn00[10];
static WORD gwLedBarPtn01[10];
static WORD gwLedBarPtn02[10];
static WORD gwLedBarPtn03[1];

static ST_LED_BLINK_INFO gstLedBarGr00 = { gwLedBarPtn00, 10, 100, 5 };
static ST_LED_BLINK_INFO gstLedBarGr01 = { gwLedBarPtn01, 10, 100, 5 };
static ST_LED_BLINK_INFO gstLedBarGr02 = { gwLedBarPtn02, 10, 100, 1 };
static ST_LED_BLINK_INFO gstLedBarGr03 = { gwLedBarPtn03, 1, 100, 50 };

static ST_LED_BLINK_INFO *gpstGroups[] = {
	&gstLedBarGr00,
	&gstLedBarGr01,
	&gstLedBarGr02,
	&gstLedBarGr03,
	&gstLedBarGr02,
	&gstLedBarGr03,
	&gstLedBarGr02,
	&gstLedBarGr03,
	NULL,
};



/*
 * $B%W%m%H%?%$%W@k8@(B
 */
void ddddd( void ); // extern
void initLedBar( void ); // extern
void setNextGroupLedBar( void ); // extern
void createDataLedBar( void ); // extern
void checkTurnOffLedBar( void ); // extern
static void turnOff( BYTE bIdx, BYTE bN );
void setDynamicLedBar( void ); //extern


void ddddd( void )
{
	logHexVal( "&gstLedBarGr00 = 0x", (DWORD)&gstLedBarGr00 );
	logHexVal( "&gstLedBarGr01 = 0x", (DWORD)&gstLedBarGr01 );
	logHexVal( "gpstGroups[0] = 0x", (DWORD)gpstGroups[0] );
	logHexVal( "gpstGroups[1] = 0x", (DWORD)gpstGroups[1] );
	logHexVal( "gpstGroups[2] = 0x", (DWORD)gpstGroups[2] );
	logHexVal( "&gwIdx = 0x", (DWORD)&gwIdx );
	logDecVal( "gwIdx = ", gwIdx );
	logHexVal( "gpstNowGroup = 0x", (DWORD)gpstNowGroup );
	logHexVal( "gbCol_9_5 = 0x", gbCol_9_5 );
	logHexVal( "gbCol_4_0 = 0x", gbCol_4_0 );
	logDecVal( "gdwIntrCnt = ", gdwIntrCnt );
	logDecVal( "gdwRowCnt = ", gdwRowCnt );
	logDecVal( "gstLedBarGr01.wRowNum = ", gstLedBarGr01.wRowNum );
	logDecVal( "gstLedBarGr01.wIntrCnt = ", gstLedBarGr01.wIntrCnt );
	logHexVal( "&gstLedBarGr01.wRowNum = 0x", (DWORD)&gstLedBarGr01.wRowNum );
	logHexVal( "&gstLedBarGr01.wIntrCnt = 0x", (DWORD)&gstLedBarGr01.wIntrCnt );
}

/**
 * LED$B%P!<=i4|2=(B
 */
void initLedBar( void )
{
	P4DDR = 0x7f;
	P4DR = 0x7f;

	gdwIntrCnt = 0;
	gdwRowCnt = 0;
	gdwLoopCnt = 0;
	gIsToggle = FALSE;
	gIsNext = FALSE;
	gwIdx = 0;
	gbCol_9_5 = 0;
	gbCol_4_0 = 0;
	gpstNowGroup = NULL;
	gwNowPtn = 0;
	gwBeforePtn = 0;
	memset( gwCntIntr, 0xff, sizeof(gwCntIntr) );
	memset( gwCntIntrTmp, 0x00, sizeof(gwCntIntrTmp) );


	/* led$B%G!<%?%Q%?%s(B */
	gwLedBarPtn00[0] = (WORD)bit2val("0111110000");
	gwLedBarPtn00[1] = (WORD)bit2val("1111100000");
	gwLedBarPtn00[2] = (WORD)bit2val("1111000001");
	gwLedBarPtn00[3] = (WORD)bit2val("1110000011");
	gwLedBarPtn00[4] = (WORD)bit2val("1100000111");
	gwLedBarPtn00[5] = (WORD)bit2val("1000001111");
	gwLedBarPtn00[6] = (WORD)bit2val("0000011111");
	gwLedBarPtn00[7] = (WORD)bit2val("0000111110");
	gwLedBarPtn00[8] = (WORD)bit2val("0001111100");
	gwLedBarPtn00[9] = (WORD)bit2val("0011111000");

	gwLedBarPtn01[0] = (WORD)bit2val("0111111110");
	gwLedBarPtn01[1] = (WORD)bit2val("0011111100");
	gwLedBarPtn01[2] = (WORD)bit2val("0001111000");
	gwLedBarPtn01[3] = (WORD)bit2val("0000110000");
	gwLedBarPtn01[4] = (WORD)bit2val("0000000000");
	gwLedBarPtn01[5] = (WORD)bit2val("1000000001");
	gwLedBarPtn01[6] = (WORD)bit2val("1100000011");
	gwLedBarPtn01[7] = (WORD)bit2val("1110000111");
	gwLedBarPtn01[8] = (WORD)bit2val("1111001111");
	gwLedBarPtn01[9] = (WORD)bit2val("1111111111");

	gwLedBarPtn02[0] = (WORD)bit2val("1000000000");
	gwLedBarPtn02[1] = (WORD)bit2val("0100000000");
	gwLedBarPtn02[2] = (WORD)bit2val("0010000000");
	gwLedBarPtn02[3] = (WORD)bit2val("0001000000");
	gwLedBarPtn02[4] = (WORD)bit2val("0000100000");
	gwLedBarPtn02[5] = (WORD)bit2val("0000010000");
	gwLedBarPtn02[6] = (WORD)bit2val("0000001000");
	gwLedBarPtn02[7] = (WORD)bit2val("0000000100");
	gwLedBarPtn02[8] = (WORD)bit2val("0000000010");
	gwLedBarPtn02[9] = (WORD)bit2val("0000000001");

	gwLedBarPtn03[0] = (WORD)bit2val("0000000000");


	// $B=i4|CM$,2u$l$F$k$N$G$3$3$GF~$lD>$9(B $B%9%?%C%/%*!<%P!<%U%m!<$+(B...
	gstLedBarGr00.pwPat = gwLedBarPtn00;
	gstLedBarGr00.wRowNum = 10;
	gstLedBarGr00.wIntrCnt = 100;
	gstLedBarGr00.wLoopCnt = 5;

	gstLedBarGr01.pwPat = gwLedBarPtn01;
	gstLedBarGr01.wRowNum = 10;
	gstLedBarGr01.wIntrCnt = 100;
	gstLedBarGr01.wLoopCnt = 5;

	gstLedBarGr02.pwPat = gwLedBarPtn02;
	gstLedBarGr02.wRowNum = 10;
	gstLedBarGr02.wIntrCnt = 100;
	gstLedBarGr02.wLoopCnt = 1;

	gstLedBarGr03.pwPat = gwLedBarPtn03;
	gstLedBarGr03.wRowNum = 1;
	gstLedBarGr03.wIntrCnt = 100;
	gstLedBarGr03.wLoopCnt = 20;

}

/**
 * $BE@Et%G!<%?<!%0%k!<%W$r%;%C%H(B
 * $B3d$j9~$_$G8F$S=P$9(B
 */
void setNextGroupLedBar( void )
{
	if (gIsNext) {
		gIsNext = FALSE;
		gwIdx ++;

		if (!gpstGroups[ gwIdx ]) {
			gwIdx = 0;
		}

		logDecVal( "next gwIdx = ", gwIdx );
	}

	gpstNowGroup = gpstGroups[ gwIdx ];
}

/**
 * $BE@Et%G!<%?%;%C%H(B
 * $B3d$j9~$_$G8F$S=P$9(B
 */
void createDataLedBar( void )
{
	ST_LED_BLINK_INFO *pstInfo = gpstNowGroup;

	if (!pstInfo) {
		return;
	}

	gdwIntrCnt ++;

	if (gdwIntrCnt <= pstInfo->wIntrCnt) {
		return;
	}
	gdwIntrCnt = 0;

	gwNowPtn = *((pstInfo->pwPat)+gdwRowCnt);

	/* Colmun$B%G!<%?:n@.(B */
	gbCol_9_5 = (gwNowPtn >> 5) & 0x1f;
	gbCol_4_0 = (gwNowPtn)      & 0x1f;

	/* Row$BCM99?7(B */
	gdwRowCnt ++;

	if (gdwRowCnt >= pstInfo->wRowNum) {
		gdwRowCnt = 0;
		gdwLoopCnt ++;
	}

	if (gdwLoopCnt >= pstInfo->wLoopCnt) {
		gdwLoopCnt = 0;
		gIsNext = TRUE;
	}

//logPrint( "%2d %2d [%05b%05b]\n", gdwLoopCnt, gdwRowCnt, gbCol_9_5, gbCol_4_0 );
}

/**
 *
 *
 */
void checkTurnOffLedBar( void )
{
	turnOff (0, 17);
	turnOff (1, 17);
	turnOff (2, 17);
	turnOff (3, 17);
	turnOff (4, 17);
	turnOff (5, 17);
	turnOff (6, 17);
	turnOff (7, 17);
	turnOff (8, 17);
	turnOff (9, 17);
}

/**
 * gwBeforePtn $B$H(BgwNowPtn $B$rHf3S$7(B
 * 1 -> 0 $B$K@Z$jBX$o$C$?;~$K(BLED$B51EY8:?j=hM}$r9T$&(B
 * $B0z?t(BbN$B$r3d$j9~$_?t$G(B1$B<~4|$H$7$F(B $B%G%e!<%F%#!<Hf$rJQ2=$5$;$k(B
 */
static void turnOff( BYTE bIdx, BYTE bN )
{
	if ((bIdx < 0) && (bIdx >= 10)) {
		return;
	}

	WORD v = 1 << bIdx;

	if (((gwBeforePtn & v) == v) && ((gwNowPtn & v) == 0)) {
		/* 1 -> 0 $B@Z$jBX$o$C$?(B */
		gwCntIntr[ bIdx ] = 0;

	} else if ((gwNowPtn & v) == v) {
		/* 1 $B$K$J$C$?(B ($B%/%j%"(B) */
		gwCntIntr[ bIdx ] = 0xffff;
		gwCntIntrTmp[ bIdx ] = 0;
	}


	if (gwCntIntr[ bIdx ] != 0xffff) {

		/* bN$B2s3d$j9~$_$G(B1$B<~4|$H9M$($k(B */
		if ((gwCntIntr[ bIdx ] % bN) == 0) {
			/* high */
			if ((bIdx >= 0) && (bIdx < 5)) {
				gbCol_4_0 |= 1 << bIdx;
			} else {
				gbCol_9_5 |= 1 << (bIdx -5);
			}

			/* $B<!$K(Blow$B$KMn$9%?%$%_%s%0(B */
			gwCntIntrTmp[ bIdx ] = gwCntIntr[ bIdx ] +bN -(gwCntIntr[ bIdx ] / bN) -1;
		}

		if (gwCntIntr[ bIdx ] == gwCntIntrTmp[ bIdx ]) {
			/* low */
			if ((bIdx >= 0) && (bIdx < 5)) {
				gbCol_4_0 &= ~(1 << bIdx);
			} else {
				gbCol_9_5 &= ~(1 << (bIdx -5));
			}
		}

		gwCntIntr[ bIdx ] ++;

		/* $BDd;_(B bN*bN$B2s(B) */
		if (gwCntIntr[ bIdx ] == bN*bN) {
			/* $B%/%j%"(B */
			gwCntIntr[ bIdx ] = 0xffff;
			gwCntIntrTmp[ bIdx ] = 0;
		}
	}
}

/**
 * $B%@%$%J%_%C%/E@Et@Z$jBX$((B
 * $B3d$j9~$_$G8F$S=P$9(B
 *
 * $B:G=*E*$J%G!<%?$r%l%8%9%?$K%;%C%H$9$k$N$G0lO"$N=hM}$N:G8e$K9T$&$3$H(B
 */
void setDynamicLedBar( void )
{
	/* $BA0$N%G!<%?%Q%?%sJ]B8(B */
	gwBeforePtn = gwNowPtn;

	/* $B%@%$%J%_%C%/E@Et@Z$jBX$((B */
	if (!gIsToggle) {
		P4DR = (gbCol_4_0 & 0x1f) | (0<<5) | (1<<6);
		gIsToggle = TRUE;
	} else {
		P4DR = (gbCol_9_5 & 0x1f) | (1<<5) | (0<<6);
		gIsToggle = FALSE;
	}
}
