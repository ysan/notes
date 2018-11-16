#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "Defs.h"
#include "TsParser.h"
#include "Utils.h"


int main (int argc, char **argv)
{
	int nReadSize = 0;
	int nReadTotal = 0;
	uint8_t buff [65536] = {0};

	CTsParser tp;


	while (1) {

		memset (buff, 0x00, sizeof(buff));
		nReadSize = CUtils::readFile (STDIN_FILENO, buff, sizeof(buff));
		if (nReadSize < 0) {
			fprintf (stdout, "CUtils::readFile() is failure.\n");
		} else if (nReadSize == 0) {
			break;
		}

		nReadTotal += nReadSize;

printf ("%d\n", nReadSize);

		tp.run (buff, nReadSize);


	}

	printf ("nReadTotal %d\n", nReadTotal);

	exit (EXIT_SUCCESS);
}
