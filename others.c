#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>


int dec2bin( double, char* );
int NoRepeatRandom( int, int* );
int atoi_hex( const char* pszHex );


int main(void)
{
	int i = 0;
	int aaa[3] = { 3 ,5, 6 };
	int nTmp[20];
	int *p;
	char szTmp[1024];
	time_t l_time;
	struct tm *pt_localtm;
	char *wday[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
	struct timeval t_tp;
	struct timezone t_zp;
	char szBuf[10];
	int nRtn = 0;
	struct stat stat_buf;
	char szTim[16];

	p = aaa;
	printf("%d\n",*p);
	printf("%d\n",*(p+1));
	printf("%d\n\n",*(p+2));




	memset(szTmp,'\0',sizeof(szTmp));
	nRtn = dec2bin(1, szTmp); printf("%d %20s\n", nRtn, szTmp);
	nRtn = dec2bin(2, szTmp); printf("%d %20s\n", nRtn, szTmp);
	nRtn = dec2bin(3, szTmp); printf("%d %20s\n", nRtn, szTmp);
	nRtn = dec2bin(4, szTmp); printf("%d %20s\n", nRtn, szTmp);
	nRtn = dec2bin(5, szTmp); printf("%d %20s\n", nRtn, szTmp);
	nRtn = dec2bin(6, szTmp); printf("%d %20s\n", nRtn, szTmp);
	nRtn = dec2bin(7, szTmp); printf("%d %20s\n", nRtn, szTmp);
	nRtn = dec2bin(1023.5, szTmp); printf("%d %20s\n", nRtn, szTmp);
	nRtn = dec2bin(234.435, szTmp); printf("%d %20s\n", nRtn, szTmp);
	nRtn = dec2bin(4.52, szTmp); printf("%d %20s\n", nRtn, szTmp);
	nRtn = dec2bin(-4.52, szTmp); printf("%d %20s\n", nRtn, szTmp);
	/*
	  int i;
	  for(i=0;i<4096;i++){
	  nTmp = dec2bin(i, &szTmp[0]);
	  printf("%d %20s", nTmp, szTmp);
	  printf("\n");
	  }
	  printf("\n");
	*/




	time(&l_time);
	pt_localtm = localtime(&l_time);
	printf(
		"Date: %4d/%02d/%02d %s %02d:%02d:%02d\n",
		pt_localtm->tm_year+1900,
		pt_localtm->tm_mon+1,
		pt_localtm->tm_mday,
		wday[pt_localtm->tm_wday],
		pt_localtm->tm_hour,
		pt_localtm->tm_min,
		pt_localtm->tm_sec
		);

	gettimeofday(&t_tp, &t_zp);
	pt_localtm = localtime(&t_tp.tv_sec);
	sprintf( szBuf, "%06d", (int)t_tp.tv_usec );
	printf(
		"Date: %4d/%02d/%02d %s %02d:%02d:%02d.%.3s\n",
		pt_localtm->tm_year+1900,
		pt_localtm->tm_mon+1,
		pt_localtm->tm_mday,
		wday[pt_localtm->tm_wday],
		pt_localtm->tm_hour,
		pt_localtm->tm_min,
		pt_localtm->tm_sec,
		szBuf
		);
	



	printf("\n");
	nRtn = NoRepeatRandom( sizeof(nTmp)/sizeof(nTmp[0]), nTmp );
	if(nRtn==0){
		for(i=0; i<20; i++){ printf("%d ", nTmp[i]); }
		printf("\n");
	}
	putchar('\n');


	/* ファイル情報を表示 */
	nRtn = stat("others.c", &stat_buf);
	if(nRtn==0){
		stat_buf.st_mode = stat_buf.st_mode & (S_IRWXU|S_IRWXG|S_IRWXO);
		
		printf( "デバイスID: %d\n", (int)stat_buf.st_dev );
		printf( "inode番号: %d\n", (int)stat_buf.st_ino );
		printf( "アクセス保護: %o\n", stat_buf.st_mode );
		printf( "ハードリンクの数: %d\n", stat_buf.st_nlink );
		printf( "所有者のユーザID: %d\n", stat_buf.st_uid );
		printf( "所有者のグループID: %d\n", stat_buf.st_gid );
		printf( "デバイスID（特殊ファイルの場合）: %d\n", (int)stat_buf.st_rdev );
		printf( "容量（バイト単位）: %d\n", (int)stat_buf.st_size );
		printf( "ファイルシステムのブロックサイズ: %d\n", (int)stat_buf.st_blksize );
		printf( "割り当てられたブロック数: %d\n", (int)stat_buf.st_blocks );
		printf( "最終アクセス時刻: %s", ctime(&stat_buf.st_atime) );
		printf( "最終修正時刻: %s", ctime(&stat_buf.st_mtime) );
		printf( "最終状態変更時刻: %s", ctime(&stat_buf.st_ctime) );
	}

	/* strftime */
	memset(szTim, '\0', sizeof(szTim));
	pt_localtm = localtime(&stat_buf.st_mtime);
	strftime( szTim, sizeof(szTim)-1, "%Y%m%d%H%M%S", pt_localtm);
	puts(szTim);


	/* 端末画面サイズ */
	struct winsize strWS;
	ioctl( 1, TIOCGWINSZ, &strWS ); /* 0:stdin  1:stdout  2:stderr */
	printf( "strWS.ws_col=[%d]\n", strWS.ws_col );
	printf( "strWS.ws_row=[%d]\n", strWS.ws_row );


	/* open / read (system call) */
	int nFd = 0;
	char szBuff[16];
	ssize_t nSize = 0;
	ssize_t nDiff = 1;
	char *pt;

	pt = szBuff;
	memset( szBuff, '\0', sizeof(szBuff) );

	nFd = open( "mailadrlist.txt", O_RDONLY );
	if(nFd < 0){
		exit(1);
	}
	printf( "nFd: %d\n", nFd );

	while( nDiff < sizeof(szBuff) ){
		nSize = read( nFd, pt, sizeof(szBuff)-nDiff );
		printf( "read_res: %d\n", nSize );
		
		if( nSize < 0 ){
			exit(1);
		}
		else if( nSize == 0 ){
			break;
		}
		else {
			pt += nSize;
			nDiff += nSize;
		}
	}

	if( nSize != sizeof(szBuff)-1 ){
		puts("error");
		close( nFd );
		exit(1);
	}
	
	close( nFd );
	
	puts( szBuff );



	printf( "atoi_hex=[%d,0x%x]\n", atoi_hex( "3ff" ), atoi_hex( "3ff" ) );

	return 0;
}

/************************************************************************/
/* モジュ－ル名：10進数->2進数(文字列)変換処理                          */
/* モジュ－ルID：dec2bin                                                */
/************************************************************************/
/**
 * 10進実数を2進のビット文字列に変換
 * 整数及び小数、負の数対応
 * 呼び出し側で出力バッファを確保すること
 *
 * @param[in]  nDec      10進数
 * @param[out] *pszBin   2進数変換後文字列返却ポインタ
 *
 * @retval SUCCESS  0  正常終了
 * @retval WARNING  1  少数変換時第10位までで打ち切り
 **/
int dec2bin( double nDec, char *pszBin )
{
	int i = 0;
	int j = 0;
	int nChngFlg = 0;
	int nRtn = 0; /* SUCCESS */
	int nCnt = 0;
	int nInt = 0;
	int nSho = 0;
	int nAmari = 0;
	char szRtnInt[1024];
	char szRtnIntTmp[1024];
	int nSekiInt = 0;
	double nSmallNum = 0.0;
	double nSeki = 0.0;
	double nSekiSmallNum = 0.0;
	char szRtnSmallNum[1024];
	char szRtnSmallNumTmp[1024];
	char szRtn[1024];

	memset( szRtnInt, '\0', sizeof(szRtnInt) );
	memset( szRtnIntTmp, '\0', sizeof(szRtnIntTmp) );
	memset( szRtnSmallNum, '\0', sizeof(szRtnSmallNum) );
	memset( szRtnSmallNumTmp, '\0', sizeof(szRtnSmallNumTmp) );
	memset( szRtn, '\0', sizeof(szRtn) );

	/* 絶対値を取る */
	if( nDec < 0.0 ){
		nInt = nDec * -1;                 /* 小数点以下切捨て */
		nSmallNum = ( nDec * -1 ) - nInt; /* 小数点以下格納 */
	}
	else {
		nInt = nDec;             /* 小数点以下切捨て */
    nSmallNum = nDec - nInt; /* 小数点以下格納 */
	}

	/* 整数部分処理 */
	nSho = nInt;
	while(1){
		nAmari = nSho % 2;
		nSho = nSho / 2;

		snprintf( szRtnInt, sizeof(szRtnInt), "%d%s", nAmari, szRtnIntTmp );
		
		if( nSho == 0 ){ break; }

		strcpy( szRtnIntTmp, szRtnInt );
	}

	/* 小数部分処理 */
	if( nSmallNum > 0.0 ){
		strcpy( szRtnSmallNumTmp, "." );
		
		while(1){
			nSeki = nSmallNum * 2.0;
			nSekiInt = nSeki;                 /* 小数点以下切捨て */
			nSekiSmallNum = nSeki - nSekiInt; /* 小数点以下格納 */
	  
			snprintf( szRtnSmallNum, sizeof(szRtnSmallNum), "%s%d", szRtnSmallNumTmp, nSekiInt );
			
			if( nCnt == 9 ){ nRtn = 1;  break; } /* WARNING */ /* 小数点第10位で打ち切り(循環小数か) */
			if( nSekiSmallNum == 0.0 ){ break; }
	  
			strcpy( szRtnSmallNumTmp, szRtnSmallNum );
			nSmallNum = nSekiSmallNum;
			nCnt ++;
		}
	}

	snprintf( szRtn, sizeof(szRtn), "%s%s", szRtnInt, szRtnSmallNum );
	
	/* 負の数の場合 2の補数処理を行う */
	if( nDec < 0.0 ){
		
		/* 0,1の反転 */
		for( i=0; szRtn[i]!='\0'; i++ ){
			if     ( szRtn[i] == '0' ){ szRtn[i] = '1'; }
			else if( szRtn[i] == '1' ){ szRtn[i] = '0'; }
		}
		
		/* 反転したものに対し1を加算 */
		for( j=i-1; j>=0; j-- ){  /* szRtn[i]は'\0'の為[i-1]から開始 */
			if( j == i-1 ){
				if     ( szRtn[j] == '0' ){ szRtn[j] = '1'; nChngFlg = 0; }
				else if( szRtn[j] == '1' ){ szRtn[j] = '0'; nChngFlg = 1; }
			}
			else {
				if( nChngFlg == 1 ){
					if     ( szRtn[j] == '0' ){ szRtn[j] = '1'; nChngFlg = 0; }
					else if( szRtn[j] == '1' ){ szRtn[j] = '0'; nChngFlg = 1; }
					else                                      { nChngFlg = 1; } /* '.'はそのまま */
				}
			}
		}

	}

	strcpy( pszBin, szRtn );
	
	return nRtn;
}

/************************************************************************/
/* モジュ－ル名：重複のない乱数を生成                                   */
/* モジュ－ルID：NoRepeatRandom                                         */
/************************************************************************/
/**
 * 乱数を生成する
 * 第1引数で指定した数を重複の無い乱数の数として
 * 呼び出し側で確保した配列(第2引数)に格納する
 * (※「第1引数値」=「第2引数の配列添字数」とする)
 *
 * @param[in]  nDat     乱数の数
 * @param[out] *nArray  乱数結果格納用配列
 *
 * @retval SUCCESS  0  正常終了
 * @retval ERROR   -1  エラー
 **/
int NoRepeatRandom( int nDat, int *nArray )
{
	int i = 0;
	int n = 0;
	int nRand = 0;
	struct timeval t_tp;
	struct timezone t_zp;
	
	if( nDat <= 0 ){
		fprintf( stderr, "Error: Please make the argument[0] into a positive number.\n" );
		return -1;
	}

	/* 乱数種を時間(1秒毎)に設定 */
	/* srand( (unsigned)time(NULL) ); */

	/* 乱数種をマイクロ秒に設定 */
	gettimeofday( &t_tp, &t_zp );
	srand( t_tp.tv_usec );
	
	for( i=0; i<nDat; i++ ){ nArray[i] = i; }
	
	for( i=0; i<nDat; i++ ){
		nRand = rand() % nDat; /* 乱数をnDat未満の数にする*/
		
		/* swap */
		n = nArray[i];
		nArray[i] = nArray[nRand];
		nArray[nRand] = n;
	}

	return 0;
}

/************************************************************************/
/* モジュ－ル名：16進表記文字列を数値(int)に変換                        */
/* モジュ－ルID：atoi_hex                                               */
/************************************************************************/
/**
 * 16進表記文字列を数値(int)に変換する
 * strtol()で同じことが出来る。。
 *
 * @param[in]  *pszHex  入力文字列(16進表記)
 *
 * @retval  正常  nHex  変換結果値(int)を返す
 * @retval  異常  -1    入力文字列異常(NULL)
 * @retval  異常  -2    入力文字列に16進表記以外の文字が含まれる
 **/
int atoi_hex( const char* pszHex )
{
	int i = 0;
	int nHex = 0;
	int nLen = 0;


	/* 入力文字列チェック */
	if( !pszHex ){
		return -1;
	}

	/* 入力文字列長を確認 strlen()と同じ */
	while( *(pszHex+i) ){
		i ++;
	}
	nLen = i;

	i = 0;

	/* 入力文字列(16進表記)を下位桁から順に1文字づつ確認
	   桁が上がれば変換結果値を4bitづつ左にシフトして足す */
	while( nLen ){
		nLen -= 1;

		if(( *(pszHex+nLen) >= '0' )&&( *(pszHex+nLen) <= '9' )){
			/* 数字の場合 */
			nHex += (*(pszHex+nLen)-'0') << (4*i);

		} else if(( *(pszHex+nLen) >= 'A' )&&( *(pszHex+nLen) <= 'F' )){
			/* A~F(大文字)の場合 */
			nHex += (*(pszHex+nLen)-'A'+10) << (4*i);

		} else if(( *(pszHex+nLen) >= 'a' )&&( *(pszHex+nLen) <= 'f' )){
			/* a~f(小文字)の場合 */
			nHex += (*(pszHex+nLen)-'a'+10) << (4*i);

		} else {
			/* それ以外の文字はエラー */
			return -2;
		}

		i ++;
	}

	return nHex;
}
