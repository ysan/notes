#ifndef _DESCRIPTOR_H_
#define _DESCRIPTOR_H_

#include "Defs.h"
#include "TsCommonDefs.h"


class CDescriptor {
public:
	CDescriptor (const uint8_t *pStart);
	~CDescriptor (void);

	void dump (void) const;

	uint8_t tag;
	uint8_t length;
	uint8_t data [0xff];

	bool isValid;
};

#endif
