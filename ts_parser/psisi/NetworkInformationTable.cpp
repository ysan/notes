#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "NetworkInformationTable.h"
#include "Utils.h"


CNetworkInformationTable::CNetworkInformationTable (void)
{
}

CNetworkInformationTable::~CNetworkInformationTable (void)
{
}

void CNetworkInformationTable::onSectionComplete (const CSectionInfo *pCompSection)
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

bool CNetworkInformationTable::parse (const CSectionInfo *pCompSection, CTable* pOutTable)
{
	if (!pCompSection || !pOutTable) {
		return false;
	}

	uint8_t *p = NULL; // work
	CTable* pTable = pOutTable;

	pTable->header = *(const_cast<CSectionInfo*>(pCompSection)->getHeader());

	p = pCompSection->getDataPartAddr();
	pTable->reserved_future_use_2 = (*p >> 4) & 0xf;
	pTable->network_descriptors_length = (*p & 0xf) << 8 | *(p+1);

	p += NIT_FIX_LEN;
	int n = (int)pTable->network_descriptors_length;
	while (n > 0) {
		CDescriptor desc (p);
		if (!desc.isValid) {
			puts ("invalid desc");
			return false;
		}
		pTable->descriptors.push_back (desc);
		n -= (2 + *(p + 1));
		p += (2 + *(p + 1));
	}

	pTable->reserved_future_use_3 = (*p >> 4) & 0xf;
	pTable->transport_stream_loop_length = (*p & 0xf) << 8 | *(p+1);

	p += NIT_FIX2_LEN;

	int streamLen = (int) (pTable->header.section_length - SECTION_HEADER_FIX_LEN - SECTION_CRC32_LEN - NIT_FIX_LEN - pTable->network_descriptors_length - NIT_FIX2_LEN);
	if (streamLen <= NIT_STREAM_FIX_LEN) {
		puts ("invalid NIT stream");
		return false;
	}

	if (streamLen != pTable->transport_stream_loop_length) {
		puts ("invalid NIT stream");
		return false;
	}

	while (streamLen > 0) {

		CTable::CStream strm ;

		strm.transport_stream_id = *p << 8 | *(p+1);
		strm.original_network_id = (*(p+2) << 8) | *(p+3);
		strm.reserved_future_use = (*(p+4) >> 4) & 0x0f;
		strm.transport_descriptors_length = (*(p+4) & 0xf) << 8 | *(p+5);

		p += NIT_STREAM_FIX_LEN;
		int n = (int)strm.transport_descriptors_length;
		while (n > 0) {
			CDescriptor desc (p);
			if (!desc.isValid) {
				puts ("invalid desc");
				return false;
			}
			strm.descriptors.push_back (desc);
			n -= (2 + *(p + 1));
			p += (2 + *(p + 1));
		}

		streamLen -= (NIT_STREAM_FIX_LEN + strm.transport_descriptors_length) ;
		if (streamLen < 0) {
			puts ("invalid NIT stream");
			return false;
		}

		pTable->streams.push_back (strm);
	}

	return true;
}

void CNetworkInformationTable::releaseTables (void)
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

void CNetworkInformationTable::dumpTables (void) const
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

void CNetworkInformationTable::dumpTable (const CTable* pTable) const
{
	if (!pTable) {
		return;
	}
	
	printf ("========================================\n");

	printf ("table_id                         [0x%02x]\n", pTable->header.table_id);
	printf ("network_descriptors_length       [0x%04x]\n", pTable->network_descriptors_length);

	std::vector<CDescriptor>::const_iterator iter_desc = pTable->descriptors.begin();
	for (; iter_desc != pTable->descriptors.end(); ++ iter_desc) {
		printf ("\n--  descriptor  --\n");
		iter_desc->dump();
	}

	printf ("transport_stream_loop_length     [0x%04x]\n", pTable->transport_stream_loop_length);

	std::vector<CTable::CStream>::const_iterator iter_strm = pTable->streams.begin();
	for (; iter_strm != pTable->streams.end(); ++ iter_strm) {
		printf ("\n--  stream  --\n");
		printf ("transport_stream_id          [0x%04x]\n", iter_strm->transport_stream_id);
		printf ("original_network_id          [0x%04x]\n", iter_strm->original_network_id);
		printf ("transport_descriptors_length [0x%04x]\n", iter_strm->transport_descriptors_length);

		std::vector<CDescriptor>::const_iterator iter_desc = iter_strm->descriptors.begin();
		for (; iter_desc != iter_strm->descriptors.end(); ++ iter_desc) {
			printf ("\n--  descriptor  --\n");
			switch (iter_desc->tag) {
			case DESC_TAG__SERVICE_LIST_DESCRIPTOR:
				{
					CServiceListDescriptor sld (*iter_desc);
					if (sld.isValid) {
						sld.dump();
					} else {
						printf ("invalid ServiceListDescriptor\n");
					}
				}
				break;

			case DESC_TAG__SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR:
				{
					CSatelliteDeliverySystemDescriptor sdsd (*iter_desc);
					if (sdsd.isValid) {
						sdsd.dump();
					} else {
						printf ("invalid SatelliteDeliverySystemDescriptor\n");
					}
				}
				break;

			case DESC_TAG__TERRESTRIAL_DELIVERY_SYSTEM_DESCRIPTOR:
				{
					CTerrestrialDeliverySystemDescriptor tdsd (*iter_desc);
					if (tdsd.isValid) {
						tdsd.dump();
					} else {
						printf ("invalid TerrestrialDeliverySystemDescriptor\n");
					}
				}
				break;

			case DESC_TAG__TS_INFORMATION_DESCRIPTOR:
				{
					CTSInformationDescriptor tsid (*iter_desc);
					if (tsid.isValid) {
						tsid.dump();
					} else {
						printf ("invalid TSInformationDescriptor\n");
					}
				}
				break;

			default:
				iter_desc->dump();
				break;
			}
		}
	}

	printf ("========================================\n");
}

void CNetworkInformationTable::clear (void)
{
	releaseTables ();
	detachAllSectionList ();
}
