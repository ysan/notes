#ifndef _PSISI_MANAGER_H_
#define _PSISI_MANAGER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "threadmgr_if.h"
#include "threadmgr_util.h"

#include "ThreadMgrIf.h"
#include "ThreadMgrExternalIf.h"

#include "ThreadMgrBase.h"
#include "ThreadMgr.h"

#include "Utils.h"
#include "PsisiManagerIf.h"
//#include "PsisiManagerTables.h"
#include "TsParserListener.h"

#include "TunerControlIf.h"
#include "TsParser.h"


using namespace ThreadManager;


class CPsisiManager : public CThreadMgrBase, CTunerControlIf::ITsReceiveHandler
{
public:
	CPsisiManager (char *pszName, uint8_t nQueNum);
	virtual ~CPsisiManager (void);


	void moduleUp (CThreadMgrIf *pIf);
	void moduleDown (CThreadMgrIf *pIf);


private:
	// CTunerControlIf::ITsReceiveHandler
	bool onPreTsReceive (void) override;
	void onPostTsReceive (void) override;
	bool onCheckTsReceiveLoop (void) override;
	bool onTsReceived (void *p_ts_data, int length) override;


	ST_SEQ_BASE mSeqs [EN_SEQ_PSISI_MANAGER_NUM]; // entity

	CTsParser m_parser;
	CTsParserListener m_parser_listener;

	int m_ts_receive_handler_id;
};

#endif
