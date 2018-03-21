#ifndef __HTTP_DEFINES
#define __HTTP_DEFINES


#define HTTP_VER "HTTP/1.1"

enum {
	RESPONSE_STATUS_100 = 0,
	RESPONSE_STATUS_101,

	RESPONSE_STATUS_200,
	RESPONSE_STATUS_201,
	RESPONSE_STATUS_202,
	RESPONSE_STATUS_203,
	RESPONSE_STATUS_204,
	RESPONSE_STATUS_205,
	RESPONSE_STATUS_206,

	RESPONSE_STATUS_300,
	RESPONSE_STATUS_301,
	RESPONSE_STATUS_302,
	RESPONSE_STATUS_303,
	RESPONSE_STATUS_304,
	RESPONSE_STATUS_305,
	RESPONSE_STATUS_306,
	RESPONSE_STATUS_307,

	RESPONSE_STATUS_400,
	RESPONSE_STATUS_401,
	RESPONSE_STATUS_402,
	RESPONSE_STATUS_403,
	RESPONSE_STATUS_404,
	RESPONSE_STATUS_405,
	RESPONSE_STATUS_406,
	RESPONSE_STATUS_407,
	RESPONSE_STATUS_408,
	RESPONSE_STATUS_409,
	RESPONSE_STATUS_410,
	RESPONSE_STATUS_411,
	RESPONSE_STATUS_412,
	RESPONSE_STATUS_413,
	RESPONSE_STATUS_414,
	RESPONSE_STATUS_415,
	RESPONSE_STATUS_416,
	RESPONSE_STATUS_417,

	RESPONSE_STATUS_500,
	RESPONSE_STATUS_501,
	RESPONSE_STATUS_502,
	RESPONSE_STATUS_503,
	RESPONSE_STATUS_504,
	RESPONSE_STATUS_505,	

	RESPONSE_STATUS_MAX
};

static const char *g_pszResponseStatus[RESPONSE_STATUS_MAX][2] = {
	{ "100 Continue",							"100.html" },
	{ "101 Switching Protocols Upgrade",		"101.html" },

	{ "200 OK",									"200.html" },
	{ "201 Created Location",					"201.html" },
	{ "202 Accepted",							"202.html" },
	{ "203 Non-Authoritative Information",		"203.html" },
	{ "204 No Content",							"204.html" },
	{ "205 Reset Content",						"205.html" },
	{ "206 Partial Content",					"206.html" },

	{ "300 Multiple Choices",					"300.html" },
	{ "301 Moved Permanently Location",			"301.html" },
	{ "302 Found Location",						"302.html" },
	{ "303 See Other Location",					"303.html" },
	{ "304 Not Modified",						"304.html" },
	{ "305 Use Proxy Location",					"305.html" },
	{ "306 (undefine)",							"306.html" },
	{ "307 Temporary Redirect",					"307.html" },

	{ "400 Bad Request",						"400.html" },
	{ "401 Unauthorized",						"401.html" },
	{ "402 Payment Required",					"402.html" },
	{ "403 Forbidden",							"403.html" },
	{ "404 Not Found",							"404.html" },
	{ "405 Method Not Allowed",					"405.html" },
	{ "406 Not Acceptable",						"406.html" },
	{ "407 Proxy Authentication Required",		"407.html" },
	{ "408 Request Timeout",					"408.html" },
	{ "409 Conflict",							"409.html" },
	{ "410 Gone",								"410.html" },
	{ "411 Length Required Content-Length",		"411.html" },
	{ "412 Precondition Failed",				"412.html" },
	{ "413 Request Entity Too Large",			"413.html" },
	{ "414 Request-URI Too Long",				"414.html" },
	{ "415 Unsupported Media Type",				"415.html" },
	{ "416 Requested Range Not Satisfiable",	"416.html" },
	{ "417 Expectation Failed Expect",			"417.html" },

	{ "500 Internal Server Error",				"500.html" },
	{ "501 Not Implemented",					"501.html" },
	{ "502 Bad Gateway",						"502.html" },
	{ "503 Service Unavailable",				"503.html" },
	{ "504 Gateway Timeout",					"504.html" },
	{ "505 HTTP Version Not Supported",			"505.html" }
};


#define CONTENT_TYPE			"Content-Type: %s"

enum {
	CONTENT_TYPE_TEXT_HTML = 0,
	CONTENT_TYPE_TEXT_PLAIN,
	CONTENT_TYPE_TEXT_XML,
	CONTENT_TYPE_TEXT_JAVASCRIPT,
	CONTENT_TYPE_IMAGE_GIF,
	CONTENT_TYPE_IMAGE_JPG,
	CONTENT_TYPE_IMAGE_PNG,

	CONTENT_TYPE_CGI,
	CONTENT_TYPE_IMAGE_ICO,
	CONTENT_TYPE_MAX
};

static const char *g_pszContentType[CONTENT_TYPE_MAX] = {
	"text/html",
	"text/palin",
	"text/xml",
	"application/javascript",
	"image/gif",
	"image/jpg",
	"image/png",

	"undefine",
	"image/vnd.microsoft.icon"
};

#define CHARSET				"; charset=%s"
#define CHARSET_ISO2022JP	0
#define CHARSET_SHIFTJIS	1
#define CHARSET_EUCJP		2
#define CHARSET_UTF8		3
#define CHARSET_MAX			4

static const char *g_pszCharset[CHARSET_MAX] = {
	"ISO-2022-JP",
	"Shift-JIS",
	"EUC-JP",
	"UTF-8"
};


#define CONNECTION				"Connection: %s"
#define CONNECTION_CLOSE		0
#define CONNECTION_KEEPALIVE	1
#define CONNECTION_MAX			2

static const char *g_pszConnection[CONNECTION_MAX] = {
	"Close",
	"Keep-Alive"
};


#define KEEP_ALIVE			"Keep-Alive: timeout=%d max=%d"
#define KEEP_ALIVE_TIMEOUT	2
#define KEEP_ALIVE_MAX		50

#define CONTENT_LENGTH "Content-Length: %d"




#endif
