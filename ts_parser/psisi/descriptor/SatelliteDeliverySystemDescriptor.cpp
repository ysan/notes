#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "SatelliteDeliverySystemDescriptor.h"
#include "Utils.h"
#include "aribstr.h"


CSatelliteDeliverySystemDescriptor::CSatelliteDeliverySystemDescriptor (const CDescriptor &obj)
	:CDescriptor (obj)
	,frequency (0)
	,orbital_position (0)
	,west_east_flag (0)
	,polarisation (0)
	,modulation (0)
	,symbol_rate (0)
	,FEC_inner (0)
{
	if (!isValid) {
		return;
	}

	if (!parse()) {
		isValid = false;
	}
}

CSatelliteDeliverySystemDescriptor::~CSatelliteDeliverySystemDescriptor (void)
{
}

bool CSatelliteDeliverySystemDescriptor::parse (void)
{
	uint8_t *p = data;

	memcpy (&frequency, p, 4);
	p += 4;
	orbital_position = *p << 8 | *(p+1);
	p += 2;
	west_east_flag = (*p >> 7) & 0x01;
	polarisation = (*p >> 5) & 0x03;
	modulation = *p | 0x1f;
	p += 1;
	memcpy (&symbol_rate, p, 4);
	symbol_rate = (symbol_rate >> 4) & 0x0fffffff;
	FEC_inner = *(p+3) & 0x0f;
	p += 4;

	// length check
	if (length != (p - data)) {
		return false;
	}

	return true;
}

void CSatelliteDeliverySystemDescriptor::dump (void) const
{
	printf ("%s\n", __PRETTY_FUNCTION__);

	char aribstr [MAXSECLEN];

	CDescriptor::dump (true);

	printf ("frequency        [0x%08x]\n", frequency);
	printf ("orbital_position [0x%04x]\n", orbital_position);
	printf ("west_east_flag   [0x%02x]\n", west_east_flag);
	printf ("polarisation     [0x%02x]\n", polarisation);
	printf ("modulation       [0x%02x]\n", modulation);
	printf ("symbol_rate      [0x%08x]\n", symbol_rate);
	printf ("FEC_inner        [0x%02x]\n", FEC_inner);
}
