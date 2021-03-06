#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "utils.h"
#include "verstr.h"
#include "message.h"
#ifdef __cplusplus
};
#endif

#include "it9175_extern.h"

#include "RecManager.h"
#include "RecManagerIf.h"

#include "modules.h"

#include "aribstr.h"



static const struct reserve_state_pair g_reserveStatePair [] = {
	{0x00000000, "INIT"},
	{0x00000001, "REMOVE_RESERVE"},
	{0x00000002, "START_TIME_PASSED"},
	{0x00000004, "REQ_START_RECORDING"},
	{0x00000008, "NOW_RECORDING"},
	{0x00000010, "FORCE_STOP"},
	{0x00000020, "END_SUCCESS"},
	{0x00000040, "END_ERROR__ALREADY_PASSED"},
	{0x00000080, "END_ERROR__HDD_FREE_SPACE_LOW"},
	{0x00000100, "END_ERROR__INTERNAL_ERR"},

	{0x00000000, NULL}, // term
};

static char gsz_reserveState [100];

const char * getReserveState (uint32_t state)
{
	memset (gsz_reserveState, 0x00, sizeof(gsz_reserveState));
	int n = 0;
	int s = 0;
	char *pw = gsz_reserveState;
	int _size = sizeof(gsz_reserveState);

	while (g_reserveStatePair [n].psz_reserveState != NULL) {
		if (state & g_reserveStatePair [n].state) {
			s = snprintf (pw, _size,
								"%s,", g_reserveStatePair [n].psz_reserveState);
			pw += s;
			_size -= s;
		}
		++ n ;
	}

	if (pw == gsz_reserveState) {
		strncpy (
			gsz_reserveState,
			g_reserveStatePair[0].psz_reserveState,
			sizeof(gsz_reserveState) -1
		);
	}

	return gsz_reserveState;
}

const char *g_repeatability [] = {
	"NONE",
	"DAILY",
	"WEEKLY",
	"AUTO",
};


typedef struct {
	EN_REC_PROGRESS rec_progress;
	uint32_t reserve_state;
} RECORDING_NOTICE_t;


CRecManager::CRecManager (char *pszName, uint8_t nQueNum)
	:CThreadMgrBase (pszName, nQueNum)
	,m_tunerNotify_clientId (0xff)
	,m_tsReceive_handlerId (-1)
	,m_patDetectNotify_clientId (0xff)
	,m_eventChangeNotify_clientId (0xff)
	,m_recProgress (EN_REC_PROGRESS__INIT)
	,mp_outputBuffer (NULL)
{
	mSeqs [EN_SEQ_REC_MANAGER__MODULE_UP] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_moduleUp,                (char*)"onReq_moduleUp"};
	mSeqs [EN_SEQ_REC_MANAGER__MODULE_DOWN] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_moduleDown,              (char*)"onReq_moduleDown"};
	mSeqs [EN_SEQ_REC_MANAGER__CHECK_LOOP] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_checkLoop,               (char*)"onReq_checkLoop"};
	mSeqs [EN_SEQ_REC_MANAGER__CHECK_EVENT_LOOP] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_checkEventLoop,          (char*)"onReq_checkEventLoop"};
	mSeqs [EN_SEQ_REC_MANAGER__RECORDING_NOTICE] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_recordingNotice,         (char*)"onReq_recordingNotice"};
	mSeqs [EN_SEQ_REC_MANAGER__START_RECORDING] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_startRecording,          (char*)"onReq_startRecording"};
	mSeqs [EN_SEQ_REC_MANAGER__ADD_RESERVE_CURRENT_EVENT] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_addReserve_currentEvent, (char*)"onReq_addReserve_currentEvent"};
	mSeqs [EN_SEQ_REC_MANAGER__ADD_RESERVE_EVENT] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_addReserve_event,        (char*)"onReq_addReserve_event"};
	mSeqs [EN_SEQ_REC_MANAGER__ADD_RESERVE_EVENT_HELPER] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_addReserve_eventHelper,  (char*)"onReq_addReserve_eventHelper"};
	mSeqs [EN_SEQ_REC_MANAGER__ADD_RESERVE_MANUAL] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_addReserve_manual,       (char*)"onReq_addReserve_manual"};
	mSeqs [EN_SEQ_REC_MANAGER__REMOVE_RESERVE] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_removeReserve,           (char*)"onReq_removeReserve"};
	mSeqs [EN_SEQ_REC_MANAGER__REMOVE_RESERVE_BY_INDEX] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_removeReserve_byIndex,   (char*)"onReq_removeReserve_byIndex"};
	mSeqs [EN_SEQ_REC_MANAGER__STOP_RECORDING] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_stopRecording,           (char*)"onReq_stopRecording"};
	mSeqs [EN_SEQ_REC_MANAGER__DUMP_RESERVES] =
		{(PFN_SEQ_BASE)&CRecManager::onReq_dumpReserves,            (char*)"onReq_dumpReserves"};
	setSeqs (mSeqs, EN_SEQ_REC_MANAGER__NUM);


	mp_settings = CSettings::getInstance();

	clearReserves ();
	clearResults ();
	m_recording.clear();

	memset (m_recording_tmpfile, 0x00, sizeof (m_recording_tmpfile));
}

CRecManager::~CRecManager (void)
{
	clearReserves ();
	clearResults ();
	m_recording.clear();

	memset (m_recording_tmpfile, 0x00, sizeof (m_recording_tmpfile));
}


void CRecManager::onReq_moduleUp (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_REG_TUNER_NOTIFY,
		SECTID_WAIT_REG_TUNER_NOTIFY,
		SECTID_REQ_REG_HANDLER,
		SECTID_WAIT_REG_HANDLER,
		SECTID_REQ_REG_PAT_DETECT_NOTIFY,
		SECTID_WAIT_REG_PAT_DETECT_NOTIFY,
		SECTID_REQ_REG_EVENT_CHANGE_NOTIFY,
		SECTID_WAIT_REG_EVENT_CHANGE_NOTIFY,
		SECTID_REQ_CHECK_LOOP,
		SECTID_WAIT_CHECK_LOOP,
		SECTID_REQ_CHECK_EVENT_LOOP,
		SECTID_WAIT_CHECK_EVENT_LOOP,
		SECTID_END_SUCCESS,
		SECTID_END_ERROR,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

	EN_THM_RSLT enRslt = EN_THM_RSLT_SUCCESS;
	// request msgでインスタンスアドレス渡す用
	static CTunerControlIf::ITsReceiveHandler *s_p = NULL;


	switch (sectId) {
	case SECTID_ENTRY:

		loadReserves ();
		loadResults ();

		sectId = SECTID_REQ_REG_TUNER_NOTIFY;
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
			sectId = SECTID_REQ_REG_HANDLER;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			_UTL_LOG_E ("reqRegisterTunerNotify is failure.");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}
		break;

	case SECTID_REQ_REG_HANDLER: {

		s_p = this;
		_UTL_LOG_I ("CTunerControlIf::ITsReceiveHandler %p", s_p);
		CTunerControlIf _if (getExternalIf());
		_if.reqRegisterTsReceiveHandler (&s_p);

		sectId = SECTID_WAIT_REG_HANDLER;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_REG_HANDLER:
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			m_tsReceive_handlerId = *(int*)(pIf->getSrcInfo()->msg.pMsg);
			sectId = SECTID_REQ_REG_PAT_DETECT_NOTIFY;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			_UTL_LOG_E ("reqRegisterTsReceiveHandler is failure.");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}
		break;

	case SECTID_REQ_REG_PAT_DETECT_NOTIFY: {
		CPsisiManagerIf _if (getExternalIf());
		_if.reqRegisterPatDetectNotify ();

		sectId = SECTID_WAIT_REG_PAT_DETECT_NOTIFY;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_REG_PAT_DETECT_NOTIFY:
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			m_patDetectNotify_clientId = *(uint8_t*)(pIf->getSrcInfo()->msg.pMsg);
			sectId = SECTID_REQ_REG_EVENT_CHANGE_NOTIFY;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			_UTL_LOG_E ("reqRegisterPatDetectNotify is failure.");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}
		break;

	case SECTID_REQ_REG_EVENT_CHANGE_NOTIFY: {
		CPsisiManagerIf _if (getExternalIf());
		_if.reqRegisterEventChangeNotify ();

		sectId = SECTID_WAIT_REG_EVENT_CHANGE_NOTIFY;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_REG_EVENT_CHANGE_NOTIFY:
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			m_eventChangeNotify_clientId = *(uint8_t*)(pIf->getSrcInfo()->msg.pMsg);
			sectId = SECTID_REQ_CHECK_LOOP;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			_UTL_LOG_E ("reqRegisterEventChangeNotify is failure.");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}
		break;

	case SECTID_REQ_CHECK_LOOP:
		requestAsync (EN_MODULE_REC_MANAGER, EN_SEQ_REC_MANAGER__CHECK_LOOP);

		sectId = SECTID_WAIT_CHECK_LOOP;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_CHECK_LOOP:
//		enRslt = pIf->getSrcInfo()->enRslt;
//		if (enRslt == EN_THM_RSLT_SUCCESS) {
//
//		} else {
//
//		}
// EN_THM_RSLT_SUCCESSのみ

		sectId = SECTID_REQ_CHECK_EVENT_LOOP;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_CHECK_EVENT_LOOP:
		requestAsync (EN_MODULE_REC_MANAGER, EN_SEQ_REC_MANAGER__CHECK_EVENT_LOOP);

		sectId = SECTID_WAIT_CHECK_EVENT_LOOP;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_CHECK_EVENT_LOOP:
//		enRslt = pIf->getSrcInfo()->enRslt;
//		if (enRslt == EN_THM_RSLT_SUCCESS) {
//
//		} else {
//
//		}
// EN_THM_RSLT_SUCCESSのみ

		sectId = SECTID_END_SUCCESS;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_END_SUCCESS:
		s_p = NULL;
		pIf->reply (EN_THM_RSLT_SUCCESS);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	case SECTID_END_ERROR:
		s_p = NULL;
		pIf->reply (EN_THM_RSLT_ERROR);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}


	pIf->setSectId (sectId, enAct);
}

void CRecManager::onReq_moduleDown (CThreadMgrIf *pIf)
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

void CRecManager::onReq_checkLoop (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_CHECK,
		SECTID_CHECK_WAIT,
		SECTID_REQ_START_RECORDING,
		SECTID_WAIT_START_RECORDING,
		SECTID_REQ_GET_PRESENT_EVENT_INFO,
		SECTID_WAIT_GET_PRESENT_EVENT_INFO,
		SECTID_END,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

	EN_THM_RSLT enRslt = EN_THM_RSLT_SUCCESS;
	static PSISI_EVENT_INFO s_presentEventInfo;


	switch (sectId) {
	case SECTID_ENTRY:
		// 先にreplyしておく
		pIf->reply (EN_THM_RSLT_SUCCESS);

		sectId = SECTID_CHECK;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_CHECK:

		pIf->setTimeout (1000); // 1sec

		sectId = SECTID_CHECK_WAIT;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_CHECK_WAIT:

		// reserve check
		checkReserves ();


		if (m_recording.state & RESERVE_STATE__NOW_RECORDING) {

			// recording end check
			if (checkRecordingEnd ()) {

				// recording end
				sectId = SECTID_CHECK;
				enAct = EN_THM_ACT_CONTINUE;

			} else {
				sectId = SECTID_REQ_GET_PRESENT_EVENT_INFO;
				enAct = EN_THM_ACT_CONTINUE;
			}

		} else {

			if (pickReqStartRecordingReserve ()) {
				// request start recording
				requestAsync (EN_MODULE_REC_MANAGER, EN_SEQ_REC_MANAGER__START_RECORDING);

				sectId = SECTID_WAIT_START_RECORDING;
				enAct = EN_THM_ACT_WAIT;

			} else {
				sectId = SECTID_CHECK;
				enAct = EN_THM_ACT_CONTINUE;
			}
		}

		break;

	case SECTID_WAIT_START_RECORDING:
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
//TODO imple

			sectId = SECTID_CHECK;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			// NOW_RECORDINGフラグは落としておきます
			m_recording.state &= ~RESERVE_STATE__NOW_RECORDING;
			m_recording.state |= RESERVE_STATE__END_ERROR__INTERNAL_ERR;
			setResult (&m_recording);
			m_recording.clear();

			sectId = SECTID_CHECK;
			enAct = EN_THM_ACT_CONTINUE;
		}

		break;

	case SECTID_REQ_GET_PRESENT_EVENT_INFO: {

		PSISI_SERVICE_INFO _svc_info ;
		// 以下３つの要素が入っていればOK
		_svc_info.transport_stream_id = m_recording.transport_stream_id;
		_svc_info.original_network_id = m_recording.original_network_id;
		_svc_info.service_id = m_recording.service_id;

		CPsisiManagerIf _if (getExternalIf());
		_if.reqGetPresentEventInfo (&_svc_info, &s_presentEventInfo);

		sectId = SECTID_WAIT_GET_PRESENT_EVENT_INFO;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_GET_PRESENT_EVENT_INFO:
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
//s_presentEventInfo.dump();
			if (m_recording.state & RESERVE_STATE__NOW_RECORDING) {

				if (m_recording.is_event_type) {
					// event_typeの録画は event_idとend_timeの確認
					if (m_recording.event_id == s_presentEventInfo.event_id) {
						if (m_recording.end_time != s_presentEventInfo.end_time) {
							_UTL_LOG_I (
								"#####  recording end_time is update.  [%s] -> [%s]  #####",
								m_recording.end_time.toString (),
								s_presentEventInfo.end_time.toString ()
							);
							m_recording.end_time = s_presentEventInfo.end_time;
						}

					} else {
//TODO
						// event_idが変わった とりあえずログだしとく
						_UTL_LOG_W (
							"#####  recording event_id is update.  [0x%04x] -> [0x%04x]  #####",
							s_presentEventInfo.event_id,
							m_recording.event_id
						);
						m_recording.dump();
						s_presentEventInfo.dump();
					}

				} else {
					// manual録画は event_name_charを代入しときます
					m_recording.event_id = s_presentEventInfo.event_id;
					m_recording.title_name = s_presentEventInfo.event_name_char;
				}
			}

		} else {
			_UTL_LOG_E ("reqGetPresentEventInfo err");
		}

		memset (&s_presentEventInfo, 0x00, sizeof(s_presentEventInfo));

		sectId = SECTID_CHECK;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_END:
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->setSectId (sectId, enAct);
}

//TODO  eventSchedMgrの stateNotify契機でチェックする方がいいかも
void CRecManager::onReq_checkEventLoop (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_CHECK,
		SECTID_CHECK_WAIT,
		SECTID_END,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

	EN_THM_RSLT enRslt = EN_THM_RSLT_SUCCESS;


	switch (sectId) {
	case SECTID_ENTRY:
		// 先にreplyしておく
		pIf->reply (EN_THM_RSLT_SUCCESS);

		sectId = SECTID_CHECK;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_CHECK:

		pIf->setTimeout (60*1000); // 1min

		sectId = SECTID_CHECK_WAIT;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_CHECK_WAIT: {

		// EPG取得が完了しているか確認します
		CEventScheduleManagerIf _if (getExternalIf());
		_if.syncGetCacheScheduleState ();
		EN_CACHE_SCHEDULE_STATE_t _s =  *(EN_CACHE_SCHEDULE_STATE_t*)(getIf()->getSrcInfo()->msg.pMsg);
		if (_s != EN_CACHE_SCHEDULE_STATE__READY) {
			// readyでないので以下の処理は行いません
			sectId = SECTID_CHECK;
			enAct = EN_THM_ACT_CONTINUE;
			break;
		}


		// 録画予約に対応するイベント内容ををチェックします
		for (int i = 0; i < RESERVE_NUM_MAX; ++ i) {

			if (!m_reserves [i].is_used) {
				continue;
			}

			if (!m_reserves [i].state == RESERVE_STATE__INIT) {
				continue;
			}

			if (!m_reserves [i].is_event_type) {
				continue;
			}


			// スケジュールを確認してstart_time, end_time, event_name が変わってないかチェックします

			CEventScheduleManagerIf::EVENT_t event;
			CEventScheduleManagerIf::REQ_EVENT_PARAM_t param = {
				{
					m_reserves [i].transport_stream_id,
					m_reserves [i].original_network_id,
					m_reserves [i].service_id,
					m_reserves [i].event_id
				},
				&event
			};

			CEventScheduleManagerIf _if (getExternalIf());
			_if.syncGetEvent (&param);
			
			enRslt = getIf()->getSrcInfo()->enRslt;
			if (enRslt == EN_THM_RSLT_SUCCESS) {
				if ( m_reserves [i].start_time != param.p_out_event-> start_time) {
					_UTL_LOG_I (
						"check_event  reserve start_time is update.  [%s] -> [%s]",
						m_reserves [i].start_time.toString(),
						param.p_out_event->start_time.toString()
					);
					m_reserves [i].start_time = param.p_out_event-> start_time;
				}

				if (m_reserves [i].end_time != param.p_out_event-> end_time) {
					_UTL_LOG_I (
						"check_event  reserve end_time is update.  [%s] -> [%s]",
						m_reserves [i].end_time.toString(),
						param.p_out_event->end_time.toString()
					);
					m_reserves [i].end_time = param.p_out_event-> end_time;
				}

				if (m_reserves [i].title_name != *param.p_out_event-> p_event_name) {
					_UTL_LOG_I (
						"check_event  reserve title_name is update.  [%s] -> [%s]",
						m_reserves [i].title_name.c_str(),
						param.p_out_event->p_event_name->c_str()
					);
					m_reserves [i].title_name = param.p_out_event-> p_event_name->c_str();
				}

			} else {
				// 予約に対応するイベントがなかった
				_UTL_LOG_E ("syncGetEvent error");
				m_reserves [i].dump();
			}

		}


		sectId = SECTID_CHECK;
		enAct = EN_THM_ACT_CONTINUE;

		}
		break;

	case SECTID_END:
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->setSectId (sectId, enAct);
}
void CRecManager::onReq_recordingNotice (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);


	RECORDING_NOTICE_t _notice = *(RECORDING_NOTICE_t*)(pIf->getSrcInfo()->msg.pMsg);
	switch (_notice.rec_progress) {
	case EN_REC_PROGRESS__INIT:
		break;

	case EN_REC_PROGRESS__PRE_PROCESS:
		// ここでは エラーが起きることがあるのでRESERVE_STATE をチェックします
		if (_notice.reserve_state != RESERVE_STATE__INIT) {
			m_recording.state |= _notice.reserve_state;
		}
		break;

	case EN_REC_PROGRESS__NOW_RECORDING:
		break;

	case EN_REC_PROGRESS__END_SUCCESS:
		m_recording.state |= RESERVE_STATE__END_SUCCESS;
		break;

	case EN_REC_PROGRESS__END_ERROR:
		break;

	case EN_REC_PROGRESS__POST_PROCESS: {

		// NOW_RECORDINGフラグは落としておきます
		m_recording.state &= ~RESERVE_STATE__NOW_RECORDING;

		m_recording.recording_end_time.setCurrentTime();

		setResult (&m_recording);


		// rename
		std::string *p_path = mp_settings->getParams()->getRecTsPath();

		char newfile [PATH_MAX] = {0};
		char *p_name = (char*)"rec";
		if (m_recording.title_name.c_str()) {
			p_name = (char*)m_recording.title_name.c_str();
		}
		CEtime t_end;
		t_end.setCurrentTime();
		snprintf (
			newfile,
			sizeof(newfile),
			"%s/%s_%s.m2ts",
			p_path->c_str(),
			p_name,
			t_end.toString()
		);

		rename (m_recording_tmpfile, newfile) ;

		m_recording.clear ();
		_UTL_LOG_I ("recording end...");


		//-----------------------------//
		{
			uint32_t opt = getRequestOption ();
			opt |= REQUEST_OPTION__WITHOUT_REPLY;
			setRequestOption (opt);

			// 選局を停止しときます tune stop
			// とりあえず投げっぱなし (REQUEST_OPTION__WITHOUT_REPLY)
			CTunerControlIf _if (getExternalIf());
			_if.reqTuneStop ();

			opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
			setRequestOption (opt);
		}
		//-----------------------------//


		}
		break;

	default:
		break;
	};

	pIf->reply (EN_THM_RSLT_SUCCESS);

	sectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (sectId, enAct);
}

void CRecManager::onReq_startRecording (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_TUNE_BY_SERVICE_ID,
		SECTID_WAIT_TUNE_BY_SERVICE_ID,
		SECTID_REQ_GET_PRESENT_EVENT_INFO,
		SECTID_WAIT_GET_PRESENT_EVENT_INFO,
		SECTID_REQ_GET_FOLLOW_EVENT_INFO,
		SECTID_WAIT_GET_FOLLOW_EVENT_INFO,
		SECTID_START_RECORDING,
		SECTID_END_SUCCESS,
		SECTID_END_ERROR,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

	EN_THM_RSLT enRslt = EN_THM_RSLT_SUCCESS;
	static PSISI_EVENT_INFO s_presentEventInfo;
	static PSISI_EVENT_INFO s_followEventInfo;
	static int s_retry = 0;


	switch (sectId) {
	case SECTID_ENTRY:
		sectId = SECTID_REQ_TUNE_BY_SERVICE_ID;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_TUNE_BY_SERVICE_ID: {

		CChannelManagerIf::SERVICE_ID_PARAM_t param = {
			m_recording.transport_stream_id,
			m_recording.original_network_id,
			m_recording.service_id
		};

		CChannelManagerIf _if (getExternalIf());
		_if.reqTuneByServiceId_withRetry (&param);

		sectId = SECTID_WAIT_TUNE_BY_SERVICE_ID;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_TUNE_BY_SERVICE_ID:
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
//			sectId = SECTID_START_RECORDING;

			// イベント開始時間を確認します
			sectId = SECTID_REQ_GET_PRESENT_EVENT_INFO;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			_UTL_LOG_E ("reqTuneByServiceId is failure.");
			_UTL_LOG_E ("tune is failure  -> not start recording");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}
		break;

	case SECTID_REQ_GET_PRESENT_EVENT_INFO: {

		if (s_retry >= 10) {
			_UTL_LOG_E ("(%s) retry over.", pIf->getSeqName());
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;

		} else {

			PSISI_SERVICE_INFO _svc_info ;
			// 以下３つの要素が入っていればOK
			_svc_info.transport_stream_id = m_recording.transport_stream_id;
			_svc_info.original_network_id = m_recording.original_network_id;
			_svc_info.service_id = m_recording.service_id;

			CPsisiManagerIf _if (getExternalIf());
			_if.reqGetPresentEventInfo (&_svc_info, &s_presentEventInfo);

			sectId = SECTID_WAIT_GET_PRESENT_EVENT_INFO;
			enAct = EN_THM_ACT_WAIT;
		}

		}
		break;

	case SECTID_WAIT_GET_PRESENT_EVENT_INFO:
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {

			if (s_presentEventInfo.event_id == m_recording.event_id) {

				if (s_presentEventInfo.start_time == m_recording.start_time) {
					// イベント開始時間が一致しているのでこのまま録画を開始します
					sectId = SECTID_START_RECORDING;
					enAct = EN_THM_ACT_CONTINUE;

				} else if (s_presentEventInfo.start_time > m_recording.start_time) {
					// present event なのにありえるのか？
					// システムの時計がくるっていたりしたらありえる
					_UTL_LOG_E ("s_presentEventInfo.start_time > m_recording.start_time ??");
					sectId = SECTID_END_ERROR;
					enAct = EN_THM_ACT_CONTINUE;

				} else {
					// すでに開始時間過ぎていた このまま録画を開始します
					sectId = SECTID_START_RECORDING;
					enAct = EN_THM_ACT_CONTINUE;
				}
			} else {
				// イベントが一致しないので follow eventを調べます
				sectId = SECTID_REQ_GET_FOLLOW_EVENT_INFO;
				enAct = EN_THM_ACT_CONTINUE;
			}

		} else {
			_UTL_LOG_E ("reqGetPresentEventInfo err");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}
		break;

	case SECTID_REQ_GET_FOLLOW_EVENT_INFO: {

		PSISI_SERVICE_INFO _svc_info ;
		// 以下３つの要素が入っていればOK
		_svc_info.transport_stream_id = m_recording.transport_stream_id;
		_svc_info.original_network_id = m_recording.original_network_id;
		_svc_info.service_id = m_recording.service_id;

		CPsisiManagerIf _if (getExternalIf());
		_if.reqGetFollowEventInfo (&_svc_info, &s_followEventInfo);

		sectId = SECTID_WAIT_GET_FOLLOW_EVENT_INFO;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_GET_FOLLOW_EVENT_INFO:
		if (enRslt == EN_THM_RSLT_SUCCESS) {

			if (s_followEventInfo.event_id == m_recording.event_id) {

				if (s_followEventInfo.start_time == m_recording.start_time) {
					// (おそらく psisi mgr側でまだcasheできてない場合)
					// イベント開始時間が一致している もう一度 present eventを調べます
					++ s_retry;
					pIf->setTimeout (100);
					sectId = SECTID_REQ_GET_PRESENT_EVENT_INFO;
					enAct = EN_THM_ACT_WAIT;

				} else if (s_followEventInfo.start_time > m_recording.start_time) {
					_UTL_LOG_I (
						"#####  event start time is delayed.  [%s] -> [%s]  #####",
						m_recording.start_time.toString (),
						s_followEventInfo.start_time.toString ()
					);

					// イベント開始時間が遅れた
					// 予約入れなおします
					bool r = addReserve (
									m_recording.transport_stream_id,
									m_recording.original_network_id,
									m_recording.service_id,
									m_recording.event_id,
									&s_followEventInfo.start_time, // 開始時間を更新
									&s_followEventInfo.end_time, // 終了時間も変わってるかもしれないので更新
									m_recording.title_name.c_str(),
									m_recording.service_name.c_str(),
									true, // is_event_type true
									m_recording.repeatability
   								);
					if (r) {
						// 録画は行いません
						m_recording.clear();

						sectId = SECTID_END_SUCCESS;
						enAct = EN_THM_ACT_CONTINUE;

					} else {
						sectId = SECTID_END_ERROR;
						enAct = EN_THM_ACT_CONTINUE;
					}

				} else {
					// follow event なのにありえるのか？
					// システムの時計がくるっていたりしたらありえる
					_UTL_LOG_E ("s_followEventInfo.start_time > m_recording.start_time ??");
					sectId = SECTID_END_ERROR;
					enAct = EN_THM_ACT_CONTINUE;
				}

			} else {
				// イベントが一致しないのでもう一度 present eventを調べます
				++ s_retry;
				pIf->setTimeout (100);
				sectId = SECTID_REQ_GET_PRESENT_EVENT_INFO;
				enAct = EN_THM_ACT_WAIT;
			}

		} else {
			_UTL_LOG_E ("reqGetFollowEventInfo err");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}
		break;

	case SECTID_START_RECORDING:

		// 録画開始
		// ここはm_recProgressで判断いれとく
		if (m_recProgress == EN_REC_PROGRESS__INIT) {

			_UTL_LOG_I ("start recording (on tune thread)");

			memset (m_recording_tmpfile, 0x00, sizeof(m_recording_tmpfile));
			std::string *p_path = mp_settings->getParams()->getRecTsPath();
			snprintf (
				m_recording_tmpfile,
				sizeof(m_recording_tmpfile),
				"%s/tmp.m2ts.%lu",
				p_path->c_str(),
				pthread_self()
			);


			// ######################################### //
			m_recProgress = EN_REC_PROGRESS__PRE_PROCESS;
			// ######################################### //


			m_recording.state |= RESERVE_STATE__NOW_RECORDING;
			m_recording.recording_start_time.setCurrentTime();

			sectId = SECTID_END_SUCCESS;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			_UTL_LOG_E ("m_recProgress != EN_REC_PROGRESS__INIT ???  -> not start recording");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}

		break;

	case SECTID_END_SUCCESS:

		memset (&s_presentEventInfo, 0x00, sizeof(s_presentEventInfo));
		memset (&s_followEventInfo, 0x00, sizeof(s_followEventInfo));
		s_retry = 0;

		pIf->reply (EN_THM_RSLT_SUCCESS);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	case SECTID_END_ERROR:

		memset (&s_presentEventInfo, 0x00, sizeof(s_presentEventInfo));
		memset (&s_followEventInfo, 0x00, sizeof(s_followEventInfo));
		s_retry = 0;

		pIf->reply (EN_THM_RSLT_ERROR);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->setSectId (sectId, enAct);
}

void CRecManager::onReq_addReserve_currentEvent (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_GET_TUNER_STATE,
		SECTID_WAIT_GET_TUNER_STATE,
		SECTID_REQ_GET_PAT_DETECT_STATE,
		SECTID_WAIT_GET_PAT_DETECT_STATE,
		SECTID_REQ_GET_SERVICE_INFOS,
		SECTID_WAIT_GET_SERVICE_INFOS,
		SECTID_REQ_GET_PRESENT_EVENT_INFO,
		SECTID_WAIT_GET_PRESENT_EVENT_INFO,
		SECTID_END_SUCCESS,
		SECTID_END_ERROR,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

	EN_THM_RSLT enRslt = EN_THM_RSLT_SUCCESS;
	static PSISI_SERVICE_INFO s_serviceInfos[10];
	static PSISI_EVENT_INFO s_presentEventInfo;


	switch (sectId) {
	case SECTID_ENTRY:
		sectId = SECTID_REQ_GET_TUNER_STATE;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_GET_TUNER_STATE: {
		CTunerControlIf _if (getExternalIf());
		_if.reqGetState ();

		sectId = SECTID_WAIT_GET_TUNER_STATE;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_GET_TUNER_STATE: {

		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			EN_TUNER_STATE _state = *(EN_TUNER_STATE*)(pIf->getSrcInfo()->msg.pMsg);
			if (_state == EN_TUNER_STATE__TUNING_SUCCESS) {
				sectId = SECTID_REQ_GET_PAT_DETECT_STATE;
				enAct = EN_THM_ACT_CONTINUE;
			} else {
				_UTL_LOG_E ("not EN_TUNER_STATE__TUNING_SUCCESS %d", _state);
#ifdef _DUMMY_TUNER
				sectId = SECTID_REQ_GET_PAT_DETECT_STATE;
#else
				sectId = SECTID_END_ERROR;
#endif
				enAct = EN_THM_ACT_CONTINUE;
			}

		} else {
			// success only
		}

		}
		break;

	case SECTID_REQ_GET_PAT_DETECT_STATE: {
		CPsisiManagerIf _if (getExternalIf());
		_if.reqGetPatDetectState ();

		sectId = SECTID_WAIT_GET_PAT_DETECT_STATE;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_GET_PAT_DETECT_STATE: {
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			EN_PAT_DETECT_STATE _state = *(EN_PAT_DETECT_STATE*)(pIf->getSrcInfo()->msg.pMsg);
			if (_state == EN_PAT_DETECT_STATE__DETECTED) {
				sectId = SECTID_REQ_GET_SERVICE_INFOS;
				enAct = EN_THM_ACT_CONTINUE;
			} else {
				_UTL_LOG_E ("not EN_PAT_DETECT_STATE__DETECTED %d", _state);
#ifdef _DUMMY_TUNER
				sectId = SECTID_REQ_GET_SERVICE_INFOS;
#else
				sectId = SECTID_END_ERROR;
#endif
				enAct = EN_THM_ACT_CONTINUE;
			}

		} else {
			// success only
		}

		}
		break;

	case SECTID_REQ_GET_SERVICE_INFOS: {
		CPsisiManagerIf _if (getExternalIf());
		_if.reqGetCurrentServiceInfos (s_serviceInfos, 10);

		sectId = SECTID_WAIT_GET_SERVICE_INFOS;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_GET_SERVICE_INFOS:
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			int num = *(int*)(pIf->getSrcInfo()->msg.pMsg);
			if (num > 0) {
s_serviceInfos[0].dump();
				sectId = SECTID_REQ_GET_PRESENT_EVENT_INFO;
				enAct = EN_THM_ACT_CONTINUE;

			} else {
				_UTL_LOG_E ("reqGetCurrentServiceInfos is 0");
				sectId = SECTID_END_ERROR;
				enAct = EN_THM_ACT_CONTINUE;
			}

		} else {
			_UTL_LOG_E ("reqGetCurrentServiceInfos err");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}

		break;

	case SECTID_REQ_GET_PRESENT_EVENT_INFO: {
		CPsisiManagerIf _if (getExternalIf());
//TODO s_serviceInfos[0] 暫定0番目を使います 大概service_type=0x01でHD画質だろうか
		_if.reqGetPresentEventInfo (&s_serviceInfos[0], &s_presentEventInfo);

		sectId = SECTID_WAIT_GET_PRESENT_EVENT_INFO;
		enAct = EN_THM_ACT_WAIT;

		} break;

	case SECTID_WAIT_GET_PRESENT_EVENT_INFO:
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
s_presentEventInfo.dump();

			char *p_svc_name = NULL;
			CChannelManagerIf::SERVICE_ID_PARAM_t param = {
				s_presentEventInfo.transport_stream_id,
				s_presentEventInfo.original_network_id,
				s_presentEventInfo.service_id
			};
			CChannelManagerIf _if (getExternalIf());
			_if.syncGetServiceName (&param); // sync wait
			enRslt = getIf()->getSrcInfo()->enRslt;
			if (enRslt == EN_THM_RSLT_SUCCESS) {
				p_svc_name = (char*)(getIf()->getSrcInfo()->msg.pMsg);
			}

			bool r = addReserve (
							s_presentEventInfo.transport_stream_id,
							s_presentEventInfo.original_network_id,
							s_presentEventInfo.service_id,
							s_presentEventInfo.event_id,
							&s_presentEventInfo.start_time,
							&s_presentEventInfo.end_time,
							s_presentEventInfo.event_name_char,
							p_svc_name,
							true // is_event_type true
						);
			if (r) {
				sectId = SECTID_END_SUCCESS;
				enAct = EN_THM_ACT_CONTINUE;

			} else {
				sectId = SECTID_END_ERROR;
				enAct = EN_THM_ACT_CONTINUE;
			}

		} else {
			_UTL_LOG_E ("reqGetPresentEventInfo err");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}

		break;

	case SECTID_END_SUCCESS:

		memset (s_serviceInfos, 0x00, sizeof(s_serviceInfos));
		memset (&s_presentEventInfo, 0x00, sizeof(s_presentEventInfo));

		pIf->reply (EN_THM_RSLT_SUCCESS);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	case SECTID_END_ERROR:

		memset (s_serviceInfos, 0x00, sizeof(s_serviceInfos));
		memset (&s_presentEventInfo, 0x00, sizeof(s_presentEventInfo));

		pIf->reply (EN_THM_RSLT_ERROR);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->setSectId (sectId, enAct);
}

void CRecManager::onReq_addReserve_event (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_GET_EVENT,
		SECTID_WAIT_GET_EVENT,
		SECTID_ADD_RESERVE,
		SECTID_END_SUCCESS,
		SECTID_END_ERROR,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

	EN_THM_RSLT enRslt = EN_THM_RSLT_SUCCESS;
	static CRecManagerIf::ADD_RESERVE_PARAM_t s_param;
	static CEventScheduleManagerIf::EVENT_t s_event;


	switch (sectId) {
	case SECTID_ENTRY:

		s_param = *(CRecManagerIf::ADD_RESERVE_PARAM_t*)(pIf->getSrcInfo()->msg.pMsg);


		// repeatablityのチェック入れときます
		if (
			s_param.repeatablity == EN_RESERVE_REPEATABILITY__NONE ||
			s_param.repeatablity == EN_RESERVE_REPEATABILITY__AUTO
		) {
			sectId = SECTID_REQ_GET_EVENT;
			enAct = EN_THM_ACT_CONTINUE;
		} else {
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}

		break;

	case SECTID_REQ_GET_EVENT: {

		CEventScheduleManagerIf::REQ_EVENT_PARAM_t param = {
			{
				s_param.transport_stream_id,
				s_param.original_network_id,
				s_param.service_id,
				s_param.event_id
			},
			&s_event
		};

		CEventScheduleManagerIf _if (getExternalIf());
		_if.reqGetEvent (&param);

		sectId = SECTID_WAIT_GET_EVENT;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_GET_EVENT:
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			sectId = SECTID_ADD_RESERVE;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			// 予約に対応するイベントがなかった あらら...
			_UTL_LOG_E ("reqGetEvent error");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}

		break;

	case SECTID_ADD_RESERVE: {

		char *p_svc_name = NULL;
		CChannelManagerIf::SERVICE_ID_PARAM_t param = {
			s_param.transport_stream_id,
			s_param.original_network_id,
			s_param.service_id
		};
		CChannelManagerIf _if (getExternalIf());
		_if.syncGetServiceName (&param); // sync wait
		enRslt = getIf()->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			p_svc_name = (char*)(getIf()->getSrcInfo()->msg.pMsg);
		}


		bool r = addReserve (
						s_event.transport_stream_id,
						s_event.original_network_id,
						s_event.service_id,
						s_event.event_id,
						&s_event.start_time,
						&s_event.end_time,
						s_event.p_event_name->c_str(),
						p_svc_name,
						true, // is_event_type true
						s_param.repeatablity
					);
		if (r) {
			sectId = SECTID_END_SUCCESS;
			enAct = EN_THM_ACT_CONTINUE;
		} else {
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}

		}
		break;

	case SECTID_END_SUCCESS:

		memset (&s_param, 0x00, sizeof(s_param));
		memset (&s_event, 0x00, sizeof(s_event));

		pIf->reply (EN_THM_RSLT_SUCCESS);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	case SECTID_END_ERROR:

		memset (&s_param, 0x00, sizeof(s_param));
		memset (&s_event, 0x00, sizeof(s_event));

		pIf->reply (EN_THM_RSLT_ERROR);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}


	pIf->setSectId (sectId, enAct);
}

void CRecManager::onReq_addReserve_eventHelper (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_GET_EVENT,
		SECTID_WAIT_GET_EVENT,
		SECTID_ADD_RESERVE,
		SECTID_END_SUCCESS,
		SECTID_END_ERROR,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

	EN_THM_RSLT enRslt = EN_THM_RSLT_SUCCESS;
	static CRecManagerIf::ADD_RESERVE_HELPER_PARAM_t s_param;
	static CEventScheduleManagerIf::EVENT_t s_event;


	switch (sectId) {
	case SECTID_ENTRY:

		s_param = *(CRecManagerIf::ADD_RESERVE_HELPER_PARAM_t*)(pIf->getSrcInfo()->msg.pMsg);


		// repeatablityのチェック入れときます
		if (
			s_param.repeatablity == EN_RESERVE_REPEATABILITY__NONE ||
			s_param.repeatablity == EN_RESERVE_REPEATABILITY__AUTO
		) {
			sectId = SECTID_REQ_GET_EVENT;
			enAct = EN_THM_ACT_CONTINUE;
		} else {
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}

		break;

	case SECTID_REQ_GET_EVENT: {

		CEventScheduleManagerIf::REQ_EVENT_PARAM_t param ;
		param.arg.index = s_param.index;
		param.p_out_event = &s_event;

		CEventScheduleManagerIf _if (getExternalIf());
		_if.reqGetEvent_latestDumpedSchedule (&param);

		sectId = SECTID_WAIT_GET_EVENT;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_GET_EVENT:
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			sectId = SECTID_ADD_RESERVE;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			// 予約に対応するイベントがなかった あらら...
			_UTL_LOG_E ("reqGetEvent error");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}

		break;

	case SECTID_ADD_RESERVE: {

		char *p_svc_name = NULL;
		CChannelManagerIf::SERVICE_ID_PARAM_t param = {
			s_event.transport_stream_id,
			s_event.original_network_id,
			s_event.service_id
		};
		CChannelManagerIf _if (getExternalIf());
		_if.syncGetServiceName (&param); // sync wait
		enRslt = getIf()->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			p_svc_name = (char*)(getIf()->getSrcInfo()->msg.pMsg);
		}


		bool r = addReserve (
						s_event.transport_stream_id,
						s_event.original_network_id,
						s_event.service_id,
						s_event.event_id,
						&s_event.start_time,
						&s_event.end_time,
						s_event.p_event_name->c_str(),
						p_svc_name,
						true, // is_event_type true
						s_param.repeatablity
					);
		if (r) {
			sectId = SECTID_END_SUCCESS;
			enAct = EN_THM_ACT_CONTINUE;
		} else {
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}

		}
		break;

	case SECTID_END_SUCCESS:

		memset (&s_param, 0x00, sizeof(s_param));
		memset (&s_event, 0x00, sizeof(s_event));

		pIf->reply (EN_THM_RSLT_SUCCESS);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	case SECTID_END_ERROR:

		memset (&s_param, 0x00, sizeof(s_param));
		memset (&s_event, 0x00, sizeof(s_event));

		pIf->reply (EN_THM_RSLT_ERROR);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}


	pIf->setSectId (sectId, enAct);
}

void CRecManager::onReq_addReserve_manual (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_GET_PYSICAL_CH_BY_SERVICE_ID,
		SECTID_WAIT_GET_PYSICAL_CH_BY_SERVICE_ID,
		SECTID_ADD_RESERVE,
		SECTID_END_SUCCESS,
		SECTID_END_ERROR,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

	static CRecManagerIf::ADD_RESERVE_PARAM_t s_param;
	EN_THM_RSLT enRslt = EN_THM_RSLT_SUCCESS;


	switch (sectId) {
	case SECTID_ENTRY:

		s_param = *(CRecManagerIf::ADD_RESERVE_PARAM_t*)(pIf->getSrcInfo()->msg.pMsg);


		// repeatablityのチェック入れときます
		if (
			s_param.repeatablity >= EN_RESERVE_REPEATABILITY__NONE &&
			s_param.repeatablity <= EN_RESERVE_REPEATABILITY__WEEKLY
		) {
			sectId = SECTID_REQ_GET_PYSICAL_CH_BY_SERVICE_ID;
			enAct = EN_THM_ACT_CONTINUE;
		} else {
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}

		break;

	case SECTID_REQ_GET_PYSICAL_CH_BY_SERVICE_ID: {
		// サービスの存在チェックします

		CChannelManagerIf::SERVICE_ID_PARAM_t param = {
			s_param.transport_stream_id,
			s_param.original_network_id,
			s_param.service_id
		};

		CChannelManagerIf _if (getExternalIf());
		_if.reqGetPysicalChannelByServiceId (&param);

		sectId = SECTID_WAIT_GET_PYSICAL_CH_BY_SERVICE_ID;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_GET_PYSICAL_CH_BY_SERVICE_ID:
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			sectId = SECTID_ADD_RESERVE;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			_UTL_LOG_E ("reqGetPysicalChannelByServiceId is failure.");
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}
		break;

	case SECTID_ADD_RESERVE: {

		char *p_svc_name = NULL;
		CChannelManagerIf::SERVICE_ID_PARAM_t param = {
			s_param.transport_stream_id,
			s_param.original_network_id,
			s_param.service_id
		};
		CChannelManagerIf _if (getExternalIf());
		_if.syncGetServiceName (&param); // sync wait
		enRslt = getIf()->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			p_svc_name = (char*)(getIf()->getSrcInfo()->msg.pMsg);
		}


		bool r = addReserve (
						s_param.transport_stream_id,
						s_param.original_network_id,
						s_param.service_id,
						0x00, // event_idはこの時点では不明
						&s_param.start_time,
						&s_param.end_time,
						NULL, // タイトル名も不明 (そもそもs_paramにない)
						p_svc_name,
						false,
						s_param.repeatablity
					);
		if (r) {
			sectId = SECTID_END_SUCCESS;
			enAct = EN_THM_ACT_CONTINUE;
		} else {
			sectId = SECTID_END_ERROR;
			enAct = EN_THM_ACT_CONTINUE;
		}

		}
		break;

	case SECTID_END_SUCCESS:

		memset (&s_param, 0x00, sizeof(s_param));

		pIf->reply (EN_THM_RSLT_SUCCESS);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	case SECTID_END_ERROR:

		memset (&s_param, 0x00, sizeof(s_param));

		pIf->reply (EN_THM_RSLT_ERROR);
		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}


	pIf->setSectId (sectId, enAct);
}

void CRecManager::onReq_removeReserve (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);


	CRecManagerIf::REMOVE_RESERVE_PARAM_t param =
			*(CRecManagerIf::REMOVE_RESERVE_PARAM_t*)(pIf->getSrcInfo()->msg.pMsg);

	int idx = getReserveIndex (
					param.arg.key.transport_stream_id,
					param.arg.key.original_network_id,
					param.arg.key.service_id,
					param.arg.key.event_id
				);

	if (idx < 0) {
		pIf->reply (EN_THM_RSLT_ERROR);

	} else {
		if (removeReserve (idx, param.isConsiderRepeatability, param.isApplyResult)) {
			pIf->reply (EN_THM_RSLT_SUCCESS);
		} else {
			pIf->reply (EN_THM_RSLT_ERROR);
		}
	}

	sectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (sectId, enAct);
}

void CRecManager::onReq_removeReserve_byIndex (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);


	CRecManagerIf::REMOVE_RESERVE_PARAM_t param =
			*(CRecManagerIf::REMOVE_RESERVE_PARAM_t*)(pIf->getSrcInfo()->msg.pMsg);

	if (removeReserve (param.arg.index, param.isConsiderRepeatability, param.isApplyResult)) {
		pIf->reply (EN_THM_RSLT_SUCCESS);
	} else {
		pIf->reply (EN_THM_RSLT_ERROR);
	}


	sectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (sectId, enAct);
}

void CRecManager::onReq_stopRecording (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);


	if (m_recording.state & RESERVE_STATE__NOW_RECORDING) {

		// stopRecording が呼ばれたらエラー終了にしておきます
		_UTL_LOG_W ("m_recProgress = EN_REC_PROGRESS__END_ERROR");


		// ######################################### //
		m_recProgress = EN_REC_PROGRESS__END_ERROR;
		// ######################################### //


		m_recording.state |= RESERVE_STATE__FORCE_STOP;

		pIf->reply (EN_THM_RSLT_SUCCESS);

	} else {

		_UTL_LOG_E ("invalid rec state");
		pIf->reply (EN_THM_RSLT_ERROR);
	}

	sectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (sectId, enAct);
}

void CRecManager::onReq_dumpReserves (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);


	int type = *(int*)(pIf->getSrcInfo()->msg.pMsg);
	switch (type) {
	case 0:
		dumpReserves();
		break;

	case 1:
		dumpResults();
		break;

	case 2:
		dumpRecording();
		break;

	default:
		break;
	}


	pIf->reply (EN_THM_RSLT_SUCCESS);

	sectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (sectId, enAct);
}

void CRecManager::onReceiveNotify (CThreadMgrIf *pIf)
{
	if (pIf->getSrcInfo()->nClientId == m_tunerNotify_clientId) {

		EN_TUNER_STATE enState = *(EN_TUNER_STATE*)(pIf->getSrcInfo()->msg.pMsg);
		switch (enState) {
		case EN_TUNER_STATE__TUNING_BEGIN:
			_UTL_LOG_I ("EN_TUNER_STATE__TUNING_BEGIN");
			break;

		case EN_TUNER_STATE__TUNING_SUCCESS:
			_UTL_LOG_I ("EN_TUNER_STATE__TUNING_SUCCESS");
			break;

		case EN_TUNER_STATE__TUNING_ERROR_STOP:
			_UTL_LOG_I ("EN_TUNER_STATE__TUNING_ERROR_STOP");
			break;

		case EN_TUNER_STATE__TUNE_STOP:
			_UTL_LOG_I ("EN_TUNER_STATE__TUNE_STOP");
			break;

		default:
			break;
		}


	} else if (pIf->getSrcInfo()->nClientId == m_patDetectNotify_clientId) {

		EN_PAT_DETECT_STATE _state = *(EN_PAT_DETECT_STATE*)(pIf->getSrcInfo()->msg.pMsg);
		if (_state == EN_PAT_DETECT_STATE__DETECTED) {
			_UTL_LOG_I ("EN_PAT_DETECT_STATE__DETECTED");

		} else if (_state == EN_PAT_DETECT_STATE__NOT_DETECTED) {
			_UTL_LOG_E ("EN_PAT_DETECT_STATE__NOT_DETECTED");

			// PAT途絶したら録画中止します
			if (m_recording.state & RESERVE_STATE__NOW_RECORDING) {

				// ######################################### //
				m_recProgress = EN_REC_PROGRESS__POST_PROCESS;
				// ######################################### //


				m_recording.state |= RESERVE_STATE__END_ERROR__INTERNAL_ERR;


				uint32_t opt = getRequestOption ();
				opt |= REQUEST_OPTION__WITHOUT_REPLY;
				setRequestOption (opt);

				// 自ら呼び出します
				// 内部で自リクエストするので
				// REQUEST_OPTION__WITHOUT_REPLY を入れときます
				//
				// PAT途絶してTsReceiveHandlerは動いていない前提
				this->onTsReceived (NULL, 0);

				opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
				setRequestOption (opt);

			}
		}

	} else if (pIf->getSrcInfo()->nClientId == m_eventChangeNotify_clientId) {

		PSISI_NOTIFY_EVENT_INFO _info = *(PSISI_NOTIFY_EVENT_INFO*)(pIf->getSrcInfo()->msg.pMsg);
		_UTL_LOG_I ("!!! event changed !!!");
		_info.dump ();

	}

}

bool CRecManager::addReserve (
	uint16_t _transport_stream_id,
	uint16_t _original_network_id,
	uint16_t _service_id,
	uint16_t _event_id,
	CEtime* p_start_time,
	CEtime* p_end_time,
	const char *psz_title_name,
	const char *psz_service_name,
	bool _is_event_type,
	EN_RESERVE_REPEATABILITY repeatabilitiy
)
{
	if (!p_start_time || !p_end_time) {
		return false;
	}

	if (*p_start_time >= *p_end_time) {
		_UTL_LOG_E ("invalid reserve time");
		return false;
	}


	CRecReserve tmp;
	tmp.set (
		_transport_stream_id,
		_original_network_id,
		_service_id,
		_event_id,
		p_start_time,
		p_end_time,
		psz_title_name,
		psz_service_name,
		_is_event_type,
		repeatabilitiy
	);


	if (!isExistEmptyReserve ()) {
		_UTL_LOG_E ("reserve full.");
		return false;
	}

	if (isDuplicateReserve (&tmp)) {
		_UTL_LOG_E ("reserve is duplicate.");
		return false;
	}

	if (isOverrapTimeReserve (&tmp)) {
		_UTL_LOG_W ("reserve time is overrap.");
// 時間被ってるけど予約入れます
//		return false;
	}


	CRecReserve* p_reserve = searchAscendingOrderReserve (p_start_time);
	if (!p_reserve) {
		_UTL_LOG_E ("searchAscendingOrderReserve failure.");
		return false;
	}

	p_reserve->set (
		_transport_stream_id,
		_original_network_id,
		_service_id,
		_event_id,
		p_start_time,
		p_end_time,
		psz_title_name,
		psz_service_name,
		_is_event_type,
		repeatabilitiy
	);


	_UTL_LOG_I ("###########################");
	_UTL_LOG_I ("####    add reserve    ####");
	_UTL_LOG_I ("###########################");
	p_reserve->dump();


	// 毎回書き込み
	saveReserves ();


	return true;
}

/**
 * 引数をキーと同一予約のindexを返します
 * 見つからない場合は -1 で返します
 */
int CRecManager::getReserveIndex (
	uint16_t _transport_stream_id,
	uint16_t _original_network_id,
	uint16_t _service_id,
	uint16_t _event_id
)
{
	CRecReserve tmp;
	tmp.set (
		_transport_stream_id,
		_original_network_id,
		_service_id,
		_event_id,
		NULL,
		NULL,
		NULL,
		NULL,
		false,
		EN_RESERVE_REPEATABILITY__NONE
	);

	for (int i = 0; i < RESERVE_NUM_MAX; ++ i) {
		if (!m_reserves [i].is_used) {
			continue;
		}

		if (m_reserves [i] == tmp) {
			return i;
		}
	}

	return -1;
}

/**
 * indexで指定した予約を削除します
 * 0始まり
 * isConsiderRepeatability == false で Repeatability関係なく削除します
 */
bool CRecManager::removeReserve (int index, bool isConsiderRepeatability, bool isApplyResult)
{
	if (index >= RESERVE_NUM_MAX) {
		return false;
	}

	m_reserves [index].state |= RESERVE_STATE__REMOVE_RESERVE;
	if (isApplyResult) {
		setResult (&m_reserves[index]);
	}

	_UTL_LOG_I ("##############################");
	_UTL_LOG_I ("####    remove reserve    ####");
	_UTL_LOG_I ("##############################");
	m_reserves [index].dump();

	if (isConsiderRepeatability) {
		checkRepeatability (&m_reserves[index]);
	}

	// 間詰め
	for (int i = index; i < RESERVE_NUM_MAX -1; ++ i) {
		m_reserves [i] = m_reserves [i+1];
	}
	m_reserves [RESERVE_NUM_MAX -1].clear();


	// 毎回書き込み
	saveReserves ();


	return true;
}

/**
 * 開始時間を基準に降順で空きをさがします
 * 空きがある前提
 */
CRecReserve* CRecManager::searchAscendingOrderReserve (CEtime *p_start_time_ref)
{
	if (!p_start_time_ref) {
		return NULL;
	}


	int i = 0;
	for (i = 0; i < RESERVE_NUM_MAX; ++ i) {

		// 先頭から詰まっているはず
//		if (!m_reserves [i].is_used) {
//			continue;
//		}

		// 基準時間より後ろの時間を探します
		if (m_reserves [i].start_time > *p_start_time_ref) {

			// 後ろから見てずらします
			for (int j = RESERVE_NUM_MAX -1; j > i; -- j) {
				m_reserves [j] = m_reserves [j-1] ;
			}

			break;
		}
	}

	if (i == RESERVE_NUM_MAX) {
		// 見つからなかったので最後尾にします
		return findEmptyReserve ();

	} else {

		m_reserves [i].clear ();
		return &m_reserves [i];
	}
}

bool CRecManager::isExistEmptyReserve (void) const
{
	int i = 0;
	for (i = 0; i < RESERVE_NUM_MAX; ++ i) {
		if (!m_reserves [i].is_used) {
			break;
		}
	}

	if (i == RESERVE_NUM_MAX) {
		_UTL_LOG_W ("m_reserves full.");
		return false;
	}

	return true;
}

CRecReserve* CRecManager::findEmptyReserve (void)
{
	int i = 0;
	for (i = 0; i < RESERVE_NUM_MAX; ++ i) {
		if (!m_reserves [i].is_used) {
			break;
		}
	}

	if (i == RESERVE_NUM_MAX) {
		_UTL_LOG_W ("m_reserves full.");
		return NULL;
	}

	return &m_reserves [i];
}

bool CRecManager::isDuplicateReserve (const CRecReserve* p_reserve) const
{
	if (!p_reserve) {
		return false;
	}

	for (int i = 0; i < RESERVE_NUM_MAX; ++ i) {
		if (!m_reserves [i].is_used) {
			continue;
		}

		if (m_reserves [i] == *p_reserve) {
			// duplicate
			return true;
		}
	}

	return false;
}

bool CRecManager::isOverrapTimeReserve (const CRecReserve* p_reserve) const
{
	if (!p_reserve) {
		return false;
	}

	for (int i = 0; i < RESERVE_NUM_MAX; ++ i) {
		if (!m_reserves [i].is_used) {
			continue;
		}

		if (m_reserves [i].start_time > p_reserve->start_time && m_reserves [i].start_time > p_reserve->end_time) {
			continue;
		}

		if (m_reserves [i].end_time < p_reserve->start_time && m_reserves [i].end_time < p_reserve->end_time) {
			continue;
		}

		// overrap
		return true;
	}

	return false;
}

void CRecManager::checkReserves (void)
{
	CEtime current_time;
	current_time.setCurrentTime();

	for (int i = 0; i < RESERVE_NUM_MAX; ++ i) {

		if (!m_reserves [i].is_used) {
			continue;
		}

		// 開始時間/終了時間とも過ぎていたら resultsに入れときます
		if (m_reserves [i].start_time < current_time && m_reserves [i].end_time <= current_time) {
			m_reserves [i].state |= RESERVE_STATE__END_ERROR__ALREADY_PASSED;
			setResult (&m_reserves[i]);
			checkRepeatability (&m_reserves[i]);
			continue;
		}

		// 開始時間をみて 時間が来たら 録画要求を立てます
		// start_time - end_time 範囲内
		if (m_reserves [i].start_time <= current_time && m_reserves [i].end_time > current_time) {

			// 開始時間から10秒以上経過していたら フラグたてときます
			CEtime tmp = m_reserves [i].start_time;
			tmp.addSec(10);
			if (tmp <= current_time) {
				m_reserves [i].state |= RESERVE_STATE__START_TIME_PASSED;
			}

			if (!m_recording.is_used) {
				// 録画中でなければ 録画開始フラグ立てます
				m_reserves [i].state |= RESERVE_STATE__REQ_START_RECORDING;

				// 見つかったのでbreakします。
				// 同時刻で予約が入っていた場合 配列の並び順で先の方を録画開始します
				break;
			}
		}
	}

	refreshReserves (RESERVE_STATE__END_ERROR__ALREADY_PASSED);
}

/**
 * 引数のstateが立っている予約を除去します
 */
void CRecManager::refreshReserves (uint32_t state)
{
	// 逆から見ます
	for (int i = RESERVE_NUM_MAX -1; i >= 0; -- i) {
		if (!m_reserves [i].is_used) {
			continue;
		}

		if (!(m_reserves[i].state & state)) {
			continue;
		}

		// 間詰め
		for (int j = i; j < RESERVE_NUM_MAX -1; ++ j) {
			m_reserves [j] = m_reserves [j+1];
		}
		m_reserves [RESERVE_NUM_MAX -1].clear();

		saveReserves ();
	}
}

bool CRecManager::pickReqStartRecordingReserve (void)
{
	if (m_recording.is_used) {
		return false;
	}

	for (int i = 0; i < RESERVE_NUM_MAX; ++ i) {
		if (!m_reserves [i].is_used) {
			continue;
		}

		if (m_reserves[i].state & RESERVE_STATE__REQ_START_RECORDING) {

			// 次に録画する予約を取り出します
			m_recording = m_reserves[i];

			// フラグは落としておきます
			m_recording.state &= ~RESERVE_STATE__REQ_START_RECORDING;


			// 間詰め
			for (int j = i; j < RESERVE_NUM_MAX -1; ++ j) {
				m_reserves [j] = m_reserves [j+1];
			}
			m_reserves [RESERVE_NUM_MAX -1].clear();

			checkRepeatability (&m_recording);

			saveReserves ();

			return true;
		}
	}

	return false;
}

//TODO event_typeの場合はどうするか
/**
 * Repeatabilityの確認して
 * 予約入れるべきものは予約します
 */
void CRecManager::checkRepeatability (const CRecReserve *p_reserve)
{
	if (!p_reserve) {
		return ;
	}

	CEtime s;
	CEtime e;
	s = p_reserve->start_time;
	e = p_reserve->end_time;

	switch (p_reserve->repeatability) {
	case EN_RESERVE_REPEATABILITY__NONE:
		return;
		break;

	case EN_RESERVE_REPEATABILITY__DAILY:
		s.addDay(1);
		e.addDay(1);
		break;

	case EN_RESERVE_REPEATABILITY__WEEKLY:
		s.addWeek(1);
		e.addWeek(1);
		break;

	default:
		_UTL_LOG_W ("invalid repeatability");
		return ;
		break;
	}

	bool r = addReserve (
				p_reserve->transport_stream_id,
				p_reserve->original_network_id,
				p_reserve->service_id,
				p_reserve->event_id,
				&s,
				&e,
				p_reserve->title_name.c_str(),
				p_reserve->service_name.c_str(),
				p_reserve->is_event_type,
				p_reserve->repeatability
			);

	if (r) {
		_UTL_LOG_I ("addReserve by repeatability -- success.");
	} else {
		_UTL_LOG_W ("addReserve by repeatability -- failure.");
	}
}

/**
 * 録画結果をリストに保存します
 */
void CRecManager::setResult (CRecReserve *p)
{
	if (!p) {
		return ;
	}


	for (int i = 0; i < RESULT_NUM_MAX; ++ i) {
		if (!m_results [i].is_used) {
			m_results [i] = *p;

			// 毎回書き込み
			saveResults ();

			return ;
		}
	}

	// m_results full
	// 最古のものを消します
	for (int i = 0; i < RESULT_NUM_MAX -1; ++ i) {
		m_results [i] = m_results [i + 1] ;		
	}

	m_results [RESULT_NUM_MAX -1].clear ();
	m_results [RESULT_NUM_MAX -1] = *p;


	// 毎回書き込み
	saveResults ();
}

bool CRecManager::checkRecordingEnd (void)
{
	if (!m_recording.is_used) {
		return false;
	}

	if (!(m_recording.state & RESERVE_STATE__NOW_RECORDING)) {
		return false;
	}

	CEtime current_time ;
	current_time.setCurrentTime();

	if (m_recording.end_time <= current_time) {

		// 録画 正常終了します
		if (m_recProgress == EN_REC_PROGRESS__NOW_RECORDING) {
			_UTL_LOG_I ("m_recProgress = EN_REC_PROGRESS__END_SUCCESS");


			// ######################################### //
			m_recProgress = EN_REC_PROGRESS__END_SUCCESS;
			// ######################################### //


			return true;
		}
	}

	return false;
}

void CRecManager::dumpReserves (void)
{
	_UTL_LOG_I (__PRETTY_FUNCTION__);

	for (int i = 0; i < RESERVE_NUM_MAX; ++ i) {
		if (m_reserves [i].is_used) {
			_UTL_LOG_I ("-----------------------------------------");
			_UTL_LOG_I ("-- index=[%d] --", i);
			m_reserves [i].dump();
		}
	}
}

void CRecManager::dumpResults (void)
{
	_UTL_LOG_I (__PRETTY_FUNCTION__);

	for (int i = 0; i < RESULT_NUM_MAX; ++ i) {
		if (m_results [i].is_used) {
			_UTL_LOG_I ("-----------------------------------------");
			m_results [i].dump();
		}
	}
}

void CRecManager::dumpRecording (void)
{
	_UTL_LOG_I (__PRETTY_FUNCTION__);

	if (m_recording.is_used) {
		_UTL_LOG_I ("-----------   now recording   -----------");
		m_recording.dump();
		_UTL_LOG_I ("\n");
	} else {
		_UTL_LOG_I ("not recording now...");
	}
}

void CRecManager::clearReserves (void)
{
	for (int i = 0; i < RESERVE_NUM_MAX; ++ i) {
		m_reserves [i].clear();
	}
}

void CRecManager::clearResults (void)
{
	for (int i = 0; i < RESULT_NUM_MAX; ++ i) {
		m_results [i].clear();
	}
}



//////////  CTunerControlIf::ITsReceiveHandler  //////////

bool CRecManager::onPreTsReceive (void)
{
	getExternalIf()->createExternalCp();

	uint32_t opt = getExternalIf()->getRequestOption ();
	opt |= REQUEST_OPTION__WITHOUT_REPLY;
	getExternalIf()->setRequestOption (opt);

	return true;
}

void CRecManager::onPostTsReceive (void)
{
	uint32_t opt = getExternalIf()->getRequestOption ();
	opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
	getExternalIf()->setRequestOption (opt);

	getExternalIf()->destroyExternalCp();
}

bool CRecManager::onCheckTsReceiveLoop (void)
{
	return true;
}

bool CRecManager::onTsReceived (void *p_ts_data, int length)
{
	switch (m_recProgress) {
	case EN_REC_PROGRESS__PRE_PROCESS: {
		_UTL_LOG_I ("EN_REC_PROGRESS__PRE_PROCESS");

		mp_outputBuffer = create_FileBufferedWriter (768 * 1024, m_recording_tmpfile);
		if (!mp_outputBuffer) {
			_UTL_LOG_E ("failed to init FileBufferedWriter.");

			RECORDING_NOTICE_t _notice = {m_recProgress, RESERVE_STATE__END_ERROR__INTERNAL_ERR};
			requestAsync (
				EN_MODULE_REC_MANAGER,
				EN_SEQ_REC_MANAGER__RECORDING_NOTICE,
				(uint8_t*)&_notice,
				sizeof(_notice)
			);

			// next
			m_recProgress = EN_REC_PROGRESS__END_ERROR;

		} else {

			struct OutputBuffer * const pFileBufferedWriter = mp_outputBuffer;
			mp_outputBuffer = create_TSParser( 8192, pFileBufferedWriter, 1);
			if (!mp_outputBuffer) {
				_UTL_LOG_E ("failed to init TS Parser.");
				OutputBuffer_release (pFileBufferedWriter);

				RECORDING_NOTICE_t _notice = {m_recProgress, RESERVE_STATE__END_ERROR__INTERNAL_ERR};
				requestAsync (
					EN_MODULE_REC_MANAGER,
					EN_SEQ_REC_MANAGER__RECORDING_NOTICE,
					(uint8_t*)&_notice,
					sizeof(_notice)
				);

				// next
				m_recProgress = EN_REC_PROGRESS__END_ERROR;

			}


			RECORDING_NOTICE_t _notice = {m_recProgress, RESERVE_STATE__INIT};
			requestAsync (
				EN_MODULE_REC_MANAGER,
				EN_SEQ_REC_MANAGER__RECORDING_NOTICE,
				(uint8_t*)&_notice,
				sizeof(_notice)
			);

			// next
			m_recProgress = EN_REC_PROGRESS__NOW_RECORDING;
			_UTL_LOG_I ("next  EN_REC_PROGRESS__NOW_RECORDING");
		}

		}
		break;

	case EN_REC_PROGRESS__NOW_RECORDING: {

		// recording
		int r = OutputBuffer_put (mp_outputBuffer, p_ts_data, length);
		if (r < 0) {
			_UTL_LOG_W ("TS write failed");
		}

		}
		break;

	case EN_REC_PROGRESS__END_SUCCESS: {
		_UTL_LOG_I ("EN_REC_PROGRESS__END_SUCCESS");

		RECORDING_NOTICE_t _notice = {m_recProgress, RESERVE_STATE__INIT};
		requestAsync (
			EN_MODULE_REC_MANAGER,
			EN_SEQ_REC_MANAGER__RECORDING_NOTICE,
			(uint8_t*)&_notice,
			sizeof(_notice)
		);

		// next
		m_recProgress = EN_REC_PROGRESS__POST_PROCESS;

		}
		break;

	case EN_REC_PROGRESS__END_ERROR: {
		_UTL_LOG_I ("EN_REC_PROGRESS__END_ERROR");

		RECORDING_NOTICE_t _notice = {m_recProgress, RESERVE_STATE__INIT};
		requestAsync (
			EN_MODULE_REC_MANAGER,
			EN_SEQ_REC_MANAGER__RECORDING_NOTICE,
			(uint8_t*)&_notice,
			sizeof(_notice)
		);

		// next
		m_recProgress = EN_REC_PROGRESS__POST_PROCESS;

		}
		break;

	case EN_REC_PROGRESS__POST_PROCESS: {
		_UTL_LOG_I ("EN_REC_PROGRESS__POST_PROCESS");

		if (mp_outputBuffer) {
			OutputBuffer_flush (mp_outputBuffer);
			OutputBuffer_release (mp_outputBuffer);
			mp_outputBuffer = NULL;
		}

		RECORDING_NOTICE_t _notice = {m_recProgress, RESERVE_STATE__INIT};
		requestAsync (
			EN_MODULE_REC_MANAGER,
			EN_SEQ_REC_MANAGER__RECORDING_NOTICE,
			(uint8_t*)&_notice,
			sizeof(_notice)
		);

		// next
		m_recProgress = EN_REC_PROGRESS__INIT;

		}
		break;

	default:
		break;
	}

	return true;
}


//--------------------------------------------------------------------------------

template <class Archive>
void serialize (Archive &archive, struct timespec &t)
{
	archive (
		cereal::make_nvp("tv_sec", t.tv_sec),
		cereal::make_nvp("tv_nsec", t.tv_nsec)
	);
}

template <class Archive>
void serialize (Archive &archive, CEtime &t)
{
	archive (cereal::make_nvp("m_time", t.m_time));
}

template <class Archive>
void serialize (Archive &archive, CRecReserve &r)
{
	archive (
		cereal::make_nvp("transport_stream_id", r.transport_stream_id),
		cereal::make_nvp("original_network_id", r.original_network_id),
		cereal::make_nvp("service_id", r.service_id),
		cereal::make_nvp("event_id", r.event_id),
		cereal::make_nvp("start_time", r.start_time),
		cereal::make_nvp("end_time", r.end_time),
		cereal::make_nvp("title_name", r.title_name),
		cereal::make_nvp("service_name", r.service_name),
		cereal::make_nvp("is_event_type", r.is_event_type),
		cereal::make_nvp("repeatability", r.repeatability),
		cereal::make_nvp("state", r.state),
		cereal::make_nvp("recording_start_time", r.recording_start_time),
		cereal::make_nvp("recording_end_time", r.recording_end_time),
		cereal::make_nvp("is_used", r.is_used)
	);
}

void CRecManager::saveReserves (void)
{
	std::stringstream ss;
	{
		cereal::JSONOutputArchive out_archive (ss);
		out_archive (CEREAL_NVP(m_reserves), sizeof(CRecReserve) * RESERVE_NUM_MAX);
	}

	std::string *p_path = mp_settings->getParams()->getRecReservesJsonPath();
	std::ofstream ofs (p_path->c_str(), std::ios::out);
	ofs << ss.str();

	ofs.close();
	ss.clear();
}

void CRecManager::loadReserves (void)
{
	std::string *p_path = mp_settings->getParams()->getRecReservesJsonPath();
	std::ifstream ifs (p_path->c_str(), std::ios::in);
	if (!ifs.is_open()) {
		_UTL_LOG_I("rec_reserves.json is not found.");
		return;
	}

	std::stringstream ss;
	ss << ifs.rdbuf();

	cereal::JSONInputArchive in_archive (ss);
	in_archive (CEREAL_NVP(m_reserves), sizeof(CRecReserve) * RESERVE_NUM_MAX);

	ifs.close();
	ss.clear();


	// CEtimeの値は直接 tv_sec,tvnsecに書いてるので toString用の文字はここで作ります
	for (int i = 0; i < RESERVE_NUM_MAX; ++ i) {
		m_reserves [i].start_time.updateStrings();
		m_reserves [i].end_time.updateStrings();
		m_reserves [i].recording_start_time.updateStrings();
		m_reserves [i].recording_end_time.updateStrings();
	}
}

void CRecManager::saveResults (void)
{
	std::stringstream ss;
	{
		cereal::JSONOutputArchive out_archive (ss);
		out_archive (CEREAL_NVP(m_results), sizeof(CRecReserve) * RESULT_NUM_MAX);
	}

	std::string *p_path = mp_settings->getParams()->getRecResultsJsonPath();
	std::ofstream ofs (p_path->c_str(), std::ios::out);
	ofs << ss.str();

	ofs.close();
	ss.clear();
}

void CRecManager::loadResults (void)
{
	std::string *p_path = mp_settings->getParams()->getRecResultsJsonPath();
	std::ifstream ifs (p_path->c_str(), std::ios::in);
	if (!ifs.is_open()) {
		_UTL_LOG_I("rec_results.json is not found.");
		return;
	}

	std::stringstream ss;
	ss << ifs.rdbuf();

	cereal::JSONInputArchive in_archive (ss);
	in_archive (CEREAL_NVP(m_results), sizeof(CRecReserve) * RESULT_NUM_MAX);

	ifs.close();
	ss.clear();


	// CEtimeの値は直接 tv_sec,tv_nsecに書いてるので toString用の文字はここで作ります
	for (int i = 0; i < RESULT_NUM_MAX; ++ i) {
		m_results [i].start_time.updateStrings();
		m_results [i].end_time.updateStrings();
		m_results [i].recording_start_time.updateStrings();
		m_results [i].recording_end_time.updateStrings();
	}
}
