#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ProgramAssociationTable.h"
#include "Utils.h"



CProgramAssociationTable::CProgramAssociationTable (void)
{
}

CProgramAssociationTable::~CProgramAssociationTable (void)
{
}

uint16_t CProgramAssociationTable::getElementNum (const CSectionInfo *pSectInfo) const
{
	if (!pSectInfo) {
		return 0;
	}

	uint16_t nDataPartLen = pSectInfo->getDataPartLen ();
	if (nDataPartLen < 4) {
		printf ("invalid ProgramAssociationTable data\n");
		return 0;
	}

	if ((nDataPartLen % 4) != 0) {
		// 1ループの大きさは4の倍数
		printf ("invalid ProgramAssociationTable data (not multiples of 4)\n");
		return 0;
	}

	return nDataPartLen / 4;
}

uint16_t CProgramAssociationTable::getElementNum (void) const
{
	CSectionInfo *pLatest = getLatestCompleteSection ();
	if (!pLatest) {
		return 0;
	}

	return getElementNum (pLatest);
}

bool CProgramAssociationTable::getElement (ST_PAT_ELEMENT *pstOut, uint16_t nOutSetNum) const
{
	CSectionInfo *pLatest = getLatestCompleteSection ();
	if (!pLatest) {
		return false;
	}

	if ((!pstOut) || (nOutSetNum == 0)) {
		return false;
	}

	uint8_t *p = pLatest->getDataPartAddr();
	uint16_t n = getElementNum (pLatest);
	if (n <= 0) {
		return false;
	}

	while (n != 0 && nOutSetNum != 0) {
		pstOut->program_number = ((*p << 8) | *(p+1)) & 0xffff;

		if (pstOut->program_number == 0) {
			pstOut->network_PID = (((*(p+2) & 0x01f) << 8) | *(p+3)) & 0xffff;
		} else {
			pstOut->program_map_PID = (((*(p+2) & 0x01f) << 8) | *(p+3)) & 0xffff;
			if (!pstOut->mpPMT) {
				pstOut->mpPMT = new CProgramMapTable();
			}
		}

		pstOut->isUsed = true;

		p += 4;
		-- n;
		++ pstOut;
		-- nOutSetNum;
	}

	if ((n > 0) || (nOutSetNum == 0)) {
		printf ("warn:  ProgramAssociationTable is not get all.\n");
	}

	return true;
}

void CProgramAssociationTable::dumpElement (const ST_PAT_ELEMENT *pstIn, uint16_t nInSetNum) const
{
	if ((!pstIn) || (nInSetNum == 0)) {
		return ;
	}

	for (int i = 0; i < nInSetNum; ++ i) {
		printf ("program_number 0x%04x  ", pstIn->program_number);
		if (pstIn->program_number == 0) {
			printf ("network_PID     0x%04x  ", pstIn->network_PID);
		} else {
			printf ("program_map_PID 0x%04x  ", pstIn->program_map_PID);
		}
		printf ("PMT parser:%p ", pstIn->mpPMT);
		printf ("%s", pstIn->isUsed ? "used " : "noused");
		printf ("\n");

		++ pstIn;
	}
}
