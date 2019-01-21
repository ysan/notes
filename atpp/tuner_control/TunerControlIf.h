#ifndef _TUNER_CONTROL_IF_H_
#define _TUNER_CONTROL_IF_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrExternalIf.h"
#include "modules.h"


using namespace ThreadManager;

enum {
	EN_SEQ_TUNER_CONTROL_MODULE_UP = 0,
	EN_SEQ_TUNER_CONTROL_MODULE_DOWN,
	EN_SEQ_TUNER_CONTROL_TUNE,
	EN_SEQ_TUNER_CONTROL_TUNE_START,
	EN_SEQ_TUNER_CONTROL_TUNE_STOP,
	EN_SEQ_TUNER_CONTROL_REG_TS_RECEIVE_HANDLER,
	EN_SEQ_TUNER_CONTROL_UNREG_TS_RECEIVE_HANDLER,

	EN_SEQ_TUNER_CONTROL_NUM,
};

class CTunerControlIf : public CThreadMgrExternalIf
{
public:
	class ITsReceiveHandler {
	public:
		virtual ~ITsReceiveHandler (void) {};
		virtual bool onPreTsReceive (void) = 0;
		virtual void onPostTsReceive (void) = 0;
		virtual bool onCheckTsReceiveLoop (void) = 0;
		virtual bool onTsReceived (void *p_ts_data, int length) = 0;
	};

public:
	explicit CTunerControlIf (CThreadMgrExternalIf *pIf) : CThreadMgrExternalIf (pIf) {
	};

	virtual ~CTunerControlIf (void) {
	};


	bool reqModuleUp (void) {
		return requestAsync (EN_MODULE_TUNER_CONTROL, EN_SEQ_TUNER_CONTROL_MODULE_UP);
	};

	bool reqModuleDown (void) {
		return requestAsync (EN_MODULE_TUNER_CONTROL, EN_SEQ_TUNER_CONTROL_MODULE_DOWN);
	};

	bool reqTune (uint32_t freq) {
		uint32_t f = freq;
		return requestAsync (EN_MODULE_TUNER_CONTROL, EN_SEQ_TUNER_CONTROL_TUNE, (uint8_t*)&f, sizeof(f));
	};

	bool reqTuneSync (uint32_t freq) {
		uint32_t f = freq;
		return requestSync (EN_MODULE_TUNER_CONTROL, EN_SEQ_TUNER_CONTROL_TUNE, (uint8_t*)&f, sizeof(f));
	};

	bool reqTuneStop (void) {
		return requestAsync (EN_MODULE_TUNER_CONTROL, EN_SEQ_TUNER_CONTROL_TUNE_STOP);
	};

	bool reqTuneStopSync (void) {
		return requestSync (EN_MODULE_TUNER_CONTROL, EN_SEQ_TUNER_CONTROL_TUNE_STOP);
	};

	bool reqRegisterTsReceiveHandler (ITsReceiveHandler **p_handler) {
		ITsReceiveHandler **p = p_handler;
		return requestSync (EN_MODULE_TUNER_CONTROL, EN_SEQ_TUNER_CONTROL_REG_TS_RECEIVE_HANDLER, (uint8_t*)p, sizeof(p));
	};

	bool reqUnregisterTsReceiveHandler (int client_id) {
		int _id = client_id;
		return requestSync (EN_MODULE_TUNER_CONTROL, EN_SEQ_TUNER_CONTROL_REG_TS_RECEIVE_HANDLER, (uint8_t*)&_id, sizeof(_id));
	};

};

#endif
