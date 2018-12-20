#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "DSMCC.h"
#include "Utils.h"


CDSMCC::CDSMCC (size_t poolSize)
	:CSectionParser (poolSize)
{
	mTables.clear();
}

CDSMCC::~CDSMCC (void)
{
}

void CDSMCC::onSectionComplete (const CSectionInfo *pCompSection)
{
	if (!pCompSection) {
		return ;
	}

//	CTable *pTable = new CTable ();
//	if (!parse (pCompSection, pTable)) {
//		delete pTable;
//		pTable = NULL;
//		detachSectionList (pCompSection);
//		return ;
//	}

//	mTables.push_back (pTable);
//	dumpTable (pTable);

}

bool CDSMCC::parse (const CSectionInfo *pCompSection, CTable* pOutTable)
{
	if (!pCompSection || !pOutTable) {
		return false;
	}



	return true;
}

void CDSMCC::releaseTables (void)
{
	if (mTables.size() == 0) {
		return;
	}

	std::vector<CTable*>::iterator iter = mTables.begin(); 
	for (; iter != mTables.end(); ++ iter) {
		delete (*iter);
		(*iter) = NULL;
	}

	mTables.clear();
}

void CDSMCC::dumpTables (void) const
{
	if (mTables.size() == 0) {
		return;
	}

//	std::vector<CTable*>::const_iterator iter = mTables.begin(); 
//	for (; iter != mTables.end(); ++ iter) {
//		CTable *pTable = *iter;
//		dumpTable (pTable);
//	}
}

void CDSMCC::dumpTable (const CTable* pTable) const
{
	if (!pTable) {
		return;
	}
	
	_UTL_LOG_I ("========================================\n");

	_UTL_LOG_I ("========================================\n");
}

void CDSMCC::clear (void)
{
	releaseTables ();
	detachAllSectionList ();
}
