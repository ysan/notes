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

CDescriptor::~CDescriptor (void)
{
}

void CDescriptor::dump (void) const
{
	printf ("descriptor tag    0x%02x\n", tag);
	printf ("descriptor length 0x%02x\n", length);
	CUtils::dumper (data, length);

if (tag == 0x4d) {
 printf ("%s\n", (char*)(data+4));
}

}