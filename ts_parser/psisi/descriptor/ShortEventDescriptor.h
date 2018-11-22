#ifndef _SHORT_EVENT_DESCRIPTOR_H_
#define _SHORT_EVENT_DESCRIPTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "Defs.h"
#include "TsCommonDefs.h"
#include "Descriptor.h"


class CShortEventDescriptor : public CDescriptor
{
public:
	static const uint8_t TAG;
	CShortEventDescriptor (const CDescriptor &obj); // not explicit
	virtual ~CShortEventDescriptor (void);


	void dump (void) const override;

	uint8_t ISO_639_language_code [4];
	uint8_t event_name_length;
	uint8_t event_name_char [0xff];
	uint8_t text_length; 
	uint8_t text_char [0xff];
};

#endif
