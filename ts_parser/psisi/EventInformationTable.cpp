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

puts ("EIT section complete.");


	dump (pCompSection);
}

void CEventInformationTable::dump (void) const
{
	CSectionInfo *pLatest = getLatestCompleteSection ();
	if (!pLatest) {
		return ;
	}

	dump (pLatest);
}

void CEventInformationTable::dump (const CSectionInfo *pSectInfo) const
{
	if (!pSectInfo) {
		return;
	}

	uint8_t *p = pSectInfo->getDataPartAddr ();







}
