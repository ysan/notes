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

#include "CommandServerIf.h"
#include "TunerControlIf.h"
#include "PsisiManagerIf.h"
#include "RecManagerIf.h"
#include "ChannelManagerIf.h"
#include "EventScheduleManagerIf.h"
#include "EventSearchIf.h"

#include "it9175_extern.h"

#include "Utils.h"
#include "modules.h"
#include "Settings.h"


using namespace ThreadManager;


int main (int argc, char *argv[])
{
	if (argc != 2) {
		printf ("[main] unexpected arguments.\n");
		exit (EXIT_FAILURE);
	}

	std::string settings_json = argv[1];
	CSettings *s = CSettings::getInstance ();
	s->load (settings_json);


	initLogStdout(); // threadmgr log init
	it9175_initLogDefault();

	// syslog initialize
	if (s->getParams()->isSyslogOutput()) {
		initSyslog(); // threadmgr syslog output
		it9175_initSyslog();
		CUtils::initSyslog();
	}


	s->getParams()->dump ();


	CThreadMgr *p_mgr = CThreadMgr::getInstance();
	if (!p_mgr->setup (getModules(), EN_MODULE_NUM)) {
		exit (EXIT_FAILURE);
	}

	p_mgr->getExternalIf()->createExternalCp();


	CCommandServerIf *p_comSvrIf = new CCommandServerIf (p_mgr->getExternalIf());
	CTunerControlIf *p_tunerCtlIf = new CTunerControlIf (p_mgr->getExternalIf());
	CPsisiManagerIf *p_psisiMgrIf = new CPsisiManagerIf (p_mgr->getExternalIf());
	CRecManagerIf *p_recMgrIf = new CRecManagerIf (p_mgr->getExternalIf());
	CChannelManagerIf *p_chMgrIf = new CChannelManagerIf (p_mgr->getExternalIf());
	CEventScheduleManagerIf *p_schedMgrIf = new CEventScheduleManagerIf (p_mgr->getExternalIf());
	CEventSearchIf *p_searchIf = new CEventSearchIf (p_mgr->getExternalIf());


	uint32_t opt = p_mgr->getExternalIf()->getRequestOption ();
//	opt |= REQUEST_OPTION__WITH_TIMEOUT_MSEC;
//	opt &= 0x0000ffff; // clear timeout val
//	opt |= 1000 << 16; // set timeout 1sec
	opt |= REQUEST_OPTION__WITHOUT_REPLY;
	p_mgr->getExternalIf()->setRequestOption (opt);


	// modules up
	p_comSvrIf-> reqModuleUp ();
	p_tunerCtlIf-> reqModuleUp ();
	p_psisiMgrIf->reqModuleUp();
	p_recMgrIf->reqModuleUp();
	p_chMgrIf->reqModuleUp();
	p_schedMgrIf->reqModuleUp();
	p_searchIf->reqModuleUp();



	pause ();


	p_mgr->teardown();
	delete p_mgr;
	p_mgr = NULL;


	// syslog finalize
	if (s->getParams()->isSyslogOutput()) {
		finalizSyslog(); // threadmgr syslog output
		it9175_finalizSyslog();
		CUtils::finalizSyslog();
	}


	exit (EXIT_SUCCESS);
}
