#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "Descriptor.h"
#include "Utils.h"


CDescriptor::CDescriptor (const uint8_t *pStart)
	:tag (0)
	,length (0)
	,isValid (true)
{
	if (!pStart) {
		isValid = false;
		return ;
	}
	uint8_t len = *(pStart + 1);
	if (len > 0xff || len <= 2) {
		isValid = false;
		return;
	}

	tag = *(pStart + 0);
	length = len;
	memset (data, 0x00, sizeof(data));
	memcpy (data, (pStart + 2), length);
}

CDescriptor::~CDescriptor (void)
{
}
