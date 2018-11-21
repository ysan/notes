#ifndef _TS_PARSER_H_
#define _TS_PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "Defs.h"
#include "TsCommonDefs.h"
#include "ProgramAssociationTable.h"
#include "ProgramMapTable.h"
#include "TimeOffsetTable.h"
#include "EventInformationTable.h"


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

	bool searchPAT (void);


	uint8_t *mpTop ;
	uint8_t *mpCurrent ;
	uint8_t *mpBottom ;
	size_t mBuffSize ;

	int mUnitSize;


	CProgramAssociationTable mPAT;
	CProgramAssociationTable::CElement mPatElement [4 * 8];
	CTimeOffsetTable mTOT;
	CEventInformationTable mEIT_0x12;
	CEventInformationTable mEIT_0x26;
	CEventInformationTable mEIT_0x27;

};

#endif
