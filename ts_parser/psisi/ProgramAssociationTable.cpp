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

bool CProgramAssociationTable::getElement (CElement outArr[], uint16_t outArrSize) const
{
	CSectionInfo *pLatest = getLatestCompleteSection ();
	if (!pLatest) {
		return false;
	}

	if ((!outArr) || (outArrSize == 0)) {
		return false;
	}

	uint8_t *p = pLatest->getDataPartAddr();
	uint16_t n = getElementNum (pLatest);
	if (n <= 0) {
		return false;
	}

	while (n != 0 && outArrSize != 0) {
		outArr[n].program_number = ((*p << 8) | *(p+1)) & 0xffff;

		if (outArr[n].program_number == 0) {
			outArr[n].network_PID = (((*(p+2) & 0x01f) << 8) | *(p+3)) & 0xffff;
		} else {
			outArr[n].program_map_PID = (((*(p+2) & 0x01f) << 8) | *(p+3)) & 0xffff;
			if (!outArr[n].mpPMT) {
				outArr[n].mpPMT = new CProgramMapTable();
			}
		}

		outArr[n].isUsed = true;

		p += 4;
		-- n;
		-- outArrSize;
	}

	if (n > 0 && outArrSize == 0) {
		printf ("warn:  ProgramAssociationTable is not get all.\n");
	}

	return true;
}

void CProgramAssociationTable::dumpElement (const CElement inArr[], uint16_t arrSize) const
{
	if ((!inArr) || (arrSize == 0)) {
		return ;
	}

	for (int i = 0; i < arrSize; ++ i) {
		printf ("program_number 0x%04x  ", inArr[i].program_number);
		if (inArr[i].program_number == 0) {
			printf ("network_PID     0x%04x  ", inArr[i].network_PID);
		} else {
			printf ("program_map_PID 0x%04x  ", inArr[i].program_map_PID);
		}
		printf ("PMT parser:%p ", inArr[i].mpPMT);
		printf ("%s", inArr[i].isUsed ? "used " : "noused");
		printf ("\n");
	}
}
