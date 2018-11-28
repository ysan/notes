#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "RunningStatusTable.h"
#include "Utils.h"


CRunningStatusTable::CRunningStatusTable (void)
{
}

CRunningStatusTable::~CRunningStatusTable (void)
{
}

void CRunningStatusTable::onSectionComplete (const CSectionInfo *pCompSection)
{
	if (!pCompSection) {
		return ;
	}

	CTable *pTable = new CTable ();
	if (!parse (pCompSection, pTable)) {
		delete pTable;
		pTable = NULL;
	}

	mTables.push_back (pTable);
	dumpTable (pTable);

}

bool CRunningStatusTable::parse (const CSectionInfo *pCompSection, CTable* pOutTable)
{
	if (!pCompSection || !pOutTable) {
		return false;
	}

	uint8_t *p = NULL; // work
	CTable* pTable = pOutTable;

	pTable->header = *(const_cast<CSectionInfo*>(pCompSection)->getHeader());

	p = pCompSection->getDataPartAddr();

	int statusLen = (int) pTable->header.section_length ;
	if (statusLen <= RST_STATUS_FIX_LEN) {
		puts ("invalid RST");
		return false;
	}

	while (statusLen > 0) {

		CTable::CStatus stt ;

		stt.transport_stream_id = *p << 8 | *(p+1);
		stt.original_network_id = *(p+2) << 8 | *(p+3);
		stt.service_id = *(p+4) << 8 | *(p+5);
		stt.event_id = *(p+6) << 8 | *(p+7);
		stt.running_status = *(p+8) & 0x7;

		p += RST_STATUS_FIX_LEN;

		statusLen -= RST_STATUS_FIX_LEN ;
		if (statusLen < 0) {
			puts ("invalid RST status");
			return false;
		}

		pTable->statuses.push_back (stt);
	}

	return true;
}

void CRunningStatusTable::releaseTables (void)
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

void CRunningStatusTable::dumpTables (void) const
{
	if (mTables.size() == 0) {
		return;
	}

	std::vector<CTable*>::const_iterator iter = mTables.begin(); 
	for (; iter != mTables.end(); ++ iter) {
		CTable *pTable = *iter;
		dumpTable (pTable);
	}
}

void CRunningStatusTable::dumpTable (const CTable* pTable) const
{
	if (!pTable) {
		return;
	}
	
	printf ("========================================\n");

	std::vector<CTable::CStatus>::const_iterator iter_stt = pTable->statuses.begin();
	for (; iter_stt != pTable->statuses.end(); ++ iter_stt) {
		printf ("\n--  statuses  --\n");
		printf ("transport_stream_id [0x%04x]\n", iter_stt->transport_stream_id);
		printf ("original_network_id [0x%04x]\n", iter_stt->original_network_id);
		printf ("service_id          [0x%04x]\n", iter_stt->service_id);
		printf ("event_id            [0x%04x]\n", iter_stt->event_id);
		printf ("running_status      [0x%01x]\n", iter_stt->running_status);
	}

	printf ("========================================\n");
}

void CRunningStatusTable::clear (void)
{
	releaseTables ();
	detachAllSectionList ();
}
