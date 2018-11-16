#ifndef _EVENT_INFORMATION_TABLE_H_
#define _EVENT_INFORMATION_TABLE_H_

#include "Defs.h"
#include "TsCommonDefs.h"
#include "SectionParser.h"


class CEventInformationTable : public CSectionParser
{
public:
	explicit CEventInformationTable (size_t poolSize);
	virtual ~CEventInformationTable (void);

	void onSectionComplete (const CSectionInfo *pCompSection) override;

	void dump (void) const;
	void dump (const CSectionInfo *pSectInfo) const;

private:

};

#endif
