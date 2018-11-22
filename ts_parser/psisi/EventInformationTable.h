#ifndef _EVENT_INFORMATION_TABLE_H_
#define _EVENT_INFORMATION_TABLE_H_

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
#include "ShortEventDescriptor.h"


#define EIT_FIX_LEN			(6) // transport_stream_id         16  uimsbf
								// original_network_id         16  uimsbf
								// segment_last_section_number  8  uimsbf
								// last_table_id                8  uimsbf

#define EIT_EVENT_FIX_LEN	(12) // event_id                16  uimsbf
								// start_time               40  mjd
								// duration                 24  bcdtime
								// running_status            3  uimsbf
								// free_CA_mode              1  bslbf
								// descriptors_loop_length  12  uimsbf


class CEventInformationTable : public CSectionParser
{
public:
	class CTable {
	public:
		class CEvent {
		public:
			CEvent (void)
				:event_id (0)
				,start_time (0)
				,duration (0)
				,running_status (0)
				,free_CA_mode (0)
				,descriptors_loop_length (0)
			{
				descriptors.clear();
			}
			virtual ~CEvent (void) {}

			uint16_t event_id;
			uint64_t start_time;
			uint32_t duration;
			uint8_t running_status;
			uint8_t free_CA_mode;
			uint16_t descriptors_loop_length;
			std::vector <CDescriptor> descriptors;
		};

	public:
		CTable (void)
			:transport_stream_id (0)
			,original_network_id (0)
			,segment_last_section_number (0)
			,last_table_id (0)
		{
			events.clear();
		}
		virtual ~CTable (void) {}

		uint16_t transport_stream_id;
		uint16_t original_network_id;
		uint8_t segment_last_section_number;
		uint8_t last_table_id;
		std::vector <CEvent> events;
	};
public:
	explicit CEventInformationTable (size_t poolSize);
	virtual ~CEventInformationTable (void);

	void onSectionComplete (const CSectionInfo *pCompSection) override;

	void dumpTables (void) const;
	void dumpTable (const CTable* pTable) const;
	void clear (void);

private:
	bool parse (const CSectionInfo *pCompSection, CTable* pOutTable);
	void releaseTables (void);

	std::vector <CTable*> mTables;

};

#endif
