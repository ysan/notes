#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "TsParser.h"
#include "Utils.h"


CTsParser::CTsParser (void)
	:mpTop (NULL)
	,mpCurrent (NULL)
	,mpBottom (NULL)
	,mBuffSize (0)
	,mUnitSize (0)
	,mTOT (5)
	,mEIT_0x12 (65535)
	,mEIT_0x26 (65535)
	,mEIT_0x27 (65535)
{
}

CTsParser::~CTsParser (void)
{
}

void CTsParser::run (uint8_t *pBuff, size_t size)
{
	if ((!pBuff) || (size == 0)) {
		return ;
	}

	copyInnerBuffer (pBuff, size);
	checkUnitSize ();

	searchPAT ();
}

//TODO 全部ため込むかたち
bool CTsParser::copyInnerBuffer (uint8_t *pBuff, size_t size)
{
	if ((!pBuff) || (size == 0)) {
		return false;
	}

	size_t totalDataSize = (mpBottom - mpTop) + size;

	if (mBuffSize >= totalDataSize) {
		// 	実コピー
		memcpy (mpBottom, pBuff, size);
		mpCurrent = mpBottom;
		mpBottom += size;
printf ("copyInnerBuffer size=%lu remain=%lu\n", mBuffSize, mBuffSize-totalDataSize);
		return true;
	}

	// 残りサイズ足りないので確保 初回もここに

	size_t n = 512;
	while (n < (size + mBuffSize)) {
		n += n;
	}

	uint8_t *p = (uint8_t*) malloc (n);
	if (!p) {
		return false;
	}
	memset (p, 0x00, n);


	int m = 0;
	if (mpTop != NULL) {
		// 既にデータ入っている場合はそれをコピーして 元は捨てます
		m = mpBottom - mpTop;
		if (m > 0) {
			memcpy(p, mpTop, m);
		}
		free (mpTop);
		mpTop = NULL;
    }
	mpTop = p;
	mpBottom = p + m;
    mBuffSize = n;


	// 	実コピー
	memcpy (mpBottom, pBuff, size);
	mpCurrent = mpBottom;
	mpBottom += size;

printf ("copyInnerBuffer(malloc) top=%p bottom=%p size=%lu remain=%lu\n", mpTop, mpBottom, mBuffSize, mBuffSize-totalDataSize);
	return true;
}

bool CTsParser::checkUnitSize (void)
{
	int i;
	int m;
	int n;
//	int w;
	int count [320-188];

	uint8_t *pCur;

	pCur = mpCurrent;
	memset (count, 0x00, sizeof(count));

	while ((pCur+188) < mpBottom) {
		if (*pCur != SYNC_BYTE) {
			pCur ++;
			continue;
		}

		m = 320;
		if ((pCur+m) > mpBottom) {
			// mを 320満たないのでまでにbottomに切り詰め
			m = mpBottom - pCur;
		}
//printf("%p m=%d ", pCur, m);

		for (i = 188; i < m; i ++) {
			if(*(pCur+i) == SYNC_BYTE){
				// 今見つかったSYNC_BYTEから以降の(320-188)byte間に
				// SYNC_BYTEがあったら カウント
				count [i-188] += 1;
			}
		}

		pCur ++;

//for (int iii=0; iii < 320-188; iii++) {
// printf ("%d", count[iii]);
//}
//printf ("\n");
	}

	// 最大回数m をみてSYNC_BYTE出現間隔nを決める
	m = 0;
	n = 0;
	for (i = 188; i < 320; i ++) {
		if (m < count [i-188]) {
			m = count [i-188];
			n = i;
		}
	}
printf("max_interval count m=%d  max_interval n=%d\n", m, n);

	//TODO
//	w = m*n;
//	if ((m < 8) || ((w+3*n) < (mpBottom-mpCurrent))) {
//		return false;
//	}

	mUnitSize = n;
printf("mUnitSize %d\n", n);

	return true;
}

uint8_t * CTsParser::getSyncTopAddr (uint8_t *pTop, uint8_t *pBtm, size_t unitSize) const
{
	if ((!pTop) || (!pBtm) || (unitSize == 0)) {
		return NULL;
	}

	int i;
	uint8_t *pWork = NULL;

	pWork = pTop;
	pBtm -= unitSize * 8;

	while (pWork <= pBtm) {
		if (*pWork == SYNC_BYTE) {
			for (i = 0; i < 8; ++ i) {
				if (*(pWork+(unitSize*(i+1))) != SYNC_BYTE) {
					break;
				}
			}
			// 以降 8回全てSYNC_BYTEで一致したら pWorkは先頭だ
			if (i == 8) {
				return pWork;
			}
		}
		pWork ++;
	}

	return NULL;
}

void CTsParser::getTsHeader (ST_TS_HEADER *pDst, uint8_t* pSrc) const
{
	if ((!pSrc) || (!pDst)) {
		return;
	}

	pDst->sync                         =   *(pSrc+0);
	pDst->transport_error_indicator    =  (*(pSrc+1) >> 7) & 0x01;
	pDst->payload_unit_start_indicator =  (*(pSrc+1) >> 6) & 0x01;
	pDst->transport_priority           =  (*(pSrc+1) >> 5) & 0x01;
	pDst->pid                          = ((*(pSrc+1) & 0x1f) << 8) | *(pSrc+2);
	pDst->transport_scrambling_control =  (*(pSrc+3) >> 6) & 0x03;
	pDst->adaptation_field_control     =  (*(pSrc+3) >> 4) & 0x03;
	pDst->continuity_counter           =   *(pSrc+3)       & 0x0f;
}

void CTsParser::dumpTsHeader (const ST_TS_HEADER *p) const
{
	if (!p) {
		return ;
	}
	printf (
		"TsHeader: sync:0x%02x trans_err:0x%02x start_ind:0x%02x prio:0x%02x pid:0x%04x scram:0x%02x adap:0x%02x cont:0x%02x\n",
		p->sync,
		p->transport_error_indicator,
		p->payload_unit_start_indicator,
		p->transport_priority,
		p->pid,
		p->transport_scrambling_control,
		p->adaptation_field_control,
		p->continuity_counter
	);
}

bool CTsParser::searchPAT (void)
{
	ST_TS_HEADER stTsHdr = {0};
	uint8_t *p = NULL; //work
	uint8_t *pPayload = NULL;
	uint8_t *pCur = mpCurrent;
	uint8_t *pBtm = mpBottom;
	size_t unitSize = mUnitSize;
	size_t payloadSize = 0;
	bool isCheck = false;
	CProgramAssociationTable::CTable *pPatTable = NULL;


	while ((pCur+unitSize) < pBtm) {
		if ((*pCur != SYNC_BYTE) || (*(pCur+unitSize) != SYNC_BYTE)) {
			p = getSyncTopAddr (pCur, pBtm, unitSize);
			if (!p) {
				return false;
			}

			// sync update
			pCur = p;
		}

		getTsHeader (&stTsHdr, pCur);

		switch (stTsHdr.pid) {
		case 0x0000: // PAT
puts ("###############  PAT  ###############");
CUtils::dumper (pCur, 188);
dumpTsHeader (&stTsHdr);
			isCheck = true;
			break;

		case 0x0014: // TOT
puts ("###############  TOT  ###############");
CUtils::dumper (pCur, 188);
dumpTsHeader (&stTsHdr);

			isCheck = true;
			break;

		case 0x0012: // EIT
puts ("###############  EIT  ###############");
CUtils::dumper (pCur, 188);
dumpTsHeader (&stTsHdr);
			isCheck = true;
			break;

		case 0x0026: // EIT
puts ("###############  EIT  ###############");
CUtils::dumper (pCur, 188);
dumpTsHeader (&stTsHdr);
			isCheck = true;
			break;

		case 0x0027: // EIT
puts ("###############  EIT  ###############");
CUtils::dumper (pCur, 188);
dumpTsHeader (&stTsHdr);
			isCheck = true;
			break;

		case 0x0010: // NIT
puts ("###############  NIT  ###############");
CUtils::dumper (pCur, 188);
dumpTsHeader (&stTsHdr);
			isCheck = true;
			break;

		default:
//puts ("1111");
//mPAT.dumpTable (mPatTable, 32);
			// check PMT
			pPatTable = &mPatTable [0];
			for (int i = 0; i < 32; ++ i) {
				if (!pPatTable->isUsed) {
					continue;
				}
				if (pPatTable->program_number != 0) {
					if (stTsHdr.pid == pPatTable->program_map_PID) {
puts ("###############  PMT  ###############");
CUtils::dumper (pCur, 188);
dumpTsHeader (&stTsHdr);
						isCheck = true;
						break;
					}
				}

				++ pPatTable ;
			}

			break;
		}

		if (isCheck) {
			pPayload = pCur + TS_HEADER_LEN; // ts header 4byte分進める

			// adaptation_field_control 2bit
			// 00 ISO/IECによる将来の使用のために予約されている。
			// 01 アダプテーションフィールドなし、ペイロードのみ
			// 10 アダプテーションフィールドのみ、ペイロードなし
			// 11 アダプテーションフィールドの次にペイロード
			if ((stTsHdr.adaptation_field_control & 0x02) == 0x02) {
				// アダプテーションフィールド付き
				pPayload += *pPayload + 1; // lengthとそれ自身の1byte分進める
			}

			// TTS(Timestamped TS)(total192bytes) や
			// FEC(Forward Error Correction:順方向誤り訂正)(total204bytes)
			// は除外します
			payloadSize = TS_PACKET_LEN - (pPayload - pCur);



			if (stTsHdr.pid == 0x0000) {

				mPAT.checkSection (&stTsHdr, pPayload, payloadSize);

				memset (mPatTable, 0x00, sizeof(mPatTable));

				int n = mPAT.getTableNum ();
				mPAT.getTable (mPatTable, 32);
				mPAT.dumpTable (mPatTable, n);

			} else if (stTsHdr.pid == 0x0014) {

				mTOT.checkSection (&stTsHdr, pPayload, payloadSize);

			} else if (stTsHdr.pid == 0x0012) {

				mEIT_0x12.checkSection (&stTsHdr, pPayload, payloadSize);

			} else if (stTsHdr.pid == 0x0026) {

				mEIT_0x26.checkSection (&stTsHdr, pPayload, payloadSize);

			} else if (stTsHdr.pid == 0x0027) {

				mEIT_0x27.checkSection (&stTsHdr, pPayload, payloadSize);

			} else if (stTsHdr.pid == 0x0010) {

				mNIT.checkSection (&stTsHdr, pPayload, payloadSize);

			} else if (stTsHdr.pid == pPatTable->program_map_PID) {

				if (pPatTable->mpPMT) {
					pPatTable->mpPMT->checkSection (&stTsHdr, pPayload, payloadSize);
				}
				
			}

			isCheck = false;
		}

		pCur += unitSize;
	}


	return true;
}
