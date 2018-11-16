#ifndef _PROGRAM_MAP_TABLE_H_
#define _PROGRAM_MAP_TABLE_H_

#include "Defs.h"
#include "TsCommonDefs.h"
#include "SectionParser.h"


class CProgramMapTable : public CSectionParser
{
public:
	CProgramMapTable (void);
	virtual ~CProgramMapTable (void);


	void onSectionComplete (const CSectionInfo *pCompSection) override;


	void dump (void) const;

private:

};

#endif
