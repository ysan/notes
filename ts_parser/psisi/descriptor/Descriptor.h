#ifndef _DESCRIPTOR_H_
#define _DESCRIPTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

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
