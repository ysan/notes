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

	CElement *pElem = new CElement ();
	if (!parse (pCompSection, pElem)) {
		delete pElem;
		pElem = NULL;
	}

	mElements.push_back (pElem);
	dumpElement (pElem);

}

bool CEventInformationTable::parse (const CSectionInfo *pCompSection, CElement* pOutElem)
{
	if (!pCompSection || !pOutElem) {
		return false;
	}

	uint8_t *p = NULL; // work
	CElement* pElem = pOutElem;

	p = pCompSection->getDataPartAddr();
	pElem->transport_stream_id = *p << 8 | *(p+1);
	pElem->original_network_id = *(p+2) << 8 | *(p+3);
	pElem->segment_last_section_number = *(p+4);
	pElem->last_table_id = *(p+5);

	p += EIT_FIX_LEN;

	int eventLen = (int) (const_cast<CSectionInfo*>(pCompSection)->getHeader()->section_length - SECTION_HEADER_FIX_LEN - SECTION_CRC32_LEN - EIT_FIX_LEN);
	if (eventLen <= EIT_EVENT_FIX_LEN) {
		puts ("invalid EIT event");
		return false;
	}

	while (eventLen > 0) {

		CElement::CEvent ev ;

		ev.event_id = *p << 8 | *(p+1);
		ev.start_time = ((uint64_t)(*(p+2)) << 32) | *(p+3) << 24 | *(p+4) << 16 | *(p+5) << 8 | *(p+6);
		ev.duration = *(p+7) << 16 | *(p+8) << 8 | *(p+9);
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

		pElem->events.push_back (ev);
	}

	return true;
}

void CEventInformationTable::releaseElements (void)
{
	if (mElements.size() == 0) {
		return;
	}

	std::vector<CElement*>::iterator iter = mElements.begin(); 
	for (; iter != mElements.end(); ++ iter) {
		delete (*iter);
		(*iter) = NULL;
	}

	mElements.clear();
}

void CEventInformationTable::dumpElements (void) const
{
	if (mElements.size() == 0) {
		return;
	}

	std::vector<CElement*>::const_iterator iter = mElements.begin(); 
	for (; iter != mElements.end(); ++ iter) {
		CElement *pElem = *iter;
		dumpElement (pElem);
	}
}

void CEventInformationTable::dumpElement (const CElement* pElem) const
{
	if (!pElem) {
		return;
	}
	
	printf ("========================================\n");

	printf ("transport_stream_id         0x%04x\n", pElem->transport_stream_id);
	printf ("original_network_id         0x%04x\n", pElem->original_network_id);
	printf ("segment_last_section_number 0x%02x\n", pElem->segment_last_section_number);
	printf ("original_network_id         0x%02x\n", pElem->last_table_id);

	std::vector<CElement::CEvent>::const_iterator iter_event = pElem->events.begin();
	for (; iter_event != pElem->events.end(); ++ iter_event) {
		printf ("\n--  events  --\n");
		printf ("event_id                0x%04x\n", iter_event->event_id);
		printf ("start_time              0x%02x%08x\n", (uint32_t)(iter_event->start_time >> 32) & 0xff, (uint32_t)iter_event->start_time & 0xffffffff);
		printf ("duration                0x%06x\n", iter_event->duration);
		printf ("running_status          0x%02x\n", iter_event->running_status);
		printf ("free_CA_mode            0x%02x\n", iter_event->free_CA_mode);
		printf ("descriptors_loop_length 0x%04x\n", iter_event->descriptors_loop_length);

		printf ("\n-- descriptors --\n");
		std::vector<CDescriptor>::const_iterator iter_desc = iter_event->descriptors.begin();
		for (; iter_desc != iter_event->descriptors.end(); ++ iter_desc) {
			iter_desc->dump();
		}
	}

	printf ("========================================\n");
}

void CEventInformationTable::clear (void)
{
//	releaseElements ();
	detachAllSection ();
}
