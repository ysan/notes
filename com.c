#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>


typedef void (*P_COM_CALLBACK) (int argc, char* argv[]);
typedef struct com_info {
	char *pszCom;
	char *pszDesc;
	P_COM_CALLBACK pComCallback;
	struct com_info *pNext;

} ST_COM_INFO;


ST_COM_INFO gstAribInvestigateTable[];
ST_COM_INFO gstTestTable[];

void GetPath (int argc, char* argv[])
{
//TODO
	fprintf (stdout, "%s done.\n", __func__);
}
void GetTargetName (int argc, char* argv[])
{
//TODO
	fprintf (stdout, "%s done.\n", __func__);
}
void GetLIBS (int argc, char* argv[])
{
//TODO
	fprintf (stdout, "%s done.\n", __func__);
}

void test (int argc, char* argv[])
{
//TODO
	fprintf (stdout, "%s done.\n", __func__);
}


ST_COM_INFO gstComTopTable[] = {
	{
		"A",
		"arib investigate",
		NULL,
		&gstAribInvestigateTable[0],
	},
	{
		"T",
		"test test test",
		NULL,
		&gstTestTable[0],
	},
	{
		NULL,
		NULL,
		NULL,
		NULL,
	},
};

ST_COM_INFO gstAribInvestigateTable[] = {
	{
		"GetPath",
		"get path",
		GetPath,
		NULL,
	},
	{
		"GetTargetName",
		"get target name",
		GetTargetName,
		NULL,
	},
	{
		"GetLIBS",
		"get libs",
		GetLIBS,
		NULL,
	},
	{
		NULL,
		NULL,
		NULL,
		NULL,
	},
};

ST_COM_INFO gstTestTable[] = {
	{
		"test",
		"do test",
		test,
		NULL,
	},
	{
		NULL,
		NULL,
		NULL,
		NULL,
	},
};



void deleteLF (char *pszBuff)
{
	if (!pszBuff) {
		return;
	}

	if (*(pszBuff + (strlen(pszBuff) -1)) == '\n') {

		// 改行コードLF削除
		*(pszBuff + (strlen(pszBuff) -1)) = '\0';

		// CRLFの場合 CRも削除
		if (*(pszBuff + (strlen(pszBuff) -1)) == '\r') {
			*(pszBuff + (strlen(pszBuff) -1)) = '\0';
		}
	}
}

void deleteHeadSp (char *pszBuff)
{
	if (!pszBuff) {
		return;
	}

	int i = 0;

	while (1) {
		if (*pszBuff == ' ') {
			if ((int)strlen(pszBuff) > 1) {
				i = 0;
				while (*(pszBuff+i)) {
					*(pszBuff+i) = *(pszBuff+i+1);
					i ++;
				}

			} else {
				// 全文字スペースだった
				*pszBuff = 0x00;
			}
		} else {
			break;
		}
	}
}

void deleteTailSp (char *pszBuff)
{
	if (!pszBuff) {
		return;
	}

	int nLen = (int)strlen(pszBuff);
	if (nLen == 0) {
		return;
	}

	nLen --;
	while (nLen >= 0) {
		if (*(pszBuff+nLen) == ' ') {
			*(pszBuff+nLen) = 0x00;
		} else {
			break;
		}
		nLen --;
	}
}

void showList (const ST_COM_INFO *pTable, const char *pszDesc)
{
	if (!pTable) {
		return;
	}

	const ST_COM_INFO *pWorkTable = NULL;

	fprintf (stdout, "\n  ------ %s ------\n", pszDesc ? pszDesc: "???");

	pWorkTable = pTable;
	int i = 0;
	for (i = 0; pWorkTable->pszCom; i ++) {
		fprintf (stdout, "  %-20s -- %-30s\n", pWorkTable->pszCom, pWorkTable->pszDesc);
		pWorkTable ++;
	}
	fprintf (stdout, "\n");
}

void loop (const ST_COM_INFO *pTable, const char *pszConsole, const char *pszDesc)
{
	if (!pTable) {
		return;
	}

	showList (pTable, pszDesc);

	const ST_COM_INFO *pWorkTable = NULL;
	bool isMatch = false;
	bool isBack = false;
	char szLine [256] = {0};
	while (1) {
		fprintf (stdout, "%s >> ", pszConsole ? pszConsole: "???"); // console
		fflush (stdout);

		isMatch = false;
		isBack = false;
		memset (szLine, 0x00, sizeof (szLine));
		read (STDIN_FILENO, szLine, sizeof(szLine));

		deleteLF (szLine);
		deleteHeadSp (szLine);
		deleteTailSp (szLine);

		if (strlen(szLine) == 0) {
			continue;
		}

		pWorkTable = pTable;
		while (pWorkTable->pszCom) {
			if (
				(strlen(pWorkTable->pszCom) == strlen(szLine)) &&
				(!strncmp (pWorkTable->pszCom, szLine, strlen(szLine)))
			) {
				// コマンドが一致した
				isMatch = true;

				if (pWorkTable->pComCallback) {
//TODO 引数あるときの対応 szLine
					(void) (pWorkTable->pComCallback) (0, NULL);

				} else {
					if (pWorkTable->pNext) {
						loop (pWorkTable->pNext, pWorkTable->pszCom, pWorkTable->pszDesc);
						showList (pTable, pszDesc);
					}
				}

			} else if ((strlen(".") == strlen(szLine)) && (!strncmp (".", szLine, strlen(szLine)))) {
				isBack = true;
				break;
			}

			pWorkTable ++;
		}

		if (isBack) {
			break;
		}

		if (!isMatch) {
			fprintf (stdout, "invalid command...\n");
		}
	}

}

int main (void)
{
	fprintf (stdout, "#===========================================#\n");
	fprintf (stdout, "#                commander                  #\n");
	fprintf (stdout, "#===========================================#\n");


	loop (&gstComTopTable[0], "top", "top list");

	fprintf (stdout, "exit commander.\n");

	exit (EXIT_SUCCESS);
}

