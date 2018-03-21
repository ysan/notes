/*
 * compile command:
 *      cc tree.c -o tree -Wall -Imycommon -Lmycommon -lmycommon
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "mycommon.h"


typedef struct info {
	char szData[ 128 ];
} ST_INFO;

typedef struct tree {
	uint16_t nIdx;
	struct tree *pLeft;
	struct tree *pRight;
	ST_INFO stInfo; // tree$B9=B$$GJ];}$9$k<g%G!<%?(B
} ST_TREE;


static uint32_t gnCntTree;
static ST_TREE *gpstTreeRoot;
static ST_TREE *gpstSearchBefore; // $B%5!<%ACf(B $BJ];}%G!<%?(B
static BOOL g_isLeftSearchBefore; // $B%5!<%ACf(B $BJ];}%G!<%?(B $B:8$+1&$+(B



void InitTree( void );											// extern
BOOL AddTree( ST_INFO *pstInfo, size_t nArgSize );				// extern
static BOOL AddTreeFromIdx( uint16_t nIdx, ST_INFO *pstInfo );
static BOOL AddTreeImp( ST_TREE *pstAdd );
BOOL DelTree( uint16_t nIdx );									// extern
ST_TREE *SearchTree( uint16_t nIdx );							// extern
static void RefTree( ST_TREE *pstArg );
void RefAllTree();												// extern
static void DestroyTree( ST_TREE *pstArg );
void DestroyAllTree();											// extern
static uint16_t GenIdx( const uint16_t *pArg, size_t nArgSize );


/**
 * $B=i4|2=(B
 */
void InitTree( void )
{
	DestroyAllTree();

	gnCntTree = 0;
	gpstTreeRoot = NULL;
	gpstSearchBefore = NULL;
	g_isLeftSearchBefore = FALSE;
}

/**
 * $BMWAGDI2C(B
 */
BOOL AddTree( ST_INFO *pstInfo, size_t nArgSize )
{
	if (gnCntTree >= 0xffff) {
		return FALSE;
	}

	uint32_t nIdx = GenIdx( (const uint16_t*)pstInfo, nArgSize );

	/* index$B=EJ#3NG'$9$k(B */
	while (1) {
		if (!SearchTree( (uint16_t)nIdx )) {
			break;
		}
		nIdx ++;
		nIdx &= 0xffff;
	}

	if (!AddTreeFromIdx( (uint16_t)nIdx, pstInfo )) {
		return FALSE;
	}

	gnCntTree ++;

	return TRUE;
}

/**
 * $BMWAGDI2C(B
 * index$B$h$j(B
 */
static BOOL AddTreeFromIdx( uint16_t nIdx, ST_INFO *pstInfo )
{
	ST_TREE *pstAdd = NULL;

	pstAdd = (ST_TREE*)malloc( sizeof(ST_TREE) );
	if (!pstAdd) {
		PERROR( "malloc()" );
		return FALSE;
	}

	pstAdd->nIdx = nIdx;
	memcpy( &(pstAdd->stInfo), pstInfo, sizeof(ST_INFO) );
	pstAdd->pLeft = NULL;
	pstAdd->pRight = NULL;


	/* $B=i2s(B */
	if (!gpstTreeRoot) {
		gpstTreeRoot = pstAdd;
		return TRUE;
	}

	/* $B=i2s0J9_$N>l9g$O$3$3$K(B */
	if (!AddTreeImp( pstAdd )) {
		return FALSE;
	}

	return TRUE;
}

/**
 * $BMWAGDI2C(B
 * $B8!:w$7$FDI2C(B
 */
static BOOL AddTreeImp( ST_TREE *pstAdd )
{
	BOOL bRtn = TRUE;
	ST_TREE *pstWork = gpstTreeRoot;

	if (!pstAdd) {
		return FALSE;
	}

	/* root$B$,6u(B */
	if (!pstWork) {
		return FALSE;
	}

	while (1) {
		if (pstWork->nIdx > pstAdd->nIdx) {
			if (!(pstWork->pLeft)) {
				pstWork->pLeft = pstAdd;
				break;
			} else {
				/* $BB39T(B */
				pstWork = pstWork->pLeft;
			}
		} else if (pstWork->nIdx < pstAdd->nIdx) {
			if (!(pstWork->pRight)) {
				pstWork->pRight = pstAdd;
				break;
			} else {
				/* $BB39T(B */
				pstWork = pstWork->pRight;
			}
		} else {
			/*
			 * index$B$,=EJ#$7$?>l9g(B
			 * $B$3$3$K$O$3$J$$$O$:(B
			 */
			bRtn = FALSE;
			LOG_E( "index duplicate.\n" );
			break;
		}
	}

	return bRtn;
}

/**
 * $B:o=|(B
 * $BMWAG$r(B1$B$D:o=|(B
 */
BOOL DelTree( uint16_t nIdx )
{
	ST_TREE *pstWork = NULL;
	ST_TREE *pstWorkLeft = NULL;
	ST_TREE *pstWorkRight = NULL;


	pstWork = SearchTree( nIdx );
	if (!pstWork) {
		LOG_W( "index not found.\n" );
		return FALSE;
	}

	/* 	$B?F$N%]%$%s%?$r@Z$jN%$9(B */
	if (g_isLeftSearchBefore) {
		gpstSearchBefore->pLeft = NULL;
	} else {
		gpstSearchBefore->pRight = NULL;
	}

	/* $B%^%k%A%9%l%C%I;~$O(B $B%5!<%A3+;O$+$i$3$3$^$GGSB>$9$Y$-(B */


	pstWorkLeft = pstWork->pLeft;
	pstWorkRight = pstWork->pRight;

	free( pstWork );
	pstWork = NULL;

	gnCntTree --;

//puts("ref in del");
//RefAllTree();

	/* $B0J2<(B $B;R$NMWAG$r:F9=@.(B */

	if (pstWorkLeft) {
		if (!AddTreeImp( pstWorkLeft )) {
			return FALSE;
		}
	}

	if (pstWorkRight) {
		if (!AddTreeImp( pstWorkRight )) {
			return FALSE;
		}
	}

	return TRUE;
}

/**
 * $B8!:w(B
 */
ST_TREE *SearchTree( uint16_t nIdx )
{
	ST_TREE *pstWork = gpstTreeRoot;
	ST_TREE *pstRtn = NULL;

	/* root$B$,6u(B */
	if (!pstWork) {
		return NULL;
	}

	while (1) {
		if (pstWork->nIdx > nIdx) {
			if (!(pstWork->pLeft)) {
				pstRtn = NULL;
				break;
			} else {
				/* $BB39T(B */

				/* 1$B$DA0$rJ]B8(B */
				gpstSearchBefore = pstWork;
				g_isLeftSearchBefore = TRUE;

				pstWork = pstWork->pLeft;
			}
		} else if (pstWork->nIdx < nIdx) {
			if (!(pstWork->pRight)) {
				pstRtn = NULL;
				break;
			} else {
				/* $BB39T(B */

				/* 1$B$DA0$rJ]B8(B */
				gpstSearchBefore = pstWork;
				g_isLeftSearchBefore = FALSE;

				pstWork = pstWork->pRight;
			}
		} else {
			/* index$B$,0lCW$7$?(B */
			pstRtn = pstWork;
			break;
		}
	}

	return pstRtn;
}

/**
 * $B;2>H(B
 * $B0z?t$G;XDj$7$?MWAG$H$=$N;R0J2<$r;2>H$9$k(B
 */
static void RefTree( ST_TREE *pstArg )
{
	if (!pstArg) {
		return;
	}

	LOG_I (
		"addr:[%p] nIdx:[%d] pLeft:[%p] pRight:[%p] data:[%s]\n",
		pstArg,
		pstArg->nIdx,
		pstArg->pLeft,
		pstArg->pRight,
		pstArg->stInfo.szData
	);

	if (pstArg->pLeft) {
		RefTree( pstArg->pLeft );
	}

	if (pstArg->pRight) {
		RefTree( pstArg->pRight );
	}
}

/**
 * $BA4%G!<%?;2>H(B
 */
void RefAllTree( void )
{
	RefTree( gpstTreeRoot );
}

/**
 * $B%G!<%?GK4~(B
 * $B0z?t$G;XDj$7$?MWAG$H$=$N;R0J2<$rGK4~$9$k(B
 */
static void DestroyTree( ST_TREE *pstArg )
{
	if (!pstArg) {
		return;
	}

	LOG_I( "free(%p)\n", pstArg );

	if (pstArg->pLeft) {
		DestroyTree( pstArg->pLeft );
	}

	if (pstArg->pRight) {
		DestroyTree( pstArg->pRight );
	}

	free( pstArg );
	pstArg = NULL;
}

/**
 * $BA4%G!<%?GK4~(B
 */
void DestroyAllTree( void )
{
	DestroyTree( gpstTreeRoot );
	gpstTreeRoot = NULL;
	gnCntTree = 0;
}

/**
 * Index$B@8@.(B
 * 0$B!A(B65535 $B$N%i%s%@%`(B?$B$JCM$rJV$9(B
 * ($B$H$$$&$3$H$O(B $B$3$N(Btree$B9=B$$N%G!<%?$O:GBg(B65536$B8D(B)
 * IP$B%A%'%C%/%5%`$r;29M(B
 */
static uint16_t GenIdx( const uint16_t *pArg, size_t nArgSize )
{
	uint32_t nIdx = 0;

	while (nArgSize > 1) {
		nIdx += *pArg;

		if (nIdx & 0x10000) {
			/* 16bit$BL\$,N)$C$F$$$?$i%*!<%P!<%U%m!<J,$rB-$9(B */
			nIdx = ( nIdx >> 16 ) + ( nIdx & 0xffff );
		}

		nArgSize -= 2;
		pArg ++;
	}

	/* $BM>$j$N(B1byte$BJ,(B */
	if (nArgSize == 1) {
		nIdx += *pArg;

		if (nIdx & 0x10000) {
			/* 16bit$BL\$,N)$C$F$$$?$i%*!<%P!<%U%m!<J,$rB-$9(B */
			nIdx = ( nIdx >> 16 ) + ( nIdx & 0xffff );
		}
	}

	return (uint16_t)nIdx;
}

int main( void )
{
	ST_INFO stInfo;

	InitTree();

	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "aaa" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "bbb" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "ccc" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "aaa" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "egehhryjyrj" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "234346576uhy5g" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "dfergk4k-5v,.aa" );
	AddTree( &stInfo, sizeof(stInfo) );

	RefAllTree();

	DelTree( 25284 );

puts("del idx:25284 after");

	RefAllTree();

	DestroyAllTree();

puts("destroy after");

	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "eee" );
	AddTree( &stInfo, sizeof(stInfo) );
	memset( &stInfo, 0x00, sizeof(stInfo) );
	strcpy( stInfo.szData, "ggg" );
	AddTree( &stInfo, sizeof(stInfo) );

	RefAllTree();



	exit( EXIT_SUCCESS );
}
