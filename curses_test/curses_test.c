#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "mycommon.h"
#include "cursutil.h"
#include "cursutil_string.h"


int main( void )
{

	InitCursUtil();
	StartCursUtil();


	int i = SetStringItemCursUtil (
		EN_CUTL_DISP_STR_KIND_FIXED,
		NULL,
		10,
		10,
		"%s",
		"testttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt"
	);
	LOG_I( "get idx=[%d]\n", i );

	int j = SetStringItemCursUtil (
		EN_CUTL_DISP_STR_KIND_SCROLL,
		NULL,
		20,
		30,
		"ssssss"
	);
	LOG_I( "get idx=[%d]\n", j );

	sleep(5);

	ModStringCursUtil(i,"AAA");
	ModPosCursUtil(i,5,5 );

	sleep(5);

	ClearIdxStringCursUtil( i );

	while (1) {
		sleep(5);
	}


	FinalizeCursUtil();

	exit( EXIT_SUCCESS );
}
