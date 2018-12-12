#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ServiceListDescriptor.h"
#include "Utils.h"
#include "aribstr.h"


CServiceListDescriptor::CServiceListDescriptor (const CDescriptor &obj)
	:CDescriptor (obj)
{
	if (!isValid) {
		return;
	}

	services.clear();

	if (!parse()) {
		isValid = false;
	}
}

CServiceListDescriptor::~CServiceListDescriptor (void)
{
}

bool CServiceListDescriptor::parse (void)
{
	uint8_t *p = data;

	int serviceLen = length;
	while (serviceLen > 0) {
		CService sv;

		sv.service_id = *p << 8 | *(p+1);
		p += 2;
		sv.service_type = *p;
		p += 1;

		serviceLen -= 3 ;
		if (serviceLen < 0) {
			puts ("invalid ServiceListDescriptor service");
			return false;
		}

		services.push_back (sv);
	}

	// length check
	if (length != (p - data)) {
		return false;
	}

	return true;
}

void CServiceListDescriptor::dump (void) const
{
	printf ("%s\n", __PRETTY_FUNCTION__);

	CDescriptor::dump (true);

	std::vector<CService>::const_iterator iter_sv = services.begin();
	for (; iter_sv != services.end(); ++ iter_sv) {
		printf ("\n--  service  --\n");
		printf ("service_id   [0x%04x]\n", iter_sv->service_id);
		printf ("service_type [0x%02x]\n", iter_sv->service_type);
	}
}
