#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ChannelManager.h"
#include "modules.h"

#include "Settings.h"


CChannelManager::CChannelManager (char *pszName, uint8_t nQueNum)
	:CThreadMgrBase (pszName, nQueNum)
	,m_tunerNotify_clientId (0xff)
{
	mSeqs [EN_SEQ_CHANNEL_MANAGER__MODULE_UP] =
		{(PFN_SEQ_BASE)&CChannelManager::onReq_moduleUp,                     (char*)"onReq_moduleUp"};
	mSeqs [EN_SEQ_CHANNEL_MANAGER__MODULE_DOWN] =
		{(PFN_SEQ_BASE)&CChannelManager::onReq_moduleDown,                   (char*)"onReq_moduleDown"};
	mSeqs [EN_SEQ_CHANNEL_MANAGER__CHANNEL_SCAN] =
		{(PFN_SEQ_BASE)&CChannelManager::onReq_channelScan,                  (char*)"onReq_channelScan"};
	mSeqs [EN_SEQ_CHANNEL_MANAGER__GET_PYSICAL_CHANNEL_BY_SERVICE_ID] =
		{(PFN_SEQ_BASE)&CChannelManager::onReq_getPysicalChannelByServiceId, (char*)"onReq_getPysicalChannelByServiceId"};
	mSeqs [EN_SEQ_CHANNEL_MANAGER__DUMP_SCAN_RESULTS] =
		{(PFN_SEQ_BASE)&CChannelManager::onReq_dumpScanResults,              (char*)"onReq_dumpScanResults"};
	setSeqs (mSeqs, EN_SEQ_CHANNEL_MANAGER__NUM);


	m_scanResults.clear ();

}

CChannelManager::~CChannelManager (void)
{
}


void CChannelManager::onReq_moduleUp (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_REG_TUNER_NOTIFY,
		SECTID_WAIT_REG_TUNER_NOTIFY,
		SECTID_END_SUCCESS,
		SECTID_END_ERROR,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

	EN_THM_RSLT enRslt = EN_THM_RSLT_SUCCESS;


	switch (sectId) {
	case SECTID_ENTRY:

		loadScanResults ();
		dumpScanResults_simple ();


//		sectId = SECTID_REQ_REG_TUNER_NOTIFY;
//TODO TUNER_NOTIFYは特にいらない
sectId = SECTID_END_SUCCESS;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_REG_TUNER_NOTIFY: {
		CTunerControlIf _if (getExternalIf());
		_if.reqRegisterTunerNotify ();

		sectId = SECTID_WAIT_REG_TUNER_NOTIFY;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_REG_TUNER_NOTIFY:
		enRslt = pIf->getSrcInfo()->enRslt;
        if (enRslt == EN_THM_RSLT_SUCCESS) {
			m_tunerNotify_clientId = *(uint8_t*)(pIf->getSrcInfo()->msg.pMsg);
			sectId = SECTID_END_SUCCESS;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			_UTL_LOG_E ("reqRegisterTunerNotify is failure.");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}
		break;

	case SECTID_END_SUCCESS:
		pIf->reply (EN_THM_RSLT_SUCCESS);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	case SECTID_END_ERROR:
		pIf->reply (EN_THM_RSLT_ERROR);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->setSectId (sectId, enAct);
}

void CChannelManager::onReq_moduleDown (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

//
// do nothing
//

	pIf->reply (EN_THM_RSLT_SUCCESS);

	sectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (sectId, enAct);
}

void CChannelManager::onReq_channelScan (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_SET_FREQ,
		SECTID_REQ_TUNE,
		SECTID_WAIT_TUNE,
		SECTID_WAIT_AFTER_TUNE,
		SECTID_REQ_GET_NETWORK_INFO,
		SECTID_WAIT_GET_NETWORK_INFO,
		SECTID_NEXT,
		SECTID_END,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

	static uint16_t s_ch = UHF_PHYSICAL_CHANNEL_MIN;	
	static PSISI_NETWORK_INFO s_network_info = {0};	
	EN_THM_RSLT enRslt = EN_THM_RSLT_SUCCESS;


	switch (sectId) {
	case SECTID_ENTRY:
		pIf->lock();

		// 先にreplyしとく
		pIf->reply (EN_THM_RSLT_SUCCESS);

		m_scanResults.clear ();

		sectId = SECTID_SET_FREQ;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_SET_FREQ:

		if (s_ch >= UHF_PHYSICAL_CHANNEL_MIN && s_ch <= UHF_PHYSICAL_CHANNEL_MAX) {
			sectId = SECTID_REQ_TUNE;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			sectId = SECTID_END;
			enAct = EN_THM_ACT_CONTINUE;
		}
		break;

	case SECTID_REQ_TUNE: {

		uint32_t freq = CTsAribCommon::pysicalCh2freqKHz (s_ch);
		_UTL_LOG_I ("(%s) ------  pysical channel:[%d] -> freq:[%d]kHz", pIf->getSeqName(), s_ch, freq);

		CTunerControlIf _if (getExternalIf());
		_if.reqTune (freq);

		sectId = SECTID_WAIT_TUNE;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_TUNE:
		enRslt = pIf->getSrcInfo()->enRslt;
        if (enRslt == EN_THM_RSLT_SUCCESS) {
			pIf->setTimeout (7500);
			sectId = SECTID_WAIT_AFTER_TUNE;
			enAct = EN_THM_ACT_WAIT;

		} else {
			_UTL_LOG_W ("tune is failure -> skip");
			sectId = SECTID_NEXT;
			enAct = EN_THM_ACT_CONTINUE;
		}
		break;

	case SECTID_WAIT_AFTER_TUNE:
		sectId = SECTID_REQ_GET_NETWORK_INFO;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_GET_NETWORK_INFO: {

		CPsisiManagerIf _if (getExternalIf());
		_if.reqGetCurrentNetworkInfo (&s_network_info);

		sectId = SECTID_WAIT_GET_NETWORK_INFO;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_GET_NETWORK_INFO:
		enRslt = pIf->getSrcInfo()->enRslt;
        if (enRslt == EN_THM_RSLT_SUCCESS) {

			CScanResult r;
			r.set (
				s_network_info.transport_stream_id,
				s_network_info.original_network_id,
				(const char*)s_network_info.network_name_char,
				s_network_info.area_code,
				s_network_info.remote_control_key_id,
				(const char*)s_network_info.ts_name_char,
				(CScanResult::service*)s_network_info.services, // このキャストはどうなの
				s_network_info.services_num
			);

			r.dump();

			m_scanResults.insert (pair<uint16_t, CScanResult>(s_ch, r));

			sectId = SECTID_NEXT;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			_UTL_LOG_W ("network info is not found -> skip");
			sectId = SECTID_NEXT;
			enAct = EN_THM_ACT_CONTINUE;
		}
		break;

	case SECTID_NEXT:

		// inc pysical ch
		++ s_ch;

		memset (&s_network_info, 0x00, sizeof (s_network_info));

		sectId = SECTID_SET_FREQ;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_END:
		pIf->unlock();

		// reset pysical ch
		s_ch = UHF_PHYSICAL_CHANNEL_MIN;

		memset (&s_network_info, 0x00, sizeof (s_network_info));

		dumpScanResults ();
		saveScanResults ();

		_UTL_LOG_I ("channel scan end.");

		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->setSectId (sectId, enAct);
}

void CChannelManager::onReq_getPysicalChannelByServiceId (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);


	_SERVICE_ID_PARAM param = *(_SERVICE_ID_PARAM*)(pIf->getSrcInfo()->msg.pMsg);

	uint16_t _ch = getPysicalChannelByServiceId (
						param.transport_stream_id,
						param.original_network_id,
						param.service_id
					);
	if (_ch == 0xffff) {

		_UTL_LOG_E ("getPysicalChannelByServiceId is failure.");
		pIf->reply (EN_THM_RSLT_ERROR);

	} else {

		// リプライmsgに結果を乗せます
		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)&_ch, sizeof(_ch));

	}


	sectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (sectId, enAct);
}

void CChannelManager::onReq_dumpScanResults (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);


	dumpScanResults ();


	pIf->reply (EN_THM_RSLT_SUCCESS);

	sectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (sectId, enAct);
}

uint16_t CChannelManager::getPysicalChannelByServiceId (
	uint16_t _transport_stream_id,
	uint16_t _original_network_id,
	uint16_t _service_id
)
{
	std::map <uint16_t, CScanResult>::iterator iter = m_scanResults.begin();
	for (; iter != m_scanResults.end(); ++ iter) {
		uint16_t ch = iter->first;
		CScanResult *p_rslt = &(iter->second);
		if (p_rslt) {

			if (
				(p_rslt->transport_stream_id == _transport_stream_id) &&
				(p_rslt->original_network_id == _original_network_id)
			) {
				std::vector<CScanResult::service>::iterator iter = p_rslt->services.begin();
				for (; iter != p_rslt->services.end(); ++ iter) {
					if (_service_id == iter->service_id) {
						return ch;
					}
				}
			}
		}
	}

	// not found
	return 0xffff;
}

void CChannelManager::dumpScanResults (void)
{
	std::map <uint16_t, CScanResult>::iterator iter = m_scanResults.begin();
	for (; iter != m_scanResults.end(); ++ iter) {
		uint16_t ch = iter->first;
		CScanResult *p_rslt = &(iter->second);
		if (p_rslt) {
			uint32_t freq = CTsAribCommon::pysicalCh2freqKHz (ch);
			_UTL_LOG_I ("-------------  pysical channel:[%d] ([%d]kHz) -------------", ch, freq);
			p_rslt ->dump ();
		}
	}
}

void CChannelManager::dumpScanResults_simple (void)
{
	std::map <uint16_t, CScanResult>::iterator iter = m_scanResults.begin();
	for (; iter != m_scanResults.end(); ++ iter) {
		CScanResult *p_rslt = &(iter->second);
		if (p_rslt) {
			p_rslt ->dump_simple ();
		}
	}
}



//--------------------------------------------------------------------------------

template <class Archive>
void serialize (Archive &archive, CScanResult::service &s)
{
	archive (cereal::make_nvp("service_id", s.service_id));
	archive (cereal::make_nvp("service_type", s.service_type));
}

template <class Archive>
void serialize (Archive &archive, CScanResult &r)
{
	archive (
		cereal::make_nvp("transport_stream_id", r.transport_stream_id),
		cereal::make_nvp("original_network_id", r.original_network_id),
		cereal::make_nvp("network_name", r.network_name),
		cereal::make_nvp("area_code", r.area_code),
		cereal::make_nvp("remote_control_key_id", r.remote_control_key_id),
		cereal::make_nvp("ts_name", r.ts_name),
		cereal::make_nvp("services", r.services)
	);
}

void CChannelManager::saveScanResults (void)
{
	std::stringstream ss;
	{
		cereal::JSONOutputArchive out_archive (ss);
		out_archive (CEREAL_NVP(m_scanResults));
	}

	std::string *p_path = CSettings::getInstance()->getParams()->getChannelScanJsonPath();
	std::ofstream ofs (p_path->c_str(), std::ios::out);
	ofs << ss.str();

	ofs.close();
	ss.clear();
}

void CChannelManager::loadScanResults (void)
{
	std::string *p_path = CSettings::getInstance()->getParams()->getChannelScanJsonPath();
	std::ifstream ifs (p_path->c_str(), std::ios::in);
	if (!ifs.is_open()) {
		_UTL_LOG_I ("scan.json is not found.");
		return;
	}

	std::stringstream ss;
	ss << ifs.rdbuf();

	cereal::JSONInputArchive in_archive (ss);
	in_archive (CEREAL_NVP(m_scanResults));

	ifs.close();
	ss.clear();
}