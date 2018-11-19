#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ProgramMapTable.h"
#include "Utils.h"


CProgramMapTable::CProgramMapTable (void)
{
}

CProgramMapTable::~CProgramMapTable (void)
{
}

typedef enum {
	EN_PMT_VARDATA_PARSE_SATGE__STREAM_TYPE,		// stream_type
	EN_PMT_VARDATA_PARSE_SATGE__ELEM_PID,			// elementary_PID
	EN_PMT_VARDATA_PARSE_SATGE__ELEM_PID_d,
	EN_PMT_VARDATA_PARSE_SATGE__ES_INFO_LEN,		// ES_info_length
	EN_PMT_VARDATA_PARSE_SATGE__ES_INFO_LEN_d,
	EN_PMT_VARDATA_PARSE_SATGE__DESC,				// descriptor
} EN_PMT_VARDATA_PARSE_SATGE;


void CProgramMapTable::onSectionComplete (const CSectionInfo *pCompSection)
{
	dump ();
	parse (pCompSection);
}

bool CProgramMapTable::parse (const CSectionInfo *pCompSection)
{
	if (!pCompSection) {
		return false;
	}

	uint8_t *p = NULL; // work

	CElement* pElem = new CElement ();

	p = pCompSection->getDataPartAddr();
	pElem->PCR_PID = (*p & 0x1f) << 8 | *(p+1);
	pElem->program_info_length = (*(p+2) & 0xf) << 8 | *(p+3);

	p += PMT_FIX_LEN;
	int n = (int)pElem->program_info_length;
	while (n > 0) {
		CDescriptor desc (p);
		if (!desc.isValid) {
			puts ("invalid desc");
			return false;
		}
		pElem->descriptors.push_back (desc);
		n -= (2 + *(p + 1));
		p += (2 + *(p + 1));
	}

	p += PMT_FIX_LEN + pElem->program_info_length;
	int streamLen = (int) (const_cast<CSectionInfo*>(pCompSection)->getHeader()->section_length - pElem->program_info_length - SECTION_HEADER_FIX_LEN - SECTION_CRC32_LEN - PMT_FIX_LEN);
	if (streamLen <= PMT_STREAM_FIX_LEN) {
		puts ("invalid PMTstream");
		return false;
	}

	while (streamLen > 0) {

		CElement::CStream strm ;

		strm.stream_type = *p;
		strm.elementary_PID = (*(p+1) & 0x1f) << 8 | *(p+2);
		strm.ES_info_length = (*(p+3) & 0xf) << 8 | *(p+4);

		p += PMT_STREAM_FIX_LEN;
		int n = (int)strm.ES_info_length ;
		while (n > 0) {
			CDescriptor desc (p);
			if (!desc.isValid) {
				puts ("invalid desc");
				return false;
			}
			strm.descriptors.push_back (desc);
			n -= (2 + *(p + 1));
			p += (2 + *(p + 1));
		}

		pElem->streams.push_back (strm);

		streamLen -= (PMT_STREAM_FIX_LEN + strm.ES_info_length) ;
		if (streamLen < 0) {
			puts ("invalid PMTstream");
			return false;
		}
	}

	mElements.push_back (pElem);

	return true;
}

void CProgramMapTable::dumpElements (void) const
{
	if (mElements.size() == 0) {
		return;
	}

	std::vector<CElement*>::const_iterator iter = mElements.begin(); 
	for (; iter != mElements.end(); ++ iter) {

		std::vector<CDescriptor>::const_iterator iter_desc = (*iter)->descriptors.begin();
		std::vector<CElement::CStream>::const_iterator iter_strm = (*iter)->streams.begin();

	}

}

void CProgramMapTable::dump (void) const
{
	CSectionInfo *pLatest = getLatestCompleteSection ();
	if (!pLatest) {
		return ;
	}

	dump (pLatest);
}

void CProgramMapTable::dump (const CSectionInfo *pSectionInfo) const
{
	if (!pSectionInfo) {
		return ;
	}

	uint8_t *p = NULL; // work

	p = pSectionInfo->getDataPartAddr();
	uint16_t PCR_PID = (*p & 0x1f) << 8 | *(p+1);
	printf ("PCR_PID 0x%04x\n", PCR_PID);
	uint16_t program_info_length = (*(p+2) & 0xf) << 8 | *(p+3);
	printf ("program_info_length %d\n", program_info_length);


	p = pSectionInfo->getDataPartAddr() + PMT_FIX_LEN;
	uint16_t n = program_info_length;
	EN_DESCRIPTOR_PARSE_SATGE enDesc = EN_DESCRIPTOR_PARSE_SATGE__TAG;
	uint16_t descDataLen = 0;

	while (n > 0) {
		if (enDesc == EN_DESCRIPTOR_PARSE_SATGE__TAG) {
			printf ("descriptor_tag %d\n", *p);
			enDesc = EN_DESCRIPTOR_PARSE_SATGE__LEN;

		} else if (enDesc == EN_DESCRIPTOR_PARSE_SATGE__LEN) {
			printf ("descriptor_length %d\n", *p);
			descDataLen = *p;
			enDesc = EN_DESCRIPTOR_PARSE_SATGE__DATA;

		} else {
			// EN_DESCRIPTOR_PARSE_SATGE__DATA
			printf ("[0x%02x]", *p);
			-- descDataLen;
			if (descDataLen == 0) {
				enDesc = EN_DESCRIPTOR_PARSE_SATGE__TAG;
				printf ("\n");
			}
		}
	
		-- n;
		++ p;
	}

	p = pSectionInfo->getDataPartAddr() + PMT_FIX_LEN + program_info_length;
	uint16_t remainVarDataLen = const_cast<CSectionInfo*>(pSectionInfo)->getHeader()->section_length - program_info_length - SECTION_HEADER_FIX_LEN - SECTION_CRC32_LEN - PMT_FIX_LEN;
	EN_PMT_VARDATA_PARSE_SATGE enVar = EN_PMT_VARDATA_PARSE_SATGE__STREAM_TYPE;
	uint16_t esInfoLen = 0;

	while (remainVarDataLen > 0) {
puts ("----");
		if (enVar == EN_PMT_VARDATA_PARSE_SATGE__STREAM_TYPE) {
			printf ("stream_type 0x%x\n", *p);
			enVar = EN_PMT_VARDATA_PARSE_SATGE__ELEM_PID;

		} else if (enVar == EN_PMT_VARDATA_PARSE_SATGE__ELEM_PID) {
			uint16_t elemPID  = (*p & 0x1f) << 8 | *(p+1);
			printf ("elementary_PID 0x%02x\n", elemPID);
			enVar = EN_PMT_VARDATA_PARSE_SATGE__ELEM_PID_d;

		} else if (enVar == EN_PMT_VARDATA_PARSE_SATGE__ELEM_PID_d) {
			enVar = EN_PMT_VARDATA_PARSE_SATGE__ES_INFO_LEN;

		} else if (enVar == EN_PMT_VARDATA_PARSE_SATGE__ES_INFO_LEN) {
			esInfoLen = (*p & 0xf) << 8 | *(p+1);
			printf ("ES_info_length %d\n", esInfoLen);
			enVar = EN_PMT_VARDATA_PARSE_SATGE__ES_INFO_LEN_d;

		} else if (enVar == EN_PMT_VARDATA_PARSE_SATGE__ES_INFO_LEN_d) {
			enVar = EN_PMT_VARDATA_PARSE_SATGE__DESC;

		} else {
			// EN_PMT_VARDATA_PARSE_SATGE__DESC
			n = esInfoLen;
			while (n > 0) {
				if (enDesc == EN_DESCRIPTOR_PARSE_SATGE__TAG) {
					printf ("descriptor_tag %d\n", *p);
					enDesc = EN_DESCRIPTOR_PARSE_SATGE__LEN;

				} else if (enDesc == EN_DESCRIPTOR_PARSE_SATGE__LEN) {
					printf ("descriptor_length %d\n", *p);
					descDataLen = *p;
					enDesc = EN_DESCRIPTOR_PARSE_SATGE__DATA;

				} else {
					// EN_DESCRIPTOR_PARSE_SATGE__DATA
					printf ("[0x%02x]", *p);
					-- descDataLen;
					if (descDataLen == 0) {
						enDesc = EN_DESCRIPTOR_PARSE_SATGE__TAG;
						printf ("\n");
					}
				}
	
				-- n;
				++ p;
			}

			remainVarDataLen -= esInfoLen;
			enVar = EN_PMT_VARDATA_PARSE_SATGE__STREAM_TYPE;
			continue;
		}

		-- remainVarDataLen;
		++ p;
	}

}
