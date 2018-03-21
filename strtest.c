#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <ctype.h>

#define BUFFSIZE 1024


int SedFunc(
	const char*,
	const char*,
	const char*,
	char*,
	size_t
);

int SplitFunc(
	const char*,
	const char*,
	char**,
	int *
);

int reg_string_match( const char*, size_t, const char* );

int base64enc(
	const unsigned char*,
	unsigned int,
	char*,
	unsigned int,
	unsigned int*
);

int base64dec(
	const char*,
	unsigned int,
	unsigned char*,
	unsigned int,
	unsigned int*
);


int main( void )
{
	char *ppp = "AAA";
	char aaa[] = "0000023452";
	char bbb[] = "4326g4g6y4g54gr";
	char ccc[5] = "rr";
	int num = 0;
	char *ptr = NULL;
	int i = 0;
	int j = 0;
	char szCmd[10+1];
	char *pszWk = NULL;
	char szTmp0[] = "モジュールID：CmgDmcPrc_init";
	char szTmp1[] = "/hulft/vmlcisel/temp";
	char szTmp2[] = "ABCDRRRRRRRRRR";
	char szTmp3[] = "ABCDTHDFHGABHH.,bnJK,ABCD:IYNHABCDDFABCD";
	char szTmp4[] = "ABCD";
	char szTmp5[BUFFSIZE];
	char szTmp6[10+1];
	char *pszTmp6;
	char szTmp7[64];
	char szRegStr[64];
	regex_t preg;
	size_t nmatch = 5;
	regmatch_t pmatch[nmatch];
	char szStrB64d[128];
	char szStrB64d2[BUFFSIZE];
	FILE *pFp;
	char *pszTmptmp = NULL;
	int c = 0;
	char sz1wd[1+1];
	char *ppo = NULL;
	int nDiv = 0;
	char *saaa[2] = {"asdfg", "rttyui"};
	char **pp = NULL;
	char *sbbb = "bbbbbnnnnn";
	char **ppt = NULL;
	unsigned char buf[20000];
	int nSize;


	pp = saaa;
	printf("%d\n",sizeof(pp));
	printf("%s\n",*pp);

	ppt = &sbbb;
	printf("%s\n",*ppt+3);

	printf("%d\n",sizeof(aaa));
	printf("%d\n",sizeof("aaa"));
	printf("%d\n",sizeof(szTmp5));
	printf("%d\n",sizeof(ppp));
	printf("%d\n",sizeof(*ppp));
	printf("%d\n",sizeof(ccc));

	num = atoi(aaa);
	printf("atoi: %d\n",num);


	puts(bbb);
	printf("strtol: %d\n",(int)strtol(bbb,&ptr,10));
	printf("strtol_rtn: %s\n",ptr);


	ptr = strtok(bbb,"g");
	while(ptr != NULL){
		printf("strtok: %s\n",ptr);
		ptr = strtok(NULL,"g");
	}






	for(i=0;i<sizeof(szTmp0)-1;i++){
		printf("%d  isascii(%c)=%d\n",i,szTmp0[i],isascii(szTmp0[i]));
	}
	printf("i -> %d\n",i);
	printf("size %d\n",sizeof(szTmp0));
	



	memset(szTmp6,'\0',sizeof(szTmp6));
	strcpy(szTmp6,"01234567890123456789");
	/*  strcpy(szTmp6,"0123456789");*/
	printf("szTmp6 size: %d  len: %d  str: %s\n",sizeof(szTmp6),strlen(szTmp6),szTmp6);
	szTmp6[5] = '\0';
	printf("szTmp6 size: %d  len: %d  str: %s\n",sizeof(szTmp6),strlen(szTmp6),szTmp6);
	pszTmp6 = szTmp6;
	for(i=0;i<20;i++){ printf("%c\n",*(pszTmp6+i)); }
	printf("\n\n");



	memset(szCmd,'\0',sizeof(szCmd));

	pszWk = strrchr( szTmp1, '/' );
	if(pszWk!=NULL){
		strncpy( szCmd, pszWk+1, (size_t)(sizeof(szCmd) - 1U) );
	}
	else {
		strncpy( szCmd, szTmp1, (size_t)(sizeof(szCmd) - 1U) );
	}
	printf("%s\n",szCmd);
	printf("\n");



	for(i=0;szTmp2[i]!='\0';i++){
		printf("%d [%c]\n",i,szTmp2[i]);
	}
	printf("%d\n\n",i);

	i = 0;
	while(szTmp2[i]!='\0'){
		printf("%d [%c]\n",i,szTmp2[i]);
		i ++;
	}
	printf("%d\n\n",i);
  
	for(i=0;i<sizeof(szTmp2);i++){
		printf("%d [%c]\n",i,szTmp2[i]);
	}
	printf("%d\n\n",i);
	
	for(i=0;i<4;){
		printf("%d\n",i);
		i ++;
	}
	printf("%d\n\n",i);




	printf("org str:%s\n",szTmp3);
	printf("for conv:%s\n",szTmp4);
	printf("after conv:%s\n\n","0000000000000000");

	SedFunc(szTmp3, szTmp4, "0000000000000000", szTmp5, sizeof(szTmp5));
	printf("%s\n",szTmp5);


	/* regex */
	/**********************************************************************/
	putchar('\n');
	memset(szTmp7, '\0', sizeof(szTmp7));
	memset(szRegStr, '\0', sizeof(szRegStr));
	strncpy(szTmp7, "#include <regex.h>", sizeof(szTmp7)-1);
	/*  strncpy(szRegStr, "^ *# *include +<.+\\.h> *$", sizeof(szRegStr)-1);*/
	strncpy(szRegStr, "(inc).+(\\.h)", sizeof(szRegStr)-1);
	puts(szTmp7);
	puts(szRegStr);
	for(i=0; i<strlen(szRegStr); i++){
		printf("%d [%c]\n",i,szRegStr[i]);
	}
	putchar('\n');

	/*******************/
	if(regcomp(&preg, szRegStr, REG_EXTENDED|REG_NEWLINE)!=0){
		printf("regex compile failed.\n");
		exit(1);
	}

	/* nmatchはマッチの回数以上の値であること */
	if(regexec(&preg, szTmp7, nmatch, pmatch, 0)!=0){
		printf("No match.\n"); 
	}
	else {
		for (i=0; i<nmatch; i++){
			printf("Match position = %d, %d, str = [", (int)pmatch[i].rm_so, (int)pmatch[i].rm_eo); 
			if(pmatch[i].rm_so>=0&&pmatch[i].rm_eo>=0){
				for(j=pmatch[i].rm_so; j<pmatch[i].rm_eo; j++){ putchar(szTmp7[j]); }
			}
			putchar(']');
			putchar('\n');
		}
	}
	regfree(&preg);

	/*******************/
	/* マッチか否かのみ知りたい場合はREG_NOSUBも指定すること */
	if(regcomp(&preg, szRegStr, REG_EXTENDED|REG_NEWLINE|REG_NOSUB)!=0){
		printf("regex compile failed.\n");
		exit(1);
	}
	
	if(regexec(&preg, szTmp7, 0, 0, 0)!=0){
		printf("No match.\n");
	}
	else {
		printf("Match.\n");
	}
	regfree(&preg);
	
	/**********************************************************************/


	reg_string_match( "aaaa#include <stdio.h>", 22, "^a*# *include +<........$" );


	int nn = 0;
	memset( szStrB64d, '\0', sizeof(szStrB64d) );
	base64enc( (unsigned char*)"ABCDEFG", 7, szStrB64d, sizeof(szStrB64d), (unsigned int*)&nn );
	printf( "szStrB64d:[%s]\n", szStrB64d );
	printf( "encode size:[%d]\n", nn );

	memset( szStrB64d, '\0', sizeof(szStrB64d) );
	base64enc( (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26, szStrB64d, sizeof(szStrB64d), (unsigned int*)&nn );
	printf( "szStrB64d:[%s]\n", szStrB64d );
	printf( "encode size:[%d]\n", nn );

	memset( szTmp0, '\0', sizeof(szTmp0) );
	strncpy( szTmp0, "\r\n", sizeof(szTmp0) -1 );
	memset( szStrB64d, '\0', sizeof(szStrB64d) );
	base64enc( (unsigned char*)szTmp0, strlen(szTmp0), szStrB64d, sizeof(szStrB64d), (unsigned int*)&nn );
	printf( "szStrB64d:[%s]\n", szStrB64d );
	printf( "encode size:[%d]\n", nn );


	if( ( pFp = fopen( "mailadrlist.txt", "r" ) ) == NULL ){
		exit(1);
	}
	pszTmptmp = (char*)malloc( (sizeof(char)*128) + 1 ); /* 終端分を明示的に+1 */
	if( pszTmptmp == NULL ){
		exit(1);
	}

	memset( pszTmptmp, '\0', (sizeof(char)*128) + 1 );
	
	i = 0;
	j = 0;
	while( ( c=fgetc(pFp) ) != EOF ){
		/* 128byteごと領域確保 (終端分を明示的に+1)*/
		if( (i%128==0) && (i!=0) ){
			printf( "sss %d\n", strlen(pszTmptmp) );
			j ++;
			pszTmptmp = (char*)realloc( pszTmptmp, sizeof(char)*128*(j+1) +1 );
			if( pszTmptmp == NULL ){
				free( pszTmptmp );
				exit(1);
			}
			memset( pszTmptmp+(128*j), '\0', sizeof(char)*(128+1) );
		}
		
		memset( sz1wd, '\0', sizeof(sz1wd) );
		snprintf( sz1wd, sizeof(sz1wd), "%c", c );
		strncat( pszTmptmp, sz1wd, (sizeof(char)*128*j) -1 );
		i ++;
	}
	printf( "%s%d\n", pszTmptmp, strlen(pszTmptmp) );

	memset( szStrB64d2, '\0', sizeof(szStrB64d2) );
	base64enc( (unsigned char*)pszTmptmp, strlen(pszTmptmp), szStrB64d2, sizeof(szStrB64d2), (unsigned int*)&nn );
	printf( "%s %d %d\n", szStrB64d2, strlen(szStrB64d2), nn );
	fclose( pFp );
	free( pszTmptmp );

	unsigned char szNm[1024];
	memset( szNm, 0x00, sizeof(szNm) );
	base64dec( szStrB64d2, strlen(szStrB64d2), szNm, sizeof(szNm), (unsigned int*)&nn );
	printf( "%s %d\n", szNm, nn );



	puts("");
	puts( "SplitFunc" );
	SplitFunc( "aaaabbbbcccdddbbrrrbbbbbbhh", "bbbb", &ppo, &nDiv );
	for( i=0; i<=nDiv; i++ ){
		printf( "[%s]\n", ppo+32*i );
	}
	free( ppo );

	/* fread */
	if( (pFp = fopen( "mailadrlist.txt", "rb" )) == NULL ){
		fprintf( stderr, "Error: Can't open.\n" );
		exit(1);
	}
	nSize = fread( buf, sizeof(unsigned char), 20000, pFp );
	for( i=0; i<nSize; i++ ){
		if( i%16 == 0 ){ printf( "\n" ); }
		printf( "%02x ",buf[i] );
	}
	printf( "\nSize: %d\n", nSize );
	fclose( pFp );


	return 0;
}

/************************************************************************/
/* モジュ－ル名：文字列置換                                             */
/* モジュ－ルID：SedFunc                                                */
/************************************************************************/
/**
 * 検索パタンでマッチした文字列を置換する(sedもどき)
 * 呼び出し側で出力バッファを確保すること
 * (最大100箇所まで置換)
 * (正規表現は使えない)
 *
 * @param[in]  *pszOrgStr        対象文字列ポインタ
 * @param[in]  *pszForConvStr    検索パタン文字列ポインタ
 * @param[in]  *pszAfterConvStr  置換える文字列ポインタ
 * @param[out] *pszRtnStr        返却用文字列格納ポインタ
 * @param[in]  nRtnSize          返却用文字列サイズ
 *
 * @retval SUCCESS  0  正常終了
 * @retval WARNING  1  文字列にパタンが無い
 * @retval ERROR   -1  エラー
 **/
int SedFunc(
	const char *pszOrgStr,
	const char *pszForConvStr,
	const char *pszAfterConvStr,
	char *pszRtnStr,
	size_t nRtnSize
)
{
	int nRtn = 0;
	int i = 0;
	int j = 0;
	char szTmp[strlen(pszOrgStr)+1];
	char szTmp2[strlen(pszOrgStr)+1];
	char *pAdrConvStat;
	char *pAdrConvStatTmp;
	size_t nLenForConvStr = 0;
	char szOrgStrDiv[100][strlen(pszOrgStr)+1]; /* 置換最大100箇所まで */

	memset( pszRtnStr, '\0', nRtnSize );
	memset( szTmp, '\0', sizeof(szTmp) );

	strncpy( szTmp, pszOrgStr, sizeof(szTmp)-1 );
	nLenForConvStr = strlen(pszForConvStr);

	/* 対象文字列を繰り返し検索 */
	for( i=0; ; i++ ){
		if( i == 100 ){
			fprintf( stderr, "Error: The substitution part of 100 was exceeded.\n" );
			nRtn = -1; /* ERROR */ /* 置換最大100箇所を超えた場合エラーとする */
			return nRtn;
		}

		pAdrConvStat = strstr( szTmp, pszForConvStr ); /* 文字列検索(最初に見つかった先頭アドレス返却) */
		if( pAdrConvStat == NULL ){
			if( i == 0 ){
				fprintf( stderr, "Warning: There is no convertible character string.\n" );
				nRtn = 1; /* WARNING */ /* 初回で検索されない場合 */
				return nRtn;
			}
			else {
				break;
			}
		}
		else {
			pAdrConvStatTmp = pAdrConvStat; /* 検索パタン文字列にマッチした(先頭アドレスを保存) */
		}
		

		/* 検索パタンにマッチした文字列より後ろ部分の文字列を分割保存 */
		memset( szTmp2, '\0', sizeof(szTmp2) );
		strncpy( szTmp2, pAdrConvStatTmp + nLenForConvStr, sizeof(szTmp2)-1 );


		/* 検索パタン文字列が存在した位置を文字列終端とし 先頭からの文字列を分割保存 */
		*pAdrConvStatTmp = '\0';
		memset( szOrgStrDiv[i], '\0', sizeof(szOrgStrDiv[i]) );
		strncpy( szOrgStrDiv[i], szTmp, sizeof(szOrgStrDiv[i])-1 );


		/* 次の繰り返しで使用する対象文字列をセット */
		memset( szTmp, '\0', sizeof(szTmp) );
		strncpy( szTmp, szTmp2, sizeof(szTmp)-1 );

	} /* for loop end */

	j = i; /* 繰り返し回数(検索回数)を保存 */


	/* 分割した文字列を結合する */
	for( i=0; i<j; i++ ){

		if( strlen(pszRtnStr) + strlen(szOrgStrDiv[i]) > nRtnSize-1 ){
			fprintf( stderr, "Error: Character string buffer over for return.\n" );
			nRtn = -1; /* ERROR */ /* 返却用文字列バッファオーバー */
			return nRtn;
		}
		else {
			strncat( pszRtnStr, szOrgStrDiv[i], nRtnSize-1 );
		}

		if( strlen(pszRtnStr) + strlen(pszAfterConvStr) > nRtnSize-1 ){
			fprintf( stderr, "Error: Character string buffer over for return.\n" );
			nRtn = -1; /* ERROR */ /* 返却用文字列バッファオーバー */
			return nRtn;
		}
		else {
			strncat( pszRtnStr, pszAfterConvStr, nRtnSize-1 );
		}

	}

	if( strlen(pszRtnStr) + strlen(szTmp) > nRtnSize-1 ){
		fprintf( stderr, "Error: Character string buffer over for return.\n" );
		nRtn = -1; /* ERROR */ /* 返却用文字列バッファオーバー */
		return nRtn;
	}
	else {
		strncat( pszRtnStr, szTmp, nRtnSize-1 );
	}

	return nRtn; /* ここまで来たものは SUCCESS のみ */
}

/************************************************************************/
/* モジュ－ル名：文字列分割                                             */
/* モジュ－ルID：SplitFunc                                              */
/************************************************************************/
/**
 * パタンマッチにより文字列を分割する(perlのsplitもどき)
 * (正規表現は使えない)
 *
 * 本関数内で出力バッファ領域(第3引数)を確保しているので
 * 呼び出し側にて必ず領域開放すること(free)
 *
 * 分割した個々の文字列のサイズは最大32バイトとし
 * 順に返却用文字列に追加格納される
 *
 * @param[in]  *pszOrgStr       対象文字列ポインタ
 * @param[in]  *pszForDivStr    分割パタン文字列ポインタ
 * @param[out] **pszRtnDivStr   返却用文字列ポインタ
 * @param[out] *pnDiv           分割数 (0始まり)
 *
 * @retval SUCCESS  0  正常終了
 * @retval WARNING  1  対象文字列にパタンが無い
 * @retval ERROR   -1  エラー
 **/
int SplitFunc(
	const char *pszOrgStr,
	const char *pszForDivStr,
	char **pszRtnDivStr,
	int *pnDiv
)
{
	int i = 0;
	int k = 0;
	char szTmp[strlen(pszOrgStr)+1];
	char szTmp2[strlen(pszOrgStr)+1];
	char *pAdrConvStat;
	char *pAdrConvStatTmp;
	int nLenForDivStr = 0;


	/* 入力データチェック */
	if( strlen(pszForDivStr) == 0 ){
		fprintf( stderr, "Error: The second argument is NULL.\n" );
		return -1;
	}

	/* 返却用文字列の領域確保 */
	*pszRtnDivStr = (char*)malloc( sizeof(char)*(128+1) );
	if( *pszRtnDivStr == NULL ){
		fprintf( stderr, "Error: Memory allocation Error. (malloc)\n" );
		return -1;
	}

	memset( *pszRtnDivStr, '\0', sizeof(char)*(128+1) );
	memset( szTmp, '\0', sizeof(szTmp) );

	strncpy( szTmp, pszOrgStr, sizeof(szTmp)-1 );
	nLenForDivStr = strlen(pszForDivStr);


	/* 対象文字列を繰り返し検索し分割保存 */
	for(i=0; ; i++){

		/* 返却用文字列領域の再確保( 32*4 byteごとに 128byte拡大) */
		if( (i%4==0) && (i!=0) ){
			k ++;
			*pszRtnDivStr = (char*)realloc( *pszRtnDivStr, sizeof(char)*(128*(k+1) +1) );
			if( *pszRtnDivStr == NULL ){
				free( *pszRtnDivStr );
				fprintf( stderr, "Error: Memory allocation error. (realloc)\n" );
				return -1;
			}
			memset( *pszRtnDivStr+(128*k), '\0', sizeof(char)*(128+1) );
		}


		pAdrConvStat = strstr( szTmp, pszForDivStr ); /* 文字列検索(最初に見つかった先頭アドレス返却) */
		if( pAdrConvStat == NULL ){
			if( i == 0 ){
				fprintf( stderr, "Warning: There is no convertible character string.\n" );
				return 1; /* WARNING */ /* 初回で検索されない場合 */
			}
			else {
				break;
			}
		}
		else {
			pAdrConvStatTmp = pAdrConvStat; /* 分割パタン文字列にマッチした(先頭アドレスを保存) */
		}


		/* 分割パタン文字列より後ろ部分の文字列を保存 */
		memset( szTmp2, '\0', sizeof(szTmp2) );
		strncpy( szTmp2, pAdrConvStatTmp + nLenForDivStr, sizeof(szTmp2)-1 );
		

		/* 分割パタン文字列が存在した位置を文字列終端とし 先頭からの文字列を保存 */
		*pAdrConvStatTmp = '\0';
		if( strlen(szTmp) > 32 ){
			fprintf( stderr, "Error: The buffer for 32 bytes overflows.\n" );
			return -1;
		}
		strncat( *pszRtnDivStr+32*i, szTmp, sizeof(char)*32 ); /* 32byteおきに保存 */


		/* 次の繰り返しで使用する対象文字列をセット */
		memset( szTmp, '\0', sizeof(szTmp) );
		strncpy( szTmp, szTmp2, sizeof(szTmp)-1 );

	} /* for loop end */


	/* 最後の残り文字列を保存 */
	if( strlen(szTmp) > 32 ){
		fprintf( stderr, "Error: The buffer for 32 bytes overflows.\n" );
		return -1;
	}
	strncat( *pszRtnDivStr+32*i, szTmp, sizeof(char)*32 );

	*pnDiv = i; /* 繰り返し回数(検索回数)を返却 */

	return 0;
}

/************************************************************************/
/* モジュ－ル名：正規表現文字列パタンマッチ   デバッグ中..              */
/* モジュ－ルID：reg_string_match                                       */
/************************************************************************/
/**
 * 正規表現(もどき)で文字列パタンがマッチしているか判別
 *
 * @param[in]  *pszStr      対象文字列
 * @param[in]  nStrlen      対象文字列長
 * @param[in]  *pszRegStr   検索パタン
 *
 * 以下の特殊文字に対応
 *  [*]: 直前の1文字の0回以上の繰り返し(最大100とする)
 *  [+]: 直前の1文字の1回以上の繰り返し(最大100とする)
 *  [^]: 先頭
 *  [$]: 末尾
 *  [.]: 改行コード以外の任意の1文字
 *
 * 注意点
 *  :エスケープシーケンス[\]には対応していない
 *  :[.]の後に[*]や[+]での組合せ使用は不可
 *  :[^]は先頭にあるものだけに反応し
 *   それ以外で出てきたものは通常文字とみなす
 *
 * @retval SUCCESS       0 (マッチした)
 * @retval WARNING       1
 * @retval ERROR        -1
 * @retval マッチしない -2
 **/
int reg_string_match( const char *pszStr, size_t nStrlen, const char *pszRegStr )
{
	int i = 0;
	int j = 0;
	int n = 0;
	int cHead;
	int nFlgHead = 0;
	int nTmp[nStrlen];
	char szRept[100+1];
	char szTmpC[1+1];
	char *pszTmp;
	char *pszStrLocal;
	char *pszStrLocalEnd;
	int nFlg = 0;
	int nRtn = 0;
	char szStrCpy[nStrlen];

	/* ローカルのバッファにコピー */
	memset( szStrCpy, '\0', sizeof(szStrCpy) );
	strncpy( szStrCpy, pszStr, sizeof(szStrCpy)-1 );
	pszStrLocal = szStrCpy;
	pszStrLocalEnd = pszStrLocal + strlen(szStrCpy);


	/* 検索パタン文字列を一文字づつチェック */
	for( i=0; i<nStrlen; i++ ){
		if( pszRegStr[i]=='\0' ){ break; }

		memset( szRept, '\0', sizeof(szRept) );


		/* 特殊文字そのものであるのでスルー */
		if( nFlg == 1 ){
			nFlg = 0;
			continue;
		}


		/***** 特殊文字[^]について *****/
		/* 1文字目に限定 */
		if( i==0 ){
			if( pszRegStr[i] == '^' ){
				printf("^:[%c]\n", pszRegStr[i+1]); /*DDDDDDDDDDDD*/
				if( pszRegStr[i+1] == pszStrLocal[i] ){ continue; }
				else { return 2; } /* unmatch */
			}
		}


		/***** 特殊文字[*][+]の対象の場合 *****/
		/* 1文字先を確認 */
		if( (pszRegStr[i+1]=='*') || (pszRegStr[i+1]=='+') ){
			nFlg = 1;

			/*** 特殊文字[.] ***/
			if( pszRegStr[i] == '.' ){
				fprintf( stderr, "Error: Combination of [*] and [+] is not made after [.].\n" );
				return -1;
			}
			/*** 通常の文字 ***/
			else {
				memset( szRept, pszRegStr[i], sizeof(szRept)-1 ); /* 100バイト分 対象文字を埋める */
				
				for( j=100; j>0; j-- ){ /* 100回減少繰り返し 繰り返し文字数多い方から
										   一致するものがあるかどうか確認 */
					szRept[j] = '\0';

					if( strlen(pszStrLocal) < j ){ n = strlen(pszStrLocal); }
					else { n = j; }

					nRtn = strncmp( pszStrLocal, szRept, n );
					if( nRtn == 0 ){
						pszStrLocal += strlen( szRept ); /* ポインタを進める */
						break; /* 一致したら抜ける */
					}
					else {
						if( j == 1 ){
							szRept[0] = '\0'; /* この行無くても良い */
							if( pszRegStr[i+1] == '+' ){ return -2; } /* unmatch */
						}
					}
				}

				if( pszRegStr[i+1] == '*' ){ printf("*:[%s]%s\n",szRept,pszStrLocal); }/*DDDDDDDDDDDDDDDDD*/
				else { printf("+:[%s]%s\n",szRept,pszStrLocal); }/*DDDDDDDDDDDDDDDDD*/
				
				continue;
			}
		}
		/***** 特殊文字[$]の対象 *****/
		else if( pszRegStr[i+1] == '$' ){
			nFlg = 1;

			/* アドレス値比較 文字列最後に来ているか */
			if( pszStrLocal+1 == pszStrLocalEnd ){
				continue;
			}
			else {
				return -2; /* unmatch */
			}
		}


		/***** 特殊文字[*][+][$]の対象ではない場合 *****/
		/* 特殊文字[.] */
		if( pszRegStr[i] == '.' ){
			pszStrLocal ++; /* ポインタを進める */
			continue;
		}
		/* 通常の文字 */
		else {
			memset( szTmpC, '\0', sizeof(szTmpC) );
			strncpy( szTmpC, pszStrLocal ,1 );

			/* 1文字づつ一致しているか比較 */
			if( pszRegStr[i] == szTmpC[0] ){
				pszStrLocal ++; /* ポインタを進める */
				printf(" :[%c][%c]%s\n",szTmpC[0],pszRegStr[i],pszStrLocal); /*DDDDDDDDDDDD*/
				continue;
			}
			else { return 2; } /* unmatch */
		}
		
	} /* for loop end */


	puts("match"); /*DDDDDDDDDDDDDDDDDDDDDDD*/


	return 0;
}

/************************************************************************/
/* モジュ－ル名：base64エンコード                                       */
/* モジュ－ルID：base64enc                                              */
/************************************************************************/
/**
 * base64エンコードする(CyrusSASLのソースコードを参考に)
 * 呼び出し側で出力バッファを確保すること
 *
 * 入力データを3bytesづつ読込み それを4bytesの各6bitずつに分割
 * 各6bitの値を変換テーブルに基づき4文字に変換
 * (6bitに満たないbitは0とする)
 * (4文字に満たない分はパディング文字「=」を追加して4文字にする)
 *
 *   [10000000] [10000000] [10000000]
 *    ↓         ↓         ↓
 *   [100000] [001000] [000010] [000000]
 *
 * @param[in]   *pszIn        入力データポインタ
 * @param[in]   nInSize       入力データサイズ(領域のサイズではなく データそのもののサイズ)
 * @param[out]  *pszRtn       エンコード後文字列返却ポインタ
 * @param[in]   nOutBuffSize  出力バッファサイズ
 * @param[out]  *pnRtnSize    エンコード後文字列サイズ
 *
 * @retval  0  正常終了
 * @retval -1  入力データ異常
 * @retval -2  出力バッファ異常(バッファ不足)
 **/
int base64enc(
	const unsigned char *pszIn,
	unsigned int nInSize,
	char *pszRtn,
	unsigned int nOutBuffSize,
	unsigned int *pnRtnSize
)
{
	/* 64bytes変換文字テーブル */
	static char szConvTbl[64] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ" /*  0~25 */
		"abcdefghijklmnopqrstuvwxyz" /* 26~51 */
		"0123456789+/";              /* 52~63 */
	unsigned int nConvAftrSize = 0;
	char *pszRtnStat = NULL;


	/* 入力データチェック */
	if(( !pszIn )||( nInSize == 0 )){
		return -1;
	}

	/* エンコード後の文字列サイズの仮計算 */
	nConvAftrSize = ( nInSize +2 ) / 3 * 4;

	/* 出力バッファチェック */
	if( nOutBuffSize < nConvAftrSize ){
		return -2;
	}

	/* 開始アドレスを保存 */
	pszRtnStat = pszRtn;


	/* 入力データを3byteづつ4byte(各6bit)に分割 それをszConvTbl[]の添字に */
	while( nInSize >= 3 ){
		
		/* 1byte目の上位6bit分 */
		*pszRtn = szConvTbl[ *(pszIn+0) >> 2 ];
		pszRtn ++;
		
		/* 1byte目の下位2bit分 と 2byte目の上位4bit分 */
		*pszRtn = szConvTbl[ (( *(pszIn+0) << 4 ) & 0x30 ) | ( *(pszIn+1) >> 4 ) ];
		pszRtn ++;
		
		/* 2byte目の下位4bit分 と 3byte目の上位2bit分 */
		*pszRtn = szConvTbl[ (( *(pszIn+1) << 2 ) & 0x3C ) | ( *(pszIn+2) >> 6 ) ];
		pszRtn ++;
		
		/* 3byte目の下位6bit分 */
		*pszRtn = szConvTbl[ *(pszIn+2) & 0x3F ];
		pszRtn ++;
		
		/* 入力データポインタを3byte進める */
		pszIn += 3;
		nInSize -= 3;

	}

	if( nInSize == 2 ){ /* 残り2byteの場合 */

		/* 1byte目の上位6bit分 */
		*pszRtn = szConvTbl[ *(pszIn+0) >> 2 ];
		pszRtn ++;
		
		/* 1byte目の下位2bit分 と 2byte目の上位4bit分 */
		*pszRtn = szConvTbl[ (( *(pszIn+0) << 4 ) & 0x30 ) | ( *(pszIn+1) >> 4 ) ];
		pszRtn ++;
		
		/* 2byte目の下位4bit分 */
		*pszRtn = szConvTbl[ ( *(pszIn+1) << 2 ) & 0x3C ];
		pszRtn ++;

		/* パディング文字追加 */
		*pszRtn = '=';
		pszRtn ++;

	} else if( nInSize == 1 ){ /* 残り1byteの場合 */

		/* 上位6bit分 */
		*pszRtn = szConvTbl[ *(pszIn+0) >> 2 ];
		pszRtn ++;
		
		/* 下位2bit分 */
		*pszRtn = szConvTbl[ ( *(pszIn+0) << 4 ) & 0x30 ];
		pszRtn ++;

		/* パディング文字追加 */
		*pszRtn = '=';
		pszRtn ++;

		/* パディング文字追加 */
		*pszRtn = '=';
		pszRtn ++;

	}

	/* エンコード後の文字列サイズを返却 */
	if( pnRtnSize ){
		*pnRtnSize = pszRtn - pszRtnStat;
	}

	return 0;
}

/************************************************************************/
/* モジュ－ル名：base64デコード                                         */
/* モジュ－ルID：base64dec                                              */
/************************************************************************/
/**
 * base64デコードする(CyrusSASLのソースコードを参考に)
 * 呼び出し側で出力バッファを確保すること
 *
 *   [100000] [001000] [000010] [000000]
 *    ↓         ↓         ↓
 *   [10000000] [10000000] [10000000]
 *
 * @param[in]   *pszIn        入力文字列ポインタ(base64エンコード文字列)
 * @param[in]   nInSize       入力文字列サイズ(領域のサイズではなく データそのもののサイズ)
 * @param[out]  *pszRtn       デコード後データ返却ポインタ
 * @param[in]   nOutBuffSize  出力バッファサイズ
 * @param[out]  *pnRtnSize    デコード後データサイズ
 *
 * @retval  0  正常終了
 * @retval -1  入力データ異常
 **/
int base64dec(
	const char *pszIn,
	unsigned int nInSize,
	unsigned char *pszRtn,
	unsigned int nOutBuffSize,
	unsigned int *pnRtnSize
)
{
	/* 変換テーブル */
	/* 添字をASCIIコード値としbase64で使用する文字は
	   エンコード時の変換文字テーブルの何番目かを表す */
	static char szConvTbl[123] = {
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /*   0~  9 */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /*  10~ 19 */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /*  20~ 29 */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /*  30~ 39 */
		-1,-1,-1,62,-1,-1,-1,63,52,53, /*  40~ 49 */
		54,55,56,57,-1,-1,-1,-1,-1,-1, /*  50~ 59 */
		-1, 0,-1,-1,-1, 0, 1, 2, 3, 4, /*  60~ 69 */ /* '='は0 */
		 5, 6, 7, 8, 9,10,11,12,13,14, /*  70~ 79 */
		15,16,17,18,19,20,21,22,23,24, /*  80~ 89 */
		25,-1,-1,-1,-1,-1,-1,26,27,28, /*  90~ 99 */
		29,30,31,32,33,34,35,36,37,38, /* 100~109 */
		39,40,41,42,43,44,45,46,47,48, /* 110~119 */
		49,50,51                       /* 120~122 */
	};
	unsigned char *pszRtnStat = NULL;
	int nConvAftrSize = 0;


	/* 入力データチェック */
	if(( !pszIn )||( nInSize == 0 )){
		return -1;
	}

	/* デコード後の文字列サイズの仮計算 */
	nConvAftrSize = ( nInSize +3 ) / 4 * 3;

	/* 出力バッファチェック */
	if( nOutBuffSize < nConvAftrSize ){
		return -2;
	}

	/* 開始アドレスを保存 */
	pszRtnStat = pszRtn;


	/* 入力文字列4bytesづつ3byte(各8bit)に結合 (入力文字は必ず6bitに収まる値である) */
	while( nInSize >= 4 ){

		/* 1byte目の全6bit分 と 2byte目の上位2bit分 */
		*pszRtn = ( szConvTbl[(int)*(pszIn+0)] << 2 ) | ( szConvTbl[(int)*(pszIn+1)] >> 4 );
		/*printf("%c%c[%c]\n",*(pszIn+0),*(pszIn+1),*pszRtn);*/
		pszRtn ++;

		/* 2byte目の下位4bit分 と 3byte目の上位4bit分 */
		*pszRtn = ( szConvTbl[(int)*(pszIn+1)] << 4 ) | ( szConvTbl[(int)*(pszIn+2)] >> 2 );
		/*printf("%c%c[%c]\n",*(pszIn+1),*(pszIn+2),*pszRtn);*/
		pszRtn ++;

		/* 3byte目の下位2bit分 と 4byte目の全6bit分 */
		*pszRtn = ( szConvTbl[(int)*(pszIn+2)] << 6 ) | szConvTbl[(int)*(pszIn+3)];
		/*printf("%c%c[%c]\n",*(pszIn+2),*(pszIn+3),*pszRtn);*/
		pszRtn ++;

		/* 入力データポインタを4byte進める */
		pszIn += 4;
		nInSize -= 4;

	}

	if( nInSize == 3 ){ /* 残り3byteの場合 */

		/* 1byte目の全6bit分 と 2byte目の上位2bit分 */
		*pszRtn = ( szConvTbl[(int)*(pszIn+0)] << 2 ) | ( szConvTbl[(int)*(pszIn+1)] >> 4 );
		pszRtn ++;

		/* 2byte目の下位4bit分 と 3byte目の上位4bit分 */
		*pszRtn = ( szConvTbl[(int)*(pszIn+1)] << 4 ) | ( szConvTbl[(int)*(pszIn+2)] >> 2 );
		pszRtn ++;

		/* 3byte目の下位2bit分 */
		*pszRtn = szConvTbl[(int)*(pszIn+2)] << 6;
		pszRtn ++;

	} else if( nInSize == 2 ){ /* 残り2byteの場合 */

		/* 1byte目の全6bit分 と 2byte目の上位2bit分 */
		*pszRtn = ( szConvTbl[(int)*(pszIn+0)] << 2 ) | ( szConvTbl[(int)*(pszIn+1)] >> 4 );
		pszRtn ++;

		/* 2byte目の下位4bit分 */
		*pszRtn = szConvTbl[(int)*(pszIn+1)] << 4;
		pszRtn ++;

	} else if( nInSize == 1 ){ /* 残り1byteの場合 */

		/* 1byte目の全6bit分 */
		*pszRtn = szConvTbl[(int)*(pszIn+0)] << 2;
		pszRtn ++;

	}

	/* エンコード後の文字列サイズを返却 (末尾NULL文字分含む) */
	if( pnRtnSize ){
		*pnRtnSize = pszRtn - pszRtnStat;
	}

	return 0;
}
