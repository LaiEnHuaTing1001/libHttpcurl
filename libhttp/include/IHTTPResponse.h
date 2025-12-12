#pragma once
#ifndef __I_HTTP_RESPONSE_H__
#define __I_HTTP_RESPONSE_H__

enum __tagHTTPMethod
{
	HTTP_METHOD_GET = 0,		/**< 使用GET方式发起请求 */
	HTTP_METHOD_POST,			/**< 使用POST方式发起请求 */
};

class IHTTPResponse
{
public:
	virtual int getResponseCode() = 0;

public:
	virtual const char* getResponse() = 0;

public:
	virtual const char* getLocalIp() = 0;

public:
	virtual void Release() = 0;
};

class ILogListener
{
public:
	virtual void onLog(int level, const char* szLog) = 0;
};

/*
class IHTTPBaseBuilder
{
public:
	virtual IHTTPBaseBuilder* SetHTTPUrl(const char* szUrl) = 0;
	virtual IHTTPBaseBuilder* SetHTTPHeader(const char* szHeaderKey, const char* szHeaderValue) = 0;
	virtual IHTTPBaseBuilder* SetNeedChannelAuth(bool needAuth) = 0;
	virtual IHTTPBaseBuilder* SetMethod(int method = HTTP_METHOD_POST) = 0;
	virtual IHTTPBaseBuilder* SetTimeout(int ms = 5000) = 0;	//	connect timeout
};

class IRequestService
{

};
*/
#endif