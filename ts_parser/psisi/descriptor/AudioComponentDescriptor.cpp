#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "AudioComponentDescriptor.h"
#include "Utils.h"
#include "aribstr.h"


CAudioComponentDescriptor::CAudioComponentDescriptor (const CDescriptor &obj)
	:CDescriptor (obj)
	,reserved_future_use (0)
	,stream_content (0)
	,component_type (0)
	,component_tag (0)
	,stream_type (0)
	,simulcast_group_tag (0)
	,ES_multi_lingual_flag (0)
	,main_component_flag (0)
	,quality_indicator (0)
	,sampling_rate (0)
	,reserved_future_use2 (0)
{
	if (!isValid) {
		return;
	}

	memset (ISO_639_language_code, 0x00, sizeof(ISO_639_language_code));
	memset (ISO_639_language_code_2, 0x00, sizeof(ISO_639_language_code));
	memset (text_char, 0x00, sizeof(text_char));

	if (!parse()) {
		isValid = false;
	}
}

CAudioComponentDescriptor::~CAudioComponentDescriptor (void)
{
}

bool CAudioComponentDescriptor::parse (void)
{
	memset (ISO_639_language_code, 0x00, sizeof(ISO_639_language_code));
	memset (ISO_639_language_code_2, 0x00, sizeof(ISO_639_language_code_2));
	memset (text_char, 0x00, sizeof(text_char));

	uint8_t *p = data;

	reserved_future_use =  (*p >> 4) & 0x0f;
	stream_content = *p & 0x0f;
	p += 1;
	component_type = *p;
	p += 1;
	component_tag = *p;
	p += 1;
	stream_type = *p;
	p += 1;
	simulcast_group_tag = *p;
	p += 1;
	ES_multi_lingual_flag = (*p >> 7) & 0x01;
	main_component_flag = (*p >> 6) & 0x01;
	quality_indicator = (*p >> 5) & 0x03;
	sampling_rate = *p & 0x07;
	reserved_future_use2 = *p & 0x01;
	p += 1;

	memcpy (ISO_639_language_code, p, 3);
	p += 3 ;

	if (ES_multi_lingual_flag) {
		memcpy (ISO_639_language_code_2, p, 3);
		p += 3 ;
	}

	memcpy (text_char, p, length - (p - data));
	p += length - (p - data);

	// length check
	if (length != (p - data)) {
		return false;
	}

	return true;
}

void CAudioComponentDescriptor::dump (void) const
{
	char aribstr [MAXSECLEN];

	CDescriptor::dump (true);

	printf ("stream_content              [0x%02x]\n", stream_content);
	printf ("component_type              [0x%02x][%s]\n",
		component_type, CTsCommon::getAudioComponentType(component_type));
	printf ("component_tag               [0x%02x]\n", component_tag);
	printf ("stream_type                 [0x%02x]\n", stream_type);
	printf ("simulcast_group_tag         [0x%02x]\n", simulcast_group_tag);
	printf ("ES_multi_lingual_flag       [0x%02x][%s]\n",
		ES_multi_lingual_flag, ES_multi_lingual_flag ? "二ヶ国語" : "-");
	printf ("main_component_flag         [0x%02x][%s]\n",
		main_component_flag, main_component_flag ? "主" : "副");
	printf ("quality_indicator           [0x%02x][%s]\n",
		quality_indicator, CTsCommon::getAudioQuality(quality_indicator));
	printf ("sampling_rate               [0x%02x][%s]\n",
		sampling_rate, CTsCommon::getAudioSamplingRate(sampling_rate));

	printf ("ISO_639_language_code       [%s]\n", (char*)ISO_639_language_code);

	if (ES_multi_lingual_flag) {
		printf ("ISO_639_language_code_2 [%s]\n", (char*)ISO_639_language_code_2);
	}

	memset (aribstr, 0x00, MAXSECLEN);
	AribToString (aribstr, (const char*)text_char, (int)(strlen((char*)text_char)));
	printf ("text_char                   [%s]\n", aribstr);
}
