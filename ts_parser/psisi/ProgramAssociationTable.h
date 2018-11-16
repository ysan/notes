#ifndef _PROGRAM_ASSOCIATION_TABLE_H_
#define _PROGRAM_ASSOCIATION_TABLE_H_

#include "Defs.h"
#include "TsCommonDefs.h"
#include "SectionParser.h"
#include "ProgramMapTable.h"


typedef struct _pat_element {
public:
	_pat_element (void)
		:program_number (0)
		,network_PID (0)
		,program_map_PID (0)
		,mpPMT (NULL)
		,isUsed (false)
	{}
	~_pat_element (void) {}

public:
	uint16_t program_number;
	uint16_t network_PID; // if program_number == 0 then network_PID
	uint16_t program_map_PID;
	CProgramMapTable *mpPMT; // section parser for PMT
	bool isUsed;

} ST_PAT_ELEMENT;


class CProgramAssociationTable : public CSectionParser
{
public:
	CProgramAssociationTable (void);
	virtual ~CProgramAssociationTable (void);


	uint16_t getElementNum () const;
	bool getElement (ST_PAT_ELEMENT *pstOut, uint16_t nOutSetNum) const;
	void dumpElement (const ST_PAT_ELEMENT *pstIn, uint16_t nInSetNum) const;


private:
	uint16_t getElementNum (const CSectionInfo *pSectInfo) const;

};

#endif
