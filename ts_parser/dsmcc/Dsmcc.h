#ifndef _DSMCC_H_
#define _DSMCC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <vector>

#include "Defs.h"
#include "TsCommon.h"
#include "SectionParser.h"
#include "DescriptorDefs.h"


//#define BIT_FIX_LEN				(2) // reserved_future_use        3  bslbf
									// broadcast_view_propriety   1  uimsbf
									// first_descriptors_length  12  uimsbf

//#define BIT_BROADCASTER_FIX_LEN	(3) // broadcaster_id                   8  uimsbf
									// reserved_future_use              4  bslbf
									// broadcaster_descriptors_length  12  uimsbf


class CDsmcc : public CSectionParser
{
public:
	class CTable {
	public:
		CTable (void) {};
		virtual ~CTable (void) {};


	};

public:
	explicit CDsmcc (size_t poolSize);
	virtual ~CDsmcc (void);

	void onSectionComplete (const CSectionInfo *pCompSection) override;

	void dumpTables (void) const;
	void dumpTable (const CTable* pTable) const;
	void clear (void);

private:
	bool parse (const CSectionInfo *pCompSection, CTable* pOutTable);
	void releaseTables (void);

	std::vector <CTable*> mTables;
};

#endif
