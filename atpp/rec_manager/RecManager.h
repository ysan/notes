#ifndef _REC_MANAGER_H_
#define _REC_MANAGER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include <iostream>
#include <fstream>
#include <string>

#include "threadmgr_if.h"
#include "threadmgr_util.h"

#include "ThreadMgrIf.h"
#include "ThreadMgrExternalIf.h"

#include "ThreadMgrBase.h"
#include "ThreadMgr.h"

#include "Utils.h"
#include "Settings.h"
#include "RecManagerIf.h"
#include "TsAribCommon.h"

#include "TunerControlIf.h"
#include "PsisiManagerIf.h"
#include "ChannelManagerIf.h"
#include "EventScheduleManagerIf.h"

#include "cereal/cereal.hpp"
#include "cereal/archives/json.hpp"


using namespace ThreadManager;


#define RESERVE_NUM_MAX		(50)
#define RESULT_NUM_MAX		(20)

#define RESERVE_STATE__INIT								(0x00000000)
#define RESERVE_STATE__REMOVE_RESERVE					(0x00000001)
#define RESERVE_STATE__START_TIME_PASSED				(0x00000002)
#define RESERVE_STATE__REQ_START_RECORDING				(0x00000004)
#define RESERVE_STATE__NOW_RECORDING					(0x00000008)
#define RESERVE_STATE__FORCE_STOP						(0x00000010)
#define RESERVE_STATE__END_SUCCESS						(0x00000020)
#define RESERVE_STATE__END_ERROR__ALREADY_PASSED		(0x00000040)
#define RESERVE_STATE__END_ERROR__HDD_FREE_SPACE_LOW	(0x00000080)
#define RESERVE_STATE__END_ERROR__INTERNAL_ERR			(0x00000100)


typedef enum {
	EN_REC_PROGRESS__INIT = 0,
	EN_REC_PROGRESS__PRE_PROCESS,
	EN_REC_PROGRESS__NOW_RECORDING,
	EN_REC_PROGRESS__END_SUCCESS,
	EN_REC_PROGRESS__END_ERROR,
	EN_REC_PROGRESS__POST_PROCESS,
} EN_REC_PROGRESS;


struct reserve_state_pair {
	uint32_t state;
	const char *psz_reserveState;
};

const char * getReserveState (uint32_t state);


class CRecReserve {
public:
	CRecReserve (void) {
		clear ();
	}
	virtual ~CRecReserve (void) {
		clear ();
	}

	bool operator == (const CRecReserve &obj) const {
		if (
			this->transport_stream_id == obj.transport_stream_id &&
			this->original_network_id == obj.original_network_id &&
			this->service_id == obj.service_id &&
			this->event_id == obj.event_id
		) {
			return true;
		} else {
			return false;
		}
	}

	bool operator != (const CRecReserve &obj) const {
		if (
			this->transport_stream_id != obj.transport_stream_id ||
			this->original_network_id != obj.original_network_id ||
			this->service_id != obj.service_id ||
			this->event_id != obj.event_id
		) {
			return true;
		} else {
			return false;
		}
	}

	uint16_t transport_stream_id;
	uint16_t original_network_id;
	uint16_t service_id;

	uint16_t event_id;
	CEtime start_time;
	CEtime end_time;

	std::string title_name;
	std::string service_name;

	bool is_event_type ;
	EN_RESERVE_REPEATABILITY repeatability;

	uint32_t state;

	CEtime recording_start_time;
	CEtime recording_end_time;

	bool is_used;


	void set (
		uint16_t _transport_stream_id,
		uint16_t _original_network_id,
		uint16_t _service_id,
		uint16_t _event_id,
		CEtime* p_start_time,
		CEtime* p_end_time,
		const char *psz_title_name,
		const char *psz_service_name,
		bool _is_event_type,
		EN_RESERVE_REPEATABILITY _repeatability
	)
	{
		this->transport_stream_id = _transport_stream_id;
		this->original_network_id = _original_network_id;
		this->service_id = _service_id;
		this->event_id = _event_id;
		if (p_start_time) {
			this->start_time = *p_start_time;
		}
		if (p_end_time) {
			this->end_time = *p_end_time;
		}
		if (psz_title_name) {
			this->title_name = psz_title_name ;
		}
		if (psz_service_name) {
			this->service_name = psz_service_name ;
		}
		this->is_event_type = _is_event_type;
		this->repeatability = _repeatability;
		this->state = RESERVE_STATE__INIT;
		this->is_used = true;
	}

	void clear (void) {
		transport_stream_id = 0;
		original_network_id = 0;
		service_id = 0;
		event_id = 0;
		start_time.clear();
		end_time.clear();	
		title_name.clear();
		title_name.shrink_to_fit();
		service_name.clear();
		service_name.shrink_to_fit();
		is_event_type = false;
		repeatability = EN_RESERVE_REPEATABILITY__NONE;
		state = RESERVE_STATE__INIT;
		recording_start_time.clear();
		recording_end_time.clear();	
		is_used = false;
	}

	void dump (void) const {
		_UTL_LOG_I (
			"tsid:[0x%04x] org_nid:[0x%04x] svcid:[0x%04x] evtid:[0x%04x]",
			transport_stream_id,
			original_network_id,
			service_id,
			event_id
		);
		_UTL_LOG_I (
			"time:[%s - %s] event_type:[%d] repeat:[%s] state:[%s]",
			start_time.toString(),
			end_time.toString(),
			is_event_type,
			repeatability == 0 ? "NONE" :
				repeatability == 1 ? "DAILY" :
					repeatability == 2 ? "WEEKLY" :
						repeatability == 3 ? "AUTO" : "???",
			getReserveState (state)
		);
		_UTL_LOG_I ("title:[%s]", title_name.c_str());
		_UTL_LOG_I ("service_name:[%s]", service_name.c_str());

		if (state != RESERVE_STATE__INIT) {
			struct timespec d = recording_end_time - recording_start_time;
			CEtime::CDiff diff (&d);
			_UTL_LOG_I ("recording_time:[%s]", diff.toString());
		}
	}
};


class CRecManager
	:public CThreadMgrBase
	,public CTunerControlIf::ITsReceiveHandler
{
public:
	CRecManager (char *pszName, uint8_t nQueNum);
	virtual ~CRecManager (void);


	void onReq_moduleUp (CThreadMgrIf *pIf);
	void onReq_moduleDown (CThreadMgrIf *pIf);
	void onReq_checkLoop (CThreadMgrIf *pIf);
	void onReq_checkEventLoop (CThreadMgrIf *pIf);
	void onReq_recordingNotice (CThreadMgrIf *pIf);
	void onReq_startRecording (CThreadMgrIf *pIf);
	void onReq_addReserve_currentEvent (CThreadMgrIf *pIf);
	void onReq_addReserve_event (CThreadMgrIf *pIf);
	void onReq_addReserve_eventHelper (CThreadMgrIf *pIf);
	void onReq_addReserve_manual (CThreadMgrIf *pIf);
	void onReq_removeReserve (CThreadMgrIf *pIf);
	void onReq_removeReserve_byIndex (CThreadMgrIf *pIf);
	void onReq_stopRecording (CThreadMgrIf *pIf);
	void onReq_dumpReserves (CThreadMgrIf *pIf);

	void onReceiveNotify (CThreadMgrIf *pIf) override;


private:
	bool addReserve (
		uint16_t _transport_stream_id,
		uint16_t _original_network_id,
		uint16_t _service_id,
		uint16_t _event_id,
		CEtime* p_start_time,
		CEtime* p_end_time,
		const char *psz_title_name,
		const char *psz_service_name,
		bool _is_event_type,
		EN_RESERVE_REPEATABILITY repeatability=EN_RESERVE_REPEATABILITY__NONE
	);
	int getReserveIndex (
		uint16_t _transport_stream_id,
		uint16_t _original_network_id,
		uint16_t _service_id,
		uint16_t _event_id
	);
	bool removeReserve (int index, bool isConsiderRepeatability, bool isApplyResult);
	CRecReserve* searchAscendingOrderReserve (CEtime *p_start_time_rf);
	bool isExistEmptyReserve (void) const;
	CRecReserve *findEmptyReserve (void);
	bool isDuplicateReserve (const CRecReserve* p_reserve) const;
	bool isOverrapTimeReserve (const CRecReserve* p_reserve) const;
	void checkReserves (void);
	void refreshReserves (uint32_t state);
	bool pickReqStartRecordingReserve (void);
	void setResult (CRecReserve *p);
	bool checkRecordingEnd (void);
	void checkRepeatability (const CRecReserve *p_reserve);
	void dumpReserves (void);
	void dumpResults (void);
	void dumpRecording (void);
	void clearReserves (void);
	void clearResults (void);


	// CTunerControlIf::ITsReceiveHandler
	bool onPreTsReceive (void) override;
	void onPostTsReceive (void) override;
	bool onCheckTsReceiveLoop (void) override;
	bool onTsReceived (void *p_ts_data, int length) override;


	void saveReserves (void);
	void loadReserves (void);

	void saveResults (void);
	void loadResults (void);


	ST_SEQ_BASE mSeqs [EN_SEQ_REC_MANAGER__NUM]; // entity

	CSettings *mp_settings;
	
	uint8_t m_tunerNotify_clientId;
	int m_tsReceive_handlerId;
	uint8_t m_patDetectNotify_clientId;
	uint8_t m_eventChangeNotify_clientId;

	EN_REC_PROGRESS m_recProgress; // tuneThreadと共有する とりあえず排他はいれません

	CRecReserve m_reserves [RESERVE_NUM_MAX];
	CRecReserve m_results [RESULT_NUM_MAX];
	CRecReserve m_recording;

	char m_recording_tmpfile [PATH_MAX];
	struct OutputBuffer *mp_outputBuffer;

};

#endif
