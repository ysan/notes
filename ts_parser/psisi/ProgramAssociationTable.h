#ifndef _PROGRAM_ASSOCIATION_TABLE_H_
#define _PROGRAM_ASSOCIATION_TABLE_H_

#include "Defs.h"
#include "TsCommonDefs.h"
#include "SectionParser.h"
#include "ProgramMapTable.h"



class CProgramAssociationTable : public CSectionParser
{
public:
	class CElement {
	public:
		CElement (void)
			:program_number (0)
			,network_PID (0)
			,program_map_PID (0)
			,mpPMT (NULL)
			,isUsed (false)
		{}
		virtual ~CElement (void) {}

		uint16_t program_number;
		uint16_t network_PID; // if program_number == 0 then network_PID
		uint16_t program_map_PID;
		CProgramMapTable *mpPMT; // section parser for PMT
		bool isUsed;
	};

public:
	CProgramAssociationTable (void);
	virtual ~CProgramAssociationTable (void);


	int getElementNum (void) const;
	bool getElement (CElement outArr[], int outArrSize) const;
	void dumpElement (const CElement inArr[], int arrSize) const;


private:
	int getElementNum (const CSectionInfo *pSectInfo) const;

};

#endif
