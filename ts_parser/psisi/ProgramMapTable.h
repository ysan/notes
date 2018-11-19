#ifndef _PROGRAM_MAP_TABLE_H_
#define _PROGRAM_MAP_TABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <vector>

#include "Defs.h"
#include "TsCommonDefs.h"
#include "SectionParser.h"
#include "Descriptor.h"


#define PMT_FIX_LEN		(4) // reserved             3  bslbf
							// PCR_PID             13  uimsbf
							// reserved             4  bslbf
							// program_info_length 12 uimsbf


class CProgramMapTable : public CSectionParser
{
public:
	class CElement {
	public:
		class CStream {
			public:
			CStream (void)
				:stream_type (0)
				,elementary_PID (0)
				,ES_info_length (0)
			{
				descriptors.clear();
			}
			virtual ~CStream (void) {}


			uint8_t stream_type;
			uint16_t elementary_PID;
			uint16_t ES_info_length;
			std::vector <CDescriptor> descriptors;
		};

	public:
		CElement (void)
			:PCR_PID (0)
			,program_info_length (0)
		{
			descriptors.clear();
		}
		virtual ~CElement (void) {}


		uint16_t PCR_PID;
		uint16_t program_info_length;
		std::vector <CDescriptor> descriptors;
		std::vector <CStream> streams;
	};

public:
	CProgramMapTable (void);
	virtual ~CProgramMapTable (void);


	void onSectionComplete (const CSectionInfo *pCompSection) override;
	void dump (void) const;


private:
	bool parse (const CSectionInfo *pCompSection);
	void dump (const CSectionInfo *pCompSection) const;


	std::vector <CElement*> mElements;

};

#endif
