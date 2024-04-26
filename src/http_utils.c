#include "http_utils.h"

http_request_type get_type(char *str)
{
	if (startWith(str, "GET"))
		return GET;
	else if (startWith(str, "HEAD"))
		return HEAD;
	else if (startWith(str, "POST"))
		return POST;
	else if (startWith(str, "OPTIONS"))
		return OPTIONS;
	else if (startWith(str, "CONNECT"))
		return CONNECT;
	else if (startWith(str, "TRACE"))
		return TRACE;
	else if (startWith(str, "PUT"))
		return PUT;
	else if (startWith(str, "PATCH"))
		return PATCH;
	else if (startWith(str, "DELETE"))
		return DELETE;
	return UNKNOWN;
}

char *get_request_name(http_request_type type)
{
	switch (type)
	{
		case GET:
			return "GET";
		case HEAD:
			return "HEAD";
		case POST:
			return "POST";
		case OPTIONS:
			return "OPTIONS";
		case CONNECT:
			return "CONNECT";
		case TRACE:
			return "TRACE";
		case PUT:
			return "PUT";
		case PATCH:
			return "PATCH";
		case DELETE:
			return "DELETE";
		default:
			return NULL;
	}
}

char *get_http_code_name(int code)
{
	switch (code)
	{
		case 100:
			return "Continue";
		case 101:
			return "Switching Protocols";
		case 103:
			return "Checkpoint";
		case 200:
			return "OK";
		case 201:
			return "Created";
		case 202:
			return "Accepted";
		case 203:
			return "Non-Authoritative Information";
		case 204:
			return "No Content";
		case 205:
			return "Reset Content";
		case 206:
			return "Partial Content";
		case 300:
			return "Multiple Choices";
		case 301:
			return "Moved Permanently";
		case 303:
			return "See Other";
		case 304:
			return "Not Modified";
		case 306:
			return "Switch Proxy";
		case 307:
			return "Temporary Redirect";
		case 308:
			return "Resume Incomplete";
		case 400:
			return "Bad Request";
		case 401:
			return "Unauthorized";
		case 402:
			return "Payment Required";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 405:
			return "Method Not Allowed";
		case 406:
			return "Not Acceptable";
		case 407:
			return "Proxy Authentication Required";
		case 408:
			return "Request Timeout";
		case 409:
			return "Conflict";
		case 410:
			return "Gone";
		case 411:
			return "Length Required";
		case 412:
			return "Precondition Failed";
		case 413:
			return "Request Entity Too Large";
		case 414:
			return "Request-URI Too Long";
		case 415:
			return "Unsupported Media Type";
		case 416:
			return "Requested Range Not Satisfiable";
		case 417:
			return "Expectation Failed";
		case 418:
			return "I'm a teapot";
		case 500:
			return "Internal Server Error";
		case 501:
			return "Not Implemented";
		case 502:
			return "Bad Gateway";
		case 503:
			return "Service Unavailable";
		case 504:
			return "Gateway Timeout";
		case 505:
			return "HTTP Version Not Supported";
		case 511:
			return "Network Authentication Required";
		default:
			return "UNKNOWN";
	}
}

char *get_browser(char *agent)
{
	char *browser = "Unknown";
	if (strstr(agent, "Netscape"))
		browser = "Netscape";
	if (strstr(agent, "Firefox"))
		browser = "Mozilla Firefox";
	if (strstr(agent, "MSIE") && !strstr(agent, "Opera"))
		browser = "Internet Explorer";
	if (strstr(agent, "Safari"))
		browser = "Apple Safari";
	if (strstr(agent, "Chrome"))
		browser = "Google Chrome";
	if (strstr(agent, "OPR") || strstr(agent, "Opera"))
		browser = "Opera";
	return browser;
}
