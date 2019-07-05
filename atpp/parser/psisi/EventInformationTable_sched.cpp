#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <algorithm>

#include "EventInformationTable_sched.h"
#include "Utils.h"


bool _comp__service_id (const CEventInformationTable_sched::CTable* l, const CEventInformationTable_sched::CTable* r) {
	return r->header.table_id_extension > l->header.table_id_extension;
}

bool _comp__table_id (const CEventInformationTable_sched::CTable* l, const CEventInformationTable_sched::CTable* r) {
	if (r->header.table_id_extension == l->header.table_id_extension) {
		if (r->header.table_id > l->header.table_id) {
			return true;
		}
	}

	return false;
}

bool _comp__section_number (const CEventInformationTable_sched::CTable* l, const CEventInformationTable_sched::CTable* r) {
	if (r->header.table_id_extension == l->header.table_id_extension) {
		if (r->header.table_id == l->header.table_id) {
			if (r->header.section_number > l->header.section_number) {
				return true;
			}
		}
	}

	return false;
}


CEventInformationTable_sched::CEventInformationTable_sched (size_t poolSize)
	:CSectionParser (poolSize)
{
	mTables.clear();
}

CEventInformationTable_sched::CEventInformationTable_sched (size_t poolSize, int fifoNum)
	:CSectionParser (poolSize, fifoNum)
{
	mTables.clear();
}

CEventInformationTable_sched::CEventInformationTable_sched (size_t poolSize, int fifoNum, IEventScheduleHandler *p_hander)
	:CSectionParser (poolSize, fifoNum)
{
	mTables.clear();

	if (p_hander) {
		mpEventScheduleHandler = p_hander;
	}
}

CEventInformationTable_sched::~CEventInformationTable_sched (void)
{
	clear();
}


bool CEventInformationTable_sched::onSectionStarted (const CSectionInfo *pSection)
{
	if (!pSection) {
		return false;
	}


	// pf or schedule ?
	uint8_t _tbl_id = pSection->getHeader()->table_id;
	if (_tbl_id == TBLID_EIT_PF_A || _tbl_id == TBLID_EIT_PF_O) {
		// p/f  --> ignore
		return false;

	} else if (
		(_tbl_id >= TBLID_EIT_SCH_A && _tbl_id <= TBLID_EIT_SCH_A + 0xf) ||
		(_tbl_id >= TBLID_EIT_SCH_O && _tbl_id <= TBLID_EIT_SCH_O + 0xf)
	) {
		// schedule
		return true;

	} else {
		_UTL_LOG_W ("EIT unknown table_id [0x%02x]", _tbl_id);
		return false;
	}
}

void CEventInformationTable_sched::onSectionCompleted (const CSectionInfo *pCompSection)
{
	if (!pCompSection) {
		return ;
	}


	CTable *pTable = new CTable ();
	if (!parse (pCompSection, pTable)) {
		delete pTable;
		pTable = NULL;
		detachSectionList (pCompSection);
		return ;
	}


	if (isDuplicateSubTables (pTable)) {
		delete pTable;
		pTable = NULL;
		detachSectionList (pCompSection);
		return ;
	}


	refreshSubTables (pTable) ;

	appendTable (pTable);

	std::stable_sort (mTables.begin(), mTables.end(), _comp__service_id);
	std::stable_sort (mTables.begin(), mTables.end(), _comp__table_id);
	std::stable_sort (mTables.begin(), mTables.end(), _comp__section_number);


	// debug dump
	if (CUtils::getLogLevel() <= EN_LOG_LEVEL_D) {
		dumpTables_simple ();
//TODO mutex
		std::lock_guard<std::recursive_mutex> lock (mMutexTables);
		dumpTable (pTable);
	}


	// EIT schedule is not create section-list
	detachSectionList (pCompSection);


	if (mpEventScheduleHandler) {
		mpEventScheduleHandler->onChange ();
	}
}

bool CEventInformationTable_sched::parse (const CSectionInfo *pCompSection, CTable* pOutTable)
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
	if (eventLen == 0 || eventLen == EIT_EVENT_FIX_LEN) {
		// allow
	} else if (eventLen < EIT_EVENT_FIX_LEN) {
		_UTL_LOG_W ("invalid EIT event (eventLen=%d)", eventLen);
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
				_UTL_LOG_W ("invalid EIT desc");
				return false;
			}
			ev.descriptors.push_back (desc);
			n -= (2 + *(p + 1));
			p += (2 + *(p + 1));
		}

		eventLen -= (EIT_EVENT_FIX_LEN + ev.descriptors_loop_length) ;
		if (eventLen < 0) {
			_UTL_LOG_W ("invalid EIT event");
			return false;
		}

		pTable->events.push_back (ev);
	}

	return true;
}

void CEventInformationTable_sched::appendTable (CTable *pTable)
{
	if (!pTable) {
		return ;
	}

	std::lock_guard<std::recursive_mutex> lock (mMutexTables);

	mTables.push_back (pTable);
}

void CEventInformationTable_sched::releaseTables (void)
{
	std::lock_guard<std::recursive_mutex> lock (mMutexTables);

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

void CEventInformationTable_sched::releaseTable (CTable *pErase)
{
	if (!pErase) {
		return;
	}

	std::lock_guard<std::recursive_mutex> lock (mMutexTables);

	if (mTables.size() == 0) {
		return;
	}

	std::vector<CTable*>::iterator iter = mTables.begin(); 
	for (; iter != mTables.end(); ++ iter) {
		if (*iter == pErase) {
			delete (*iter);
			(*iter) = NULL;
			iter = mTables.erase(iter);
			break;
		}
	}
}

void CEventInformationTable_sched::dumpTables (void)
{
	std::lock_guard<std::recursive_mutex> lock (mMutexTables);

	if (mTables.size() == 0) {
		return;
	}

	std::vector<CTable*>::const_iterator iter = mTables.begin(); 
	for (; iter != mTables.end(); ++ iter) {
		CTable *pTable = *iter;
		dumpTable (pTable);
	}
}

void CEventInformationTable_sched::dumpTables_event (void)
{
	std::lock_guard<std::recursive_mutex> lock (mMutexTables);

	if (mTables.size() == 0) {
		return;
	}

	_UTL_LOG_I (__PRETTY_FUNCTION__);

	std::vector<CTable*>::const_iterator iter = mTables.begin(); 
	for (; iter != mTables.end(); ++ iter) {
		CTable *pTable = *iter;
		dumpTable_event (pTable);
	}
}

void CEventInformationTable_sched::dumpTables_simple (void)
{
	std::lock_guard<std::recursive_mutex> lock (mMutexTables);

	if (mTables.size() == 0) {
		return;
	}

	_UTL_LOG_I (__PRETTY_FUNCTION__);

	std::vector<CTable*>::const_iterator iter = mTables.begin(); 
	for (; iter != mTables.end(); ++ iter) {
		CTable *pTable = *iter;
		dumpTable_simple (pTable);
	}
}

void CEventInformationTable_sched::dumpSelectTable (
	uint8_t _table_id,
	uint16_t _service_id,
	uint16_t _transport_stream_id,
	uint16_t _original_network_id,
	uint16_t _event_id
)
{
	std::lock_guard<std::recursive_mutex> lock (mMutexTables);

	if (mTables.size() == 0) {
		return;
	}

	std::vector<CTable*>::const_iterator iter = mTables.begin(); 
	for (; iter != mTables.end(); ++ iter) {
		CTable *pTable = *iter;

		if (
			(pTable->header.table_id == _table_id) && 
			(pTable->header.table_id_extension == _service_id) &&
			(pTable->transport_stream_id == _transport_stream_id) &&
			(pTable->original_network_id == _original_network_id)
		) {

			std::vector<CTable::CEvent>::const_iterator iter_event = pTable->events.begin();
			for (; iter_event != pTable->events.end(); ++ iter_event) {
				if (iter_event->event_id == _event_id) {
					dumpTable (pTable, _event_id);
					break;
				}
			}
		}
	}
}

void CEventInformationTable_sched::dumpTable (const CTable* pTable) const
{
	if (!pTable) {
		return;
	}
	
	_UTL_LOG_I (__PRETTY_FUNCTION__);
	pTable->header.dump ();
	_UTL_LOG_I ("========================================\n");

	_UTL_LOG_I ("table_id                    [0x%02x]\n", pTable->header.table_id);
	_UTL_LOG_I ("service_id                  [0x%04x]\n", pTable->header.table_id_extension);
	_UTL_LOG_I ("transport_stream_id         [0x%04x]\n", pTable->transport_stream_id);
	_UTL_LOG_I ("original_network_id         [0x%04x]\n", pTable->original_network_id);
	_UTL_LOG_I ("segment_last_section_number [0x%02x]\n", pTable->segment_last_section_number);
	_UTL_LOG_I ("last_table_id               [0x%02x]\n", pTable->last_table_id);

	std::vector<CTable::CEvent>::const_iterator iter_event = pTable->events.begin();
	for (; iter_event != pTable->events.end(); ++ iter_event) {
		_UTL_LOG_I ("\n--  event  --\n");
		_UTL_LOG_I ("event_id                [0x%04x]\n", iter_event->event_id);
		char szStime [32];
		memset (szStime, 0x00, sizeof(szStime));
		CTsAribCommon::getStrEpoch (CTsAribCommon::getEpochFromMJD (iter_event->start_time), "%Y/%m/%d %H:%M:%S", szStime, sizeof(szStime));
		_UTL_LOG_I ("start_time              [%s]\n", szStime);
		char szDuration [32];
		memset (szDuration, 0x00, sizeof(szDuration));
		CTsAribCommon::getStrSecond (CTsAribCommon::getSecFromBCD (iter_event->duration), szDuration, sizeof(szDuration));
		_UTL_LOG_I ("duration                [%s]\n", szDuration);
		_UTL_LOG_I ("running_status          [0x%02x]\n", iter_event->running_status);
		_UTL_LOG_I ("free_CA_mode            [0x%02x]\n", iter_event->free_CA_mode);
		_UTL_LOG_I ("descriptors_loop_length [%d]\n", iter_event->descriptors_loop_length);

		std::vector<CDescriptor>::const_iterator iter_desc = iter_event->descriptors.begin();
		for (; iter_desc != iter_event->descriptors.end(); ++ iter_desc) {
			_UTL_LOG_I ("\n--  descriptor  --\n");
			CDescriptorCommon::dump (iter_desc->tag, *iter_desc);
		}
	}

	_UTL_LOG_I ("\n");
}

void CEventInformationTable_sched::dumpTable (const CTable* pTable, uint16_t _event_id) const
{
	if (!pTable) {
		return;
	}
	
	_UTL_LOG_I (__PRETTY_FUNCTION__);

	std::vector<CTable::CEvent>::const_iterator iter_event = pTable->events.begin();
	for (; iter_event != pTable->events.end(); ++ iter_event) {

		if (iter_event->event_id != _event_id) {
			continue;
		}

		pTable->header.dump ();
		_UTL_LOG_I ("========================================\n");

		_UTL_LOG_I ("table_id                    [0x%02x]\n", pTable->header.table_id);
		_UTL_LOG_I ("service_id                  [0x%04x]\n", pTable->header.table_id_extension);
		_UTL_LOG_I ("transport_stream_id         [0x%04x]\n", pTable->transport_stream_id);
		_UTL_LOG_I ("original_network_id         [0x%04x]\n", pTable->original_network_id);
		_UTL_LOG_I ("segment_last_section_number [0x%02x]\n", pTable->segment_last_section_number);
		_UTL_LOG_I ("last_table_id               [0x%02x]\n", pTable->last_table_id);

		_UTL_LOG_I ("\n--  event  --\n");
		_UTL_LOG_I ("event_id                [0x%04x]\n", iter_event->event_id);
		char szStime [32];
		memset (szStime, 0x00, sizeof(szStime));
		CTsAribCommon::getStrEpoch (CTsAribCommon::getEpochFromMJD (iter_event->start_time), "%Y/%m/%d %H:%M:%S", szStime, sizeof(szStime));
		_UTL_LOG_I ("start_time              [%s]\n", szStime);
		char szDuration [32];
		memset (szDuration, 0x00, sizeof(szDuration));
		CTsAribCommon::getStrSecond (CTsAribCommon::getSecFromBCD (iter_event->duration), szDuration, sizeof(szDuration));
		_UTL_LOG_I ("duration                [%s]\n", szDuration);
		_UTL_LOG_I ("running_status          [0x%02x]\n", iter_event->running_status);
		_UTL_LOG_I ("free_CA_mode            [0x%02x]\n", iter_event->free_CA_mode);
		_UTL_LOG_I ("descriptors_loop_length [%d]\n", iter_event->descriptors_loop_length);

		std::vector<CDescriptor>::const_iterator iter_desc = iter_event->descriptors.begin();
		for (; iter_desc != iter_event->descriptors.end(); ++ iter_desc) {
			_UTL_LOG_I ("\n--  descriptor  --\n");
			CDescriptorCommon::dump (iter_desc->tag, *iter_desc);
		}
	}

	_UTL_LOG_I ("\n");
}

void CEventInformationTable_sched::dumpTable_event (const CTable* pTable) const
{
	if (!pTable) {
		return;
	}
	
//	_UTL_LOG_I (__PRETTY_FUNCTION__);
//	pTable->header.dump ();
//	_UTL_LOG_I ("========================================\n");

//	_UTL_LOG_I ("table_id                    [0x%02x]\n", pTable->header.table_id);
//	_UTL_LOG_I ("service_id                  [0x%04x]\n", pTable->header.table_id_extension);
//	_UTL_LOG_I ("transport_stream_id         [0x%04x]\n", pTable->transport_stream_id);
//	_UTL_LOG_I ("original_network_id         [0x%04x]\n", pTable->original_network_id);
//	_UTL_LOG_I ("segment_last_section_number [0x%02x]\n", pTable->segment_last_section_number);
//	_UTL_LOG_I ("last_table_id               [0x%02x]\n", pTable->last_table_id);

	std::vector<CTable::CEvent>::const_iterator iter_event = pTable->events.begin();

	if (iter_event != pTable->events.end()) {
		_UTL_LOG_I ("========================================");
	}

	for (; iter_event != pTable->events.end(); ++ iter_event) {
//		_UTL_LOG_I ("\n--  event  --\n");
		_UTL_LOG_I ("--  event  --\n");
		_UTL_LOG_I ("event_id                [0x%04x]\n", iter_event->event_id);
		char szStime [32];
		memset (szStime, 0x00, sizeof(szStime));
		CTsAribCommon::getStrEpoch (CTsAribCommon::getEpochFromMJD (iter_event->start_time), "%Y/%m/%d %H:%M:%S", szStime, sizeof(szStime));
		_UTL_LOG_I ("start_time              [%s]\n", szStime);
		char szDuration [32];
		memset (szDuration, 0x00, sizeof(szDuration));
		CTsAribCommon::getStrSecond (CTsAribCommon::getSecFromBCD (iter_event->duration), szDuration, sizeof(szDuration));
		_UTL_LOG_I ("duration                [%s]\n", szDuration);
//		_UTL_LOG_I ("running_status          [0x%02x]\n", iter_event->running_status);
//		_UTL_LOG_I ("free_CA_mode            [0x%02x]\n", iter_event->free_CA_mode);
//		_UTL_LOG_I ("descriptors_loop_length [%d]\n", iter_event->descriptors_loop_length);

		std::vector<CDescriptor>::const_iterator iter_desc = iter_event->descriptors.begin();
		for (; iter_desc != iter_event->descriptors.end(); ++ iter_desc) {
//			_UTL_LOG_I ("\n--  descriptor  --\n");
//			CDescriptorCommon::dump (iter_desc->tag, *iter_desc);

			if (iter_desc->tag == DESC_TAG__SHORT_EVENT_DESCRIPTOR) {
				CShortEventDescriptor sd (*iter_desc);
				if (!sd.isValid) {
					_UTL_LOG_W ("invalid ShortEventDescriptor");
					continue;
				}

				char aribstr [MAXSECLEN];
				memset (aribstr, 0x00, MAXSECLEN);
				AribToString (aribstr, (const char*)sd.event_name_char, (int)sd.event_name_length);
				_UTL_LOG_I ("event_name_char         [%s]\n", aribstr);
			}
		}
	}

//	_UTL_LOG_I ("\n");
}

void CEventInformationTable_sched::dumpTable_simple (const CTable* pTable) const
{
	if (!pTable) {
		return;
	}
	
//	_UTL_LOG_I ("========================================\n");
//	pTable->header.dump ();

	char buf[128] = {0};
	snprintf (buf, sizeof(buf) -1, "event_id:");
	std::vector<CTable::CEvent>::const_iterator iter_event = pTable->events.begin();
	for (; iter_event != pTable->events.end(); ++ iter_event) {
		int len = strlen (buf);
		if (len > 120) {
			_UTL_LOG_W ("event still continues");
			return;
		}
		snprintf (buf+len, sizeof(buf) -1, "[0x%04x]", iter_event->event_id);
	}

//	_UTL_LOG_I (
//		"svcid:[0x%04x] [%s] tsid:[0x%04x] org_nid:[0x%04x] seg_last:[0x%02x] last:[0x%02x] %s\n",
//		pTable->header.table_id_extension,
//		pTable->header.table_id == 0x4e ? "PF ,A " :
//			pTable->header.table_id == 0x4f ? "PF ,O " :
//			pTable->header.table_id >= 0x50 && pTable->header.table_id < 0x58 ? "Sched,A " :
//			pTable->header.table_id >= 0x58 && pTable->header.table_id < 0x60 ? "Sched,AE" :
//			pTable->header.table_id >= 0x60 && pTable->header.table_id < 0x68 ? "Sched,O " :
//			pTable->header.table_id >= 0x68 && pTable->header.table_id < 0x70 ? "Sched,OE" :
//			"unsup ",
//		pTable->transport_stream_id,
//		pTable->original_network_id,
//		pTable->segment_last_section_number,
//		pTable->last_table_id,
//		buf
//	);
	_UTL_LOG_I (
		"tblid:[0x%02x %s] syntax:[0x%02x] len:[%6d] svcid:[0x%04x] ver:[0x%02x] next:[0x%02x] num:[0x%02x] last:[0x%02x] tsid:[0x%04x] org_nid:[0x%04x] seg_last:[0x%02x] last:[0x%02x] %s\n",
		pTable->header.table_id,
		pTable->header.table_id == 0x4e ? "PF ,A " :
			pTable->header.table_id == 0x4f ? "PF ,O " :
			pTable->header.table_id >= 0x50 && pTable->header.table_id < 0x58 ? "Sched,A " :
			pTable->header.table_id >= 0x58 && pTable->header.table_id < 0x60 ? "Sched,AE" :
			pTable->header.table_id >= 0x60 && pTable->header.table_id < 0x68 ? "Sched,O " :
			pTable->header.table_id >= 0x68 && pTable->header.table_id < 0x70 ? "Sched,OE" :
			"unsup ",
		pTable->header.section_syntax_indicator,
		pTable->header.section_length,
		pTable->header.table_id_extension,
		pTable->header.version_number,
		pTable->header.current_next_indicator,
		pTable->header.section_number,
		pTable->header.last_section_number,
		pTable->transport_stream_id,
		pTable->original_network_id,
		pTable->segment_last_section_number,
		pTable->last_table_id,
		buf
	);
}

void CEventInformationTable_sched::clear (void)
{
	releaseTables ();

//	detachAllSectionList ();
	// detachAllSectionList in parser loop
	asyncDelete ();
}

void CEventInformationTable_sched::clear (CTable *pErase)
{
	releaseTable (pErase);
}

bool CEventInformationTable_sched::isDuplicateSubTables (CTable* pTable)
{
	if (!pTable) {
		return false;
	}

	std::lock_guard<std::recursive_mutex> lock (mMutexTables);


	// sub-table identify
	uint8_t _tblid = pTable->header.table_id;
	uint16_t _svcid = pTable->header.table_id_extension;
	uint16_t _tsid = pTable->transport_stream_id;
	uint16_t _org_nid = pTable->original_network_id;
	uint8_t _num = pTable->header.section_number;
	uint8_t _version = pTable->header.version_number;


	std::vector<CTable*>::const_iterator iter = mTables.begin();
    for (; iter != mTables.end(); ++ iter) {
		CTable *p = *iter;

		// check every sub-tables
		if (
			(_tblid == p->header.table_id) &&
			(_svcid == p->header.table_id_extension) &&
			(_tsid == p->transport_stream_id) &&
			(_org_nid == p->original_network_id) &&
			(_num == p->header.section_number) &&
			(_version == p->header.version_number)
		) {
			return true;
		}
	}

	return false;
}

bool CEventInformationTable_sched::refreshSubTablesByVersionNumber (CTable* pNewTable)
{
	if (!pNewTable) {
		return false;
	}

	std::lock_guard<std::recursive_mutex> lock (mMutexTables);


	// sub-table identify
	uint8_t new_tblid = pNewTable->header.table_id;
	uint16_t new_svcid = pNewTable->header.table_id_extension;
	uint16_t new_tsid = pNewTable->transport_stream_id;
	uint16_t new_org_nid = pNewTable->original_network_id;


	uint8_t new_ver = pNewTable->header.version_number;
	uint8_t sec_num = pNewTable->header.section_number;

	CTable *pErase = NULL;

	std::vector<CTable*>::const_iterator iter = mTables.begin();
    for (; iter != mTables.end(); ++ iter) {
		CTable *pTable = *iter;

		// check every sub-tables
		if (
			(new_tblid == pTable->header.table_id) &&
			(new_svcid == pTable->header.table_id_extension) &&
			(new_tsid == pTable->transport_stream_id) &&
			(new_org_nid == pTable->original_network_id)
		) {
			uint8_t ver_tmp = pTable->header.version_number;
			++ ver_tmp;
			ver_tmp &= 0x1f;

			if (new_ver >= ver_tmp) {

				// new version
				pErase = pTable;
				break;

			} else {
				if (new_ver == pTable->header.version_number) {
					if (sec_num == pTable->header.section_number) {
						// duplicate sections should be checked by CSectionParser
						_UTL_LOG_E (
							"BUG: same version.  new_ver:[0x%02x]  pTable->header.version_number:[0x%02x]",
							new_ver,
							pTable->header.version_number
						);
					}
				} else {
					_UTL_LOG_E (
						"BUG: unexpected version.  new_ver:[0x%02x]  pTable->header.version_number:[0x%02x]",
						new_ver,
						pTable->header.version_number
					);
				}
			}
		}
	}

	if (pErase) {
		releaseTable (pErase);
		return true;
	} else {
		return false;
	}
}

void CEventInformationTable_sched::refreshSubTables (CTable* pNewTable)
{
	if (!pNewTable) {
		return ;
	}

	while (1) {
		if (!refreshSubTablesByVersionNumber (pNewTable)) {
			break;
		}
	}
}

CEventInformationTable_sched::CReference CEventInformationTable_sched::reference (void)
{
	CReference ref (&mTables, &mMutexTables);
	return ref;
}
