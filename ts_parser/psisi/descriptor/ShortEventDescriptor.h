#ifndef _SHORT_EVENT_DESCRIPTOR_H_
#define _SHORT_EVENT_DESCRIPTOR_H_

#include "Defs.h"
#include "TsCommonDefs.h"
#include "Descriptor.h"


class CShortEventDescriptor : public CDescriptor
{
public:
	CShortEventDescriptor (void);
	virtual ~CShortEventDescriptor (void);


	uint8_t ISO_639_language_code [3];
	uint8_t event_name_length;
	uint8_t *event_name_char;
	uint8_t text_length; 
	uint8_t *text_char; 

};

#endif
