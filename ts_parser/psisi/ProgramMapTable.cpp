#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ProgramMapTable.h"
#include "Utils.h"


CProgramMapTable::CProgramMapTable (void)
{
}

CProgramMapTable::~CProgramMapTable (void)
{
}


void CProgramMapTable::onSectionComplete (const CSectionInfo *pCompSection)
{
	if (!pCompSection) {
		return;
	}

	CTable *pTable = new CTable ();
	if (!parse (pCompSection, pTable)) {
		delete pTable;
		pTable = NULL;
		detachSectionList (pCompSection);
		return ;
	}

	mTables.push_back (pTable);
	dumpTable (pTable);

}

bool CProgramMapTable::parse (const CSectionInfo *pCompSection, CTable* pOutTable)
{
	if (!pCompSection || !pOutTable) {
		return false;
	}

	uint8_t *p = NULL; // work
	CTable* pTable = pOutTable;

	pTable->header = *(const_cast<CSectionInfo*>(pCompSection)->getHeader());

	p = pCompSection->getDataPartAddr();
	pTable->reserved_3 = (*p >> 5) & 0x07;
	pTable->PCR_PID = (*p & 0x1f) << 8 | *(p+1);
	pTable->reserved_4 = (*(p+2) >> 4) & 0x0f;
	pTable->program_info_length = (*(p+2) & 0x0f) << 8 | *(p+3);

	p += PMT_FIX_LEN;

	int n = (int)pTable->program_info_length;
	while (n > 0) {
		CDescriptor desc (p);
//desc.dump();
		if (!desc.isValid) {
			puts ("invalid desc 1");
			return false;
		}
		pTable->descriptors.push_back (desc);
		n -= (2 + *(p + 1));
		p += (2 + *(p + 1));
	}

	int streamLen = (int) (pTable->header.section_length - pTable->program_info_length - SECTION_HEADER_FIX_LEN - SECTION_CRC32_LEN - PMT_FIX_LEN);
	if (streamLen <= PMT_STREAM_FIX_LEN) {
		puts ("invalid PMT stream");
		return false;
	}

	while (streamLen > 0) {

		CTable::CStream strm ;

		strm.stream_type = *p;
		strm.reserved_1 = (*(p+1) >> 5) & 0x07;
		strm.elementary_PID = (*(p+1) & 0x1f) << 8 | *(p+2);
		strm.reserved_2 = (*(p+3) >> 4) & 0x0f;
		strm.ES_info_length = (*(p+3) & 0x0f) << 8 | *(p+4);

		p += PMT_STREAM_FIX_LEN;

		int n = (int)strm.ES_info_length ;
		while (n > 0) {
			CDescriptor desc (p);
			if (!desc.isValid) {
				puts ("invalid desc 2");
				return false;
			}
			strm.descriptors.push_back (desc);
			n -= (2 + *(p + 1));
			p += (2 + *(p + 1));
		}

		streamLen -= (PMT_STREAM_FIX_LEN + strm.ES_info_length) ;
		if (streamLen < 0) {
			puts ("invalid PMT stream");
			return false;
		}

		pTable->streams.push_back (strm);
	}

	return true;
}

void CProgramMapTable::releaseTables (void)
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

void CProgramMapTable::dumpTables (void) const
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

void CProgramMapTable::dumpTable (const CTable* pTable) const
{
	if (!pTable) {
		return;
	}

	printf ("========================================\n");

	printf ("PCR_PID             [0x%04x]\n", pTable->PCR_PID);
	printf ("program_info_length [%d]\n", pTable->program_info_length);

	std::vector<CDescriptor>::const_iterator iter_desc = pTable->descriptors.begin();
	for (; iter_desc != pTable->descriptors.end(); ++ iter_desc) {
		printf ("\n--  descriptor  --\n");
		CDescriptorCommon::dump (iter_desc->tag, *iter_desc);
	}

	std::vector<CTable::CStream>::const_iterator iter_strm = pTable->streams.begin();
	for (; iter_strm != pTable->streams.end(); ++ iter_strm) {
		printf ("\n--  stream  --\n");
		printf ("stream_type    [0x%02x]\n", iter_strm->stream_type);
		printf ("elementary_PID [0x%04x]\n", iter_strm->elementary_PID);
		printf ("ES_info_length [%d]\n", iter_strm->ES_info_length);

		std::vector<CDescriptor>::const_iterator iter_desc = iter_strm->descriptors.begin();
		for (; iter_desc != iter_strm->descriptors.end(); ++ iter_desc) {
			printf ("\n--  descriptor  --\n");
			CDescriptorCommon::dump (iter_desc->tag, *iter_desc);
		}
	}

	printf ("========================================\n");
}

void CProgramMapTable:: clear (void)
{
	releaseTables ();
	detachAllSectionList ();
}
