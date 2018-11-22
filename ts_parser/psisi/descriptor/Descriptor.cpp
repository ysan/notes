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
	if (len == 0) {
		isValid = false;
		return;
	}

	tag = *(pStart + 0);
	length = len;
	memset (data, 0x00, sizeof(data));
	memcpy (data, (pStart + 2), length);
}

CDescriptor::CDescriptor (const CDescriptor &obj)
	:tag (0)
	,length (0)
	,isValid (true)
{
	tag = obj.tag;
	length = obj.length;
	memset (data, 0x00, sizeof(data));
	memcpy (data, obj.data, length);
}

CDescriptor::~CDescriptor (void)
{
}

void CDescriptor::dump (void) const
{
	dump (true);
}

void CDescriptor::dump (bool isDataDump) const
{
	printf ("tag    [0x%02x]\n", tag);
	printf ("length [0x%02x]\n", length);
	if (isDataDump) {
		CUtils::dumper (data, length);
	}
}