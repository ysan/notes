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

int CProgramAssociationTable::getElementNum (const CSectionInfo *pSectInfo) const
{
	if (!pSectInfo) {
		return 0;
	}

	int nDataPartLen = (int)pSectInfo->getDataPartLen ();
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

int CProgramAssociationTable::getElementNum (void) const
{
	CSectionInfo *pLatest = getLatestCompleteSection ();
	if (!pLatest) {
		return 0;
	}

	return getElementNum (pLatest);
}

bool CProgramAssociationTable::getElement (CElement outArr[], int outArrSize) const
{
	CSectionInfo *pLatest = getLatestCompleteSection ();
	if (!pLatest) {
		return false;
	}

	if ((!outArr) || (outArrSize == 0)) {
		return false;
	}

	uint8_t *p = pLatest->getDataPartAddr();
	int n = (int) getElementNum (pLatest);
	if (n <= 0) {
		return false;
	}

	int i = 0;
	while (i != n && outArrSize != 0) {
		outArr[i].program_number = ((*p << 8) | *(p+1)) & 0xffff;

		if (outArr[i].program_number == 0) {
			outArr[i].network_PID = (((*(p+2) & 0x01f) << 8) | *(p+3)) & 0xffff;
		} else {
			outArr[i].program_map_PID = (((*(p+2) & 0x01f) << 8) | *(p+3)) & 0xffff;
			if (!outArr[i].mpPMT) {
				outArr[i].mpPMT = new CProgramMapTable();
			}
		}

		outArr[i].isUsed = true;

		p += 4;
		++ i;
		-- outArrSize;
	}

	if (n > 0 && outArrSize == 0) {
		printf ("warn:  ProgramAssociationTable is not get all.\n");
	}

	return true;
}

void CProgramAssociationTable::dumpElement (const CElement inArr[], int arrSize) const
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
		printf ("PMTparser: %p ", inArr[i].mpPMT);
		printf ("%s", inArr[i].isUsed ? "used " : "noused");
		printf ("\n");
	}
}
