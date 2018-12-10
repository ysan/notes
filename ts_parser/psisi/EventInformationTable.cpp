#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "EventInformationTable.h"
#include "Utils.h"


CEventInformationTable::CEventInformationTable (size_t poolSize)
	:CSectionParser (poolSize)
{
}

CEventInformationTable::~CEventInformationTable (void)
{
}

void CEventInformationTable::onSectionComplete (const CSectionInfo *pCompSection)
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

bool CEventInformationTable::parse (const CSectionInfo *pCompSection, CTable* pOutTable)
{
	if (!pCompSection || !pOutTable) {
		return false;
	}

	uint8_t *p = NULL; // work
	CTable* pTable = pOutTable;

	pTable->header = *(const_cast<CSectionInfo*>(pCompSection)->getHeader());

	p = pCompSection->getDataPartAddr();
	pTable->transport_stream_id = *p << 8 | *(p+1);
	pTable->original_network_id = *(p+2) << 8 | *(p+3);
	pTable->segment_last_section_number = *(p+4);
	pTable->last_table_id = *(p+5);

	p += EIT_FIX_LEN;

	int eventLen = (int) (pTable->header.section_length - SECTION_HEADER_FIX_LEN - SECTION_CRC32_LEN - EIT_FIX_LEN);
	if (eventLen <= EIT_EVENT_FIX_LEN) {
		puts ("invalid EIT event");
		return false;
	}

	while (eventLen > 0) {

		CTable::CEvent ev ;

		ev.event_id = *p << 8 | *(p+1);
		memcpy (ev.start_time, p+2, 5);
		memcpy (ev.duration, p+7, 3);
		ev.running_status = (*(p+10) >> 5) & 0x07;
		ev.free_CA_mode = (*(p+10) >> 4) & 0x01;
		ev.descriptors_loop_length = (*(p+10) & 0x0f) << 8 | *(p+11);

		p += EIT_EVENT_FIX_LEN;
		int n = (int)ev.descriptors_loop_length;
		while (n > 0) {
			CDescriptor desc (p);
			if (!desc.isValid) {
				puts ("invalid desc");
				return false;
			}
			ev.descriptors.push_back (desc);
			n -= (2 + *(p + 1));
			p += (2 + *(p + 1));
		}

		eventLen -= (EIT_EVENT_FIX_LEN + ev.descriptors_loop_length) ;
		if (eventLen < 0) {
			puts ("invalid EIT event");
			return false;
		}

		pTable->events.push_back (ev);
	}

	return true;
}

void CEventInformationTable::releaseTables (void)
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

void CEventInformationTable::dumpTables (void) const
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

void CEventInformationTable::dumpTable (const CTable* pTable) const
{
	if (!pTable) {
		return;
	}
	
	printf ("========================================\n");

	printf ("table_id                    [0x%02x]\n", pTable->header.table_id);
	printf ("transport_stream_id         [0x%04x]\n", pTable->transport_stream_id);
	printf ("original_network_id         [0x%04x]\n", pTable->original_network_id);
	printf ("segment_last_section_number [0x%02x]\n", pTable->segment_last_section_number);
	printf ("original_network_id         [0x%02x]\n", pTable->last_table_id);

	std::vector<CTable::CEvent>::const_iterator iter_event = pTable->events.begin();
	for (; iter_event != pTable->events.end(); ++ iter_event) {
		printf ("\n--  event  --\n");
		printf ("event_id                [0x%04x]\n", iter_event->event_id);
		char szStime [32];
		memset (szStime, 0x00, sizeof(szStime));
		CTsCommon::getStrEpoch (CTsCommon::getEpochFromMJD (iter_event->start_time), "%Y/%m/%d %H:%M:%S", szStime, sizeof(szStime));
		printf ("start_time              [%s]\n", szStime);
		char szDuration [32];
		memset (szDuration, 0x00, sizeof(szDuration));
		CTsCommon::getStrSecond (CTsCommon::getSecFromBCD (iter_event->duration), szDuration, sizeof(szDuration));
		printf ("duration                [%s]\n", szDuration);
		printf ("running_status          [0x%02x]\n", iter_event->running_status);
		printf ("free_CA_mode            [0x%02x]\n", iter_event->free_CA_mode);
		printf ("descriptors_loop_length [0x%04x]\n", iter_event->descriptors_loop_length);

		std::vector<CDescriptor>::const_iterator iter_desc = iter_event->descriptors.begin();
		for (; iter_desc != iter_event->descriptors.end(); ++ iter_desc) {
			printf ("\n--  descriptor  --\n");
			switch (iter_desc->tag) {
			case DESC_TAG__SHORT_EVENT_DESCRIPTOR:
				{
					CShortEventDescriptor sed (*iter_desc);
					if (sed.isValid) {
						sed.dump();
					} else {
						printf ("invalid ShortEventDescriptor\n");
					}
				}
				break;

			case DESC_TAG__EXTENDED_EVENT_DESCRIPTOR:
				{
					CExtendedEventDescriptor eed (*iter_desc);
					if (eed.isValid) {
						eed.dump();
					} else {
						printf ("invalid ExtendedEventDescriptor\n");
					}
				}
				break;

			case DESC_TAG__COMPONENT_DESCRIPTOR:
				{
					CComponentDescriptor cd (*iter_desc);
					if (cd.isValid) {
						cd.dump();
					} else {
						printf ("invalid ComponentDescriptor\n");
					}
				}
				break;

			case DESC_TAG__AUDIO_COMPONENT_DESCRIPTOR:
				{
					CAudioComponentDescriptor acd (*iter_desc);
					if (acd.isValid) {
						acd.dump();
					} else {
						printf ("invalid AudioComponentDescriptor\n");
					}
				}
				break;

			case DESC_TAG__SERIES_DESCRIPTOR:
				{
					CSeriesDescriptor sd (*iter_desc);
					if (sd.isValid) {
						sd.dump();
					} else {
						printf ("invalid SeriesDescriptor\n");
					}
				}
				break;

			case DESC_TAG__CONTENT_DESCRIPTOR:
				{
					CContentDescriptor cd (*iter_desc);
					if (cd.isValid) {
						cd.dump();
					} else {
						printf ("invalid ContentDescriptor\n");
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

void CEventInformationTable::clear (void)
{
	releaseTables ();
	detachAllSectionList ();
}
