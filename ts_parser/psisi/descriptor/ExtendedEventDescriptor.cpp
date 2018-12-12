#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ExtendedEventDescriptor.h"
#include "Utils.h"
#include "aribstr.h"


CExtendedEventDescriptor::CExtendedEventDescriptor (const CDescriptor &obj)
	:CDescriptor (obj)
	,descriptor_number (0)
	,last_descriptor_number (0)
	,length_of_items (0)
	,text_length (0)
{
	if (!isValid) {
		return;
	}

	memset (ISO_639_language_code, 0x00, sizeof(ISO_639_language_code));
	memset (text_char, 0x00, sizeof(text_char));
	items.clear();

	if (!parse()) {
		isValid = false;
	}
}

CExtendedEventDescriptor::~CExtendedEventDescriptor (void)
{
}

bool CExtendedEventDescriptor::parse (void)
{
	memset (ISO_639_language_code, 0x00, sizeof(ISO_639_language_code));
	memset (text_char, 0x00, sizeof(text_char));

	uint8_t *p = data;

	descriptor_number = (*p >> 4) & 0x0f;
	last_descriptor_number = *p & 0x0f;
	p += 1;

	memcpy (ISO_639_language_code, p, 3);
	p += 3;

	length_of_items = *p;
	p += 1;

	int itemLen = length_of_items;
	while (itemLen > 0) {

		CItem itm ;

		itm.item_description_length = *p;
		p += 1;
		memcpy (itm.item_description_char, p, itm.item_description_length);
		p += itm.item_description_length;

		itm.item_length = *p;
		p += 1;
		memcpy (itm.item_char, p, itm.item_length);
		p += itm.item_length;

		itemLen -= (1 + itm.item_description_length + 1 + itm.item_length) ;
		if (itemLen < 0) {
			puts ("invalid ExtendedEventDescriptor item");
			return false;
		}

		items.push_back (itm);
	}

	text_length = *p;
	p += 1;
	memcpy (text_char, p, text_length);
	p += text_length;

	// length check
	if (length != (p - data)) {
		return false;
	}

	return true;
}

void CExtendedEventDescriptor::dump (void) const
{
	printf ("%s\n", __PRETTY_FUNCTION__);

	char aribstr [MAXSECLEN];

	CDescriptor::dump (true);
	printf ("descriptor_number       [0x%x]\n", descriptor_number);
	printf ("last_descriptor_number  [0x%x]\n", last_descriptor_number);
	printf ("ISO_639_language_code   [%s]\n", ISO_639_language_code);
	printf ("length_of_items         [%d]\n", length_of_items);

	std::vector<CItem>::const_iterator iter_item = items.begin();
    for (; iter_item != items.end(); ++ iter_item) {
		printf ("\n--  item  --\n");
		printf ("item_description_length [%d]\n", iter_item->item_description_length);
		memset (aribstr, 0x00, MAXSECLEN);
		AribToString (aribstr, (const char*)iter_item->item_description_char, (int)iter_item->item_description_length);
		printf ("item_description_char   [%s]\n", aribstr);
		printf ("item_length             [%d]\n", iter_item->item_length);
		memset (aribstr, 0x00, MAXSECLEN);
		AribToString (aribstr, (const char*)iter_item->item_char, (int)iter_item->item_length);
		printf ("item_char               [%s]\n", aribstr);
	}

	printf ("text_length             [%d]\n", text_length);
	memset (aribstr, 0x00, MAXSECLEN);
	AribToString (aribstr, (const char*)text_char, (int)text_length);
	printf ("text_char               [%s]\n", aribstr);
}
