#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "TsCommon.h"


static const char *g_pszGenre_lvl1 [] = {
	"ニュース／報道",			// 0x0
	"スポーツ",					// 0x1
	"情報／ワイドショー",		// 0x2
	"ドラマ",					// 0x3
	"音楽",						// 0x4
	"バラエティ",				// 0x5
	"映画",						// 0x6
	"アニメ／特撮",				// 0x7
	"ドキュメンタリー／教養",	// 0x8
	"劇場／公演",				// 0x9
	"趣味／教育",				// 0xa
	"福祉",						// 0xb
	"予備",						// 0xc
	"予備",						// 0xd
	"拡張",						// 0xe
	"その他 ",					// 0xf
};

static const char *g_pszGenre_lvl2 [] = {
	"定時・総合",						// 0x0 0x0
	"天気",								// 0x0 0x1
	"特集・ドキュメント",				// 0x0 0x2
	"政治・国会",						// 0x0 0x3
	"経済・市況",						// 0x0 0x4
	"海外・国際",						// 0x0 0x5
	"解説",								// 0x0 0x6
	"討論・会談",						// 0x0 0x7
	"報道特番",							// 0x0 0x8
	"ローカル・地域",					// 0x0 0x9
	"交通",								// 0x0 0xa
	"",									// 0x0 0xb
	"",									// 0x0 0xc
	"",									// 0x0 0xd
	"",									// 0x0 0xe
	"その他",							// 0x0 0xf

	"スポーツニュース",					// 0x1 0x0
	"野球",								// 0x1 0x1
	"サッカー",							// 0x1 0x2
	"ゴルフ",							// 0x1 0x3
	"その他の球技",						// 0x1 0x4
	"相撲・格闘技",						// 0x1 0x5
	"オリンピック・国際大会",			// 0x1 0x6
	"マラソン・陸上・水泳",				// 0x1 0x7
	"モータースポーツ",					// 0x1 0x8
	"マリン・ウィンタースポーツ",		// 0x1 0x9
	"競馬・公営競技",					// 0x1 0xa
	"",									// 0x1 0xb
	"",									// 0x1 0xc
	"",									// 0x1 0xd
	"",									// 0x1 0xe
	"その他",							// 0x1 0xf

	"芸能・ワイドショー",				// 0x2 0x0
	"ファッション",						// 0x2 0x1
	"暮らし・住まい",					// 0x2 0x2
	"健康・医療",						// 0x2 0x3
	"ショッピング・通販",				// 0x2 0x4
	"グルメ・料理",						// 0x2 0x5
	"イベント",							// 0x2 0x6
	"番組紹介・お知らせ",				// 0x2 0x7
	"",									// 0x2 0x8
	"",									// 0x2 0x9
	"",									// 0x2 0xa
	"",									// 0x2 0xb
	"",									// 0x2 0xc
	"",									// 0x2 0xd
	"",									// 0x2 0xe
	"その他",							// 0x2 0xf

	"国内ドラマ",						// 0x3 0x0
	"海外ドラマ",						// 0x3 0x1
	"時代劇",							// 0x3 0x2
	"",									// 0x3 0x3
	"",									// 0x3 0x4
	"",									// 0x3 0x5
	"",									// 0x3 0x6
	"",									// 0x3 0x7
	"",									// 0x3 0x8
	"",									// 0x3 0x9
	"",									// 0x3 0xa
	"",									// 0x3 0xb
	"",									// 0x3 0xc
	"",									// 0x3 0xd
	"",									// 0x3 0xe
	"その他",							// 0x3 0xf

	"国内ロック・ポップス",				// 0x4 0x0
	"海外ロック・ポップス",				// 0x4 0x1
	"クラシック・オペラ",				// 0x4 0x2
	"ジャズ・フュージョン",				// 0x4 0x3
	"歌謡曲・演歌",						// 0x4 0x4
	"ライブ・コンサート",				// 0x4 0x5
	"ランキング・リクエスト",			// 0x4 0x6
	"カラオケ・のど自慢",				// 0x4 0x7
	"民謡・邦楽",						// 0x4 0x8
	"童謡・キッズ",						// 0x4 0x9
	"民族音楽・ワールドミュージック",	// 0x4 0xa
	"",									// 0x4 0xb
	"",									// 0x4 0xc
	"",									// 0x4 0xd
	"",									// 0x4 0xe
	"その他",							// 0x4 0xf

	"クイズ",							// 0x5 0x0
	"ゲーム",							// 0x5 0x1
	"トークバラエティ",					// 0x5 0x2
	"お笑い・コメディ",					// 0x5 0x3
	"音楽バラエティ",					// 0x5 0x4
	"旅バラエティ",						// 0x5 0x5
	"料理バラエティ",					// 0x5 0x6
	"",									// 0x5 0x7
	"",									// 0x5 0x8
	"",									// 0x5 0x9
	"",									// 0x5 0xA
	"",									// 0x5 0xb
	"",									// 0x5 0xc
	"",									// 0x5 0xd
	"",									// 0x5 0xe
	"その他",							// 0x5 0xf

	"洋画",								// 0x6 0x0
	"邦画",								// 0x6 0x1
	"アニメ",							// 0x6 0x2
	"",									// 0x6 0x3
	"",									// 0x6 0x4
	"",									// 0x6 0x5
	"",									// 0x6 0x6
	"",									// 0x6 0x7
	"",									// 0x6 0x8
	"",									// 0x6 0x9
	"",									// 0x6 0xa
	"",									// 0x6 0xb
	"",									// 0x6 0xc
	"",									// 0x6 0xd
	"",									// 0x6 0xe
	"その他",							// 0x6 0xf

	"国内アニメ",						// 0x7 0x0
	"海外アニメ",						// 0x7 0x1
	"特撮",								// 0x7 0x2
	"",									// 0x7 0x3
	"",									// 0x7 0x4
	"",									// 0x7 0x5
	"",									// 0x7 0x6
	"",									// 0x7 0x7
	"",									// 0x7 0x8
	"",									// 0x7 0x9
	"",									// 0x7 0xa
	"",									// 0x7 0xb
	"",									// 0x7 0xc
	"",									// 0x7 0xd
	"",									// 0x7 0xe
	"その他",							// 0x7 0xf

	"社会・時事",						// 0x8 0x0
	"歴史・紀行",						// 0x8 0x1
	"自然・動物・環境",					// 0x8 0x2
	"宇宙・科学・医学",					// 0x8 0x3
	"カルチャー・伝統文化",				// 0x8 0x4
	"文学・文芸",						// 0x8 0x5
	"スポーツ",							// 0x8 0x6
	"ドキュメンタリー全般",				// 0x8 0x7
	"インタビュー・討論",				// 0x8 0x8
	"",									// 0x8 0x9
	"",									// 0x8 0xa
	"",									// 0x8 0xb
	"",									// 0x8 0xc
	"",									// 0x8 0xd
	"",									// 0x8 0xe
	"その他",							// 0x8 0xf

	"現代劇・新劇",						// 0x9 0x0
	"ミュージカル",						// 0x9 0x1
	"ダンス・バレエ",					// 0x9 0x2
	"落語・演芸",						// 0x9 0x3
	"歌舞伎・古典",						// 0x9 0x4
	"",									// 0x9 0x5
	"",									// 0x9 0x6
	"",									// 0x9 0x7
	"",									// 0x9 0x8
	"",									// 0x9 0x9
	"",									// 0x9 0xa
	"",									// 0x9 0xb
	"",									// 0x9 0xc
	"",									// 0x9 0xd
	"",									// 0x9 0xe
	"その他",							// 0x9 0xf

	"旅・釣り・アウトドア",				// 0xa 0x0
	"園芸・ペット・手芸",				// 0xa 0x1
	"音楽・美術・工芸",					// 0xa 0x2
	"囲碁・将棋",						// 0xa 0x3
	"麻雀・パチンコ",					// 0xa 0x4
	"車・オートバイ",					// 0xa 0x5
	"コンピュータ・ＴＶゲーム",			// 0xa 0x6
	"会話・語学",						// 0xa 0x7
	"幼児・小学生",						// 0xa 0x8
	"中学生・高校生",					// 0xa 0x9
	"大学生・受験",						// 0xa 0xa
	"生涯教育・資格",					// 0xa 0xb
	"教育問題",							// 0xa 0xc
	"",									// 0xa 0xd
	"",									// 0xa 0xe
	"その他",							// 0xa 0xf

	"高齢者",							// 0xb 0x0
	"障害者",							// 0xb 0x1
	"社会福祉",							// 0xb 0x2
	"ボランティア",						// 0xb 0x3
	"手話",								// 0xb 0x4
	"文字（字幕）",						// 0xb 0x5
	"音声解説",							// 0xb 0x6
	"",									// 0xb 0x7
	"",									// 0xb 0x8
	"",									// 0xb 0x9
	"",									// 0xb 0xa
	"",									// 0xb 0xb
	"",									// 0xb 0xc
	"",									// 0xb 0xd
	"",									// 0xb 0xe
	"その他",							// 0xb 0xf

	"",									// 0xc 0x0
	"",									// 0xc 0x1
	"",									// 0xc 0x2
	"",									// 0xc 0x3
	"",									// 0xc 0x4
	"",									// 0xc 0x5
	"",									// 0xc 0x6
	"",									// 0xc 0x7
	"",									// 0xc 0x8
	"",									// 0xc 0x9
	"",									// 0xc 0xa
	"",									// 0xc 0xb
	"",									// 0xc 0xc
	"",									// 0xc 0xd
	"",									// 0xc 0xe
	"",									// 0xc 0xf

	"",									// 0xd 0x0
	"",									// 0xd 0x1
	"",									// 0xd 0x2
	"",									// 0xd 0x3
	"",									// 0xd 0x4
	"",									// 0xd 0x5
	"",									// 0xd 0x6
	"",									// 0xd 0x7
	"",									// 0xd 0x8
	"",									// 0xd 0x9
	"",									// 0xd 0xa
	"",									// 0xd 0xb
	"",									// 0xd 0xc
	"",									// 0xd 0xd
	"",									// 0xd 0xe
	"",									// 0xd 0xf

	"BS/地上デジタル放送用番組付属情報",// 0xe 0x0
	"広帯域 CSデジタル放送用拡張",		// 0xe 0x1
	"衛星デジタル音声放送用拡張",		// 0xe 0x2
	"サーバー型番組付属情報",			// 0xe 0x3
	"IP放送用番組付属情報",				// 0xe 0x4
	"",									// 0xe 0x5
	"",									// 0xe 0x6
	"",									// 0xe 0x7
	"",									// 0xe 0x8
	"",									// 0xe 0x9
	"",									// 0xe 0xa
	"",									// 0xe 0xb
	"",									// 0xe 0xc
	"",									// 0xe 0xd
	"",									// 0xe 0xe
	"",									// 0xe 0xf

	"",									// 0xf 0x0
	"",									// 0xf 0x1
	"",									// 0xf 0x2
	"",									// 0xf 0x3
	"",									// 0xf 0x4
	"",									// 0xf 0x5
	"",									// 0xf 0x6
	"",									// 0xf 0x7
	"",									// 0xf 0x8
	"",									// 0xf 0x9
	"",									// 0xf 0xa
	"",									// 0xf 0xb
	"",									// 0xf 0xc
	"",									// 0xF 0xd
	"",									// 0xF 0xe
	"その他",							// 0xf 0xf
};

static const char *g_pszVideoComponentType [] = {
	"480i",		// 0x0
	"予約",		// 0x1
	"予約",		// 0x2
	"予約",		// 0x3
	"予約",		// 0x4
	"予約",		// 0x5
	"予約",		// 0x6
	"予約",		// 0x7
	"予約",		// 0x8
	"2160p",	// 0x9
	"480p",		// 0xa
	"1080i",	// 0xb
	"720p",		// 0xc
	"240p",		// 0xd
	"1080p",	// 0xe
};

static const char *g_pszVideoRatio [] = {
	"予約",							// 0x0
	"4:3",							// 0x1
	"16:9 (with panorama vector)",	// 0x2
	"16:9",							// 0x3
	"16:9 wider",					// 0x4
};

static const char *g_pszAudioComponentType [] = {
	"予約",										// 0x0
	"シングルモノ",								// 0x1
	"デュアルモノ",								// 0x2
	"ステレオ",									// 0x3
	"2/1モード",								// 0x4
	"3/0モード",								// 0x5
	"2/2モード",								// 0x6
	"3/1モード",								// 0x7
	"3/2モード (5.1ch)",						// 0x8
	"3/2+LFE (5.1ch + low frequency effect)",	// 0x9
};

static const char *g_pszAudioSamplingRate [] = {
	"予約",		// 0x0
	"16kHz",	// 0x1
	"22kHz",	// 0x2
	"24kHz",	// 0x3
	"予約",		// 0x4
	"32kHz",	// 0x5
	"44kHz",	// 0x6
	"48kHz",	// 0x7
	"96kHz",	// 0x8
	"192kHz",	// 0x9
};

static const char *g_pszAudioQuality [] = {
	"予約",		// 0x0
	"モード1",	// 0x1
	"モード2",	// 0x2
	"モード3",	// 0x3
};



void CTsCommon::getStrEpoch (time_t tx, const char *format, char *pszout, int outsize)
{
	struct tm *tl;
	struct tm stm;
	tl = localtime_r (&tx, &stm); 
	strftime(pszout, outsize - 1, format, tl);
}

void CTsCommon::getStrSecond (int second, char *pszout, int outsize)
{
	int hh = second / 3600;
	int mm = (second % 3600) / 60;
	int ss = (second % 3600) % 60;
	snprintf (pszout, outsize, "%02d:%02d:%02d", hh, mm, ss);
}

time_t CTsCommon::getEpochFromMJD (const uint8_t *mjd)
{
	if (!mjd) {
		return 0;
	}

	int tnum,yy,mm,dd;
	char buf[10];
	time_t l_time ;
	time_t end_time ;
	struct tm tl ;
	struct tm *endtl ;
	char cendtime[32];
	char cmjd[32];

	tnum = (mjd[0] & 0xFF) << 8 | (mjd[1] & 0xFF);

	yy = (tnum - 15078.2) / 365.25;
	mm = ((tnum - 14956.1) - (int)(yy * 365.25)) / 30.6001;
	dd = (tnum - 14956) - (int)(yy * 365.25) - (int)(mm * 30.6001);

	if (mm == 14 || mm == 15) {
		yy += 1;
		mm = mm - 1 - (1 * 12);
	} else {
		mm = mm - 1;
	}

	tl.tm_year = yy;
	tl.tm_mon = mm - 1;
	tl.tm_mday = dd;
	memset(buf, '\0', sizeof(buf));
	sprintf(buf, "%x", mjd[2]);
	tl.tm_hour = atoi(buf);
	memset(buf, '\0', sizeof(buf));
	sprintf(buf, "%x", mjd[3]);
	tl.tm_min = atoi(buf);
	memset(buf, '\0', sizeof(buf));
	sprintf(buf, "%x", mjd[4]);
	tl.tm_sec = atoi(buf);

	tl.tm_wday = 0;
	tl.tm_isdst = 0;
	tl.tm_yday = 0;

	l_time = mktime(&tl);
	return l_time;
}

int CTsCommon::getSecFromBCD (const uint8_t *bcd)
{
	if (!bcd) {
		return -1;
	}

	int hh,mm,ss;
	char buf[24];

	if ((bcd[0] == 0xFF) && (bcd[1] == 0xFF) && (bcd[2] == 0xFF)) {
		// 終了未定
		hh = mm = ss = 0;
		ss = -1;
	} else {
		memset(buf, '\0', sizeof(buf));
		sprintf(buf, "%x", bcd[0]);
		hh = atoi(buf)*3600;
		memset(buf, '\0', sizeof(buf));
		sprintf(buf, "%x", bcd[1]);
		mm = atoi(buf)*60;
		memset(buf, '\0', sizeof(buf));
		sprintf(buf, "%x", bcd[2]);
		ss = atoi(buf);
	}

	return hh+mm+ss;
}

const char* CTsCommon::getGenre_lvl1 (uint8_t genre)
{
	if (genre < 0x00 || genre > 0x0f) {
		return "ジャンル指定なし";
	} else {
		return g_pszGenre_lvl1 [genre];
	}
}

const char* CTsCommon::getGenre_lvl2 (uint8_t genre)
{
	// 0x00 - 0xff
	const char *str = g_pszGenre_lvl2 [genre];
	if (!str || (int)strlen(str) == 0) {
		return "未定義";
	} else {
		return str;
	}
}

const char* CTsCommon::getVideoComponentType (uint8_t type)
{
	uint8_t t = (type >> 4) & 0x0f;
	if (t < 0x00 || t > 0x0e) {
		return "未定義";
	} else {
		return g_pszVideoComponentType [t];
	}
}

const char* CTsCommon::getVideoRatio (uint8_t type)
{
	uint8_t r = type & 0x0f;
	if (r < 0x00 || r > 0x04) {
		return "未定義";
	} else {
		return g_pszVideoRatio [r];
	}
}

const char* CTsCommon::getAudioComponentType (uint8_t type)
{
	if (type < 0x00 || type > 0x09) {
		return "未定義";
	} else {
		return g_pszAudioComponentType [type];
	}
}

const char* CTsCommon::getAudioSamplingRate (uint8_t samplingRate)
{
	if (samplingRate < 0x00 || samplingRate > 0x09) {
		return "未定義";
	} else {
		return g_pszAudioSamplingRate [samplingRate];
	}
}

const char* CTsCommon::getAudioQuality (uint8_t quality)
{
	if (quality < 0x00 || quality > 0x03) {
		return "未定義";
	} else {
		return g_pszAudioQuality [quality];
	}
}

uint16_t CTsCommon::freq2ch (uint16_t frequency)
{
	return ((frequency/7) - 473) / 6 + 13;
}