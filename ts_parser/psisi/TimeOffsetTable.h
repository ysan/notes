#ifndef _TIME_OFFSET_TABLE_H_
#define _TIME_OFFSET_TABLE_H_

#include "Defs.h"
#include "TsCommonDefs.h"
#include "SectionParser.h"



class CTimeOffsetTable : public CSectionParser
{
public:
	explicit CTimeOffsetTable (uint8_t fifoNum);
	virtual ~CTimeOffsetTable (void);

	void onSectionComplete (const CSectionInfo *pCompSection) override;

	void dump (void) const;
	void dump (const CSectionInfo *pSectInfo) const;

private:

};

#endif
