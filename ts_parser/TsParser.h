#ifndef _TS_PARSER_H_
#define _TS_PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "Defs.h"
#include "TsCommon.h"
#include "ProgramAssociationTable.h"
#include "ProgramMapTable.h"
#include "TimeOffsetTable.h"
#include "EventInformationTable.h"
#include "NetworkInformationTable.h"
#include "ServiceDescriptionTable.h"
#include "RunningStatusTable.h"
#include "BroadcasterInformationTable.h"
#include "DSMCC.h"

class CDsmccControl {
public:
	CDsmccControl (void)
		:pid (0)
		,mpDSMCC (NULL)
		,isUsed (false)
	{}
	virtual ~CDsmccControl (void) {
		if (mpDSMCC) {
			delete mpDSMCC;
			mpDSMCC = NULL;
		}
	}

	uint16_t pid;
	CDSMCC *mpDSMCC;
	bool isUsed;
};

class CTsParser
{
public:
	CTsParser (void);
	virtual ~CTsParser (void);

	void run (uint8_t *pBuff, size_t nSize);

private:
	bool copyInnerBuffer (uint8_t *pBuff, size_t nSize);
	bool checkUnitSize (void);
	uint8_t *getSyncTopAddr (uint8_t *pTop, uint8_t *pBtm, size_t nUnitSize) const;
	void getTsHeader (ST_TS_HEADER *pDst, uint8_t *pSrc) const;
	void dumpTsHeader (const ST_TS_HEADER *p) const;

	bool parse (void);


	uint8_t *mpTop ;
	uint8_t *mpCurrent ;
	uint8_t *mpBottom ;
	size_t mBuffSize ;

	int mUnitSize;
	int mParseRemainSize;


	CProgramAssociationTable mPAT;
	CProgramAssociationTable::CTable mPatTables [4 * 8];
	CTimeOffsetTable mTOT;
	CEventInformationTable mEIT_H;
	CEventInformationTable mEIT_M;
	CEventInformationTable mEIT_L;
	CNetworkInformationTable mNIT;
	CServiceDescriptionTable mSDT;
	CRunningStatusTable mRST;
	CBroadcasterInformationTable mBIT;

	CDsmccControl mDsmccCtls [256];
};

#endif
