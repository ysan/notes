#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ServiceDescriptionTable.h"
#include "Utils.h"


CServiceDescriptionTable::CServiceDescriptionTable (void)
{
}

CServiceDescriptionTable::~CServiceDescriptionTable (void)
{
}

void CServiceDescriptionTable::onSectionComplete (const CSectionInfo *pCompSection)
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

bool CServiceDescriptionTable::parse (const CSectionInfo *pCompSection, CTable* pOutTable)
{
	if (!pCompSection || !pOutTable) {
		return false;
	}

	uint8_t *p = NULL; // work
	CTable* pTable = pOutTable;

	pTable->header = *(const_cast<CSectionInfo*>(pCompSection)->getHeader());

	p = pCompSection->getDataPartAddr();
	pTable->original_network_id = *p << 8 | *(p+1);
	pTable->reserved_future_use_2 = *(p+2);

	p += SDT_FIX_LEN;

	int serviceLen = (int) (pTable->header.section_length - SECTION_HEADER_FIX_LEN - SECTION_CRC32_LEN - SDT_FIX_LEN);
	if (serviceLen <= SDT_SERVICE_FIX_LEN) {
		puts ("invalid SDT service");
		return false;
	}

	while (serviceLen > 0) {

		CTable::CService svc ;

		svc.service_id = *p << 8 | *(p+1);
		svc.reserved_future_use = *(p+2) & 0x07;
		svc.EIT_user_defined_flags = (*(p+2) >> 2) & 0x07;
		svc.EIT_schedule_flag = (*(p+2) >> 1) & 0x01;
		svc.EIT_present_following_flag = *(p+2) & 0x01;
		svc.running_status = (*(p+3) >> 5) & 0x07;
		svc.free_CA_mode = (*(p+3) >> 4) & 0x01;
		svc.descriptors_loop_length = (*(p+3) & 0x0f) << 8 | *(p+4);

		p += SDT_SERVICE_FIX_LEN;
		int n = (int)svc.descriptors_loop_length;
		while (n > 0) {
			CDescriptor desc (p);
			if (!desc.isValid) {
				puts ("invalid desc");
				return false;
			}
			svc.descriptors.push_back (desc);
			n -= (2 + *(p + 1));
			p += (2 + *(p + 1));
		}

		serviceLen -= (SDT_SERVICE_FIX_LEN + svc.descriptors_loop_length) ;
		if (serviceLen < 0) {
			puts ("invalid SDT stream");
			return false;
		}

		pTable->services.push_back (svc);
	}

	return true;
}

void CServiceDescriptionTable::releaseTables (void)
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

void CServiceDescriptionTable::dumpTables (void) const
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

void CServiceDescriptionTable::dumpTable (const CTable* pTable) const
{
	if (!pTable) {
		return;
	}
	
	printf ("========================================\n");

	printf ("original_network_id       [0x%04x]\n", pTable->original_network_id);

	std::vector<CTable::CService>::const_iterator iter_svc = pTable->services.begin();
	for (; iter_svc != pTable->services.end(); ++ iter_svc) {
		printf ("\n--  service  --\n");
		printf ("service_id                 [0x%04x]\n", iter_svc->service_id);
		printf ("EIT_user_defined_flags     [0x%01x]\n", iter_svc->EIT_user_defined_flags);
		printf ("EIT_schedule_flag          [0x%01x]\n", iter_svc->EIT_schedule_flag);
		printf ("EIT_present_following_flag [0x%01x]\n", iter_svc->EIT_present_following_flag);
		printf ("running_status             [0x%01x]\n", iter_svc->running_status);
		printf ("free_CA_mode               [0x%01x]\n", iter_svc->free_CA_mode);
		printf ("descriptors_loop_length    [0x%04x]\n", iter_svc->descriptors_loop_length);

		printf ("\n--  descriptors  --\n");
		std::vector<CDescriptor>::const_iterator iter_desc = iter_svc->descriptors.begin();
		for (; iter_desc != iter_svc->descriptors.end(); ++ iter_desc) {
			iter_desc->dump();
		}
	}

	printf ("========================================\n");
}

void CServiceDescriptionTable::clear (void)
{
	releaseTables ();
	detachAllSectionList ();
}
