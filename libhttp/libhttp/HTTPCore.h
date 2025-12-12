#ifndef __HTTP_CORE_H__
#define __HTTP_CORE_H__

#include "HTTPGlobal.h"
#include "INetCore.h"

#define HTTP_PROTOCOL_HEADER	"http://"
#define HTTPS_PROTOCOL_HEADER	"https://"


/**
*	@class			CHTTPCore
*	@brief 			HTTP处理基类
*	@remark	
*		此类的功能：1、定义静态变量；2：分发回调；3：设置一些全局参数
*	@date    2020/12/01 18:02
*
**/
class CHTTPCore : public INetCore{

public:
	CHTTPCore(void);
	~CHTTPCore(void);

protected:
	CURL* m_pUrlRes;

protected:
	static rwmutex	m_gHttpMutex;		/**< 回调锁，所有回调共享 */

	//	libcurl执行的回调
private:
	virtual void onHTTPResponseContent(const char* pData, int nSize);

private:
	virtual int onHTTPReadContent(char* pData, int size);

private:
	virtual int onHTTPDownloadProgress(INT64 iNow, INT64 iTotal);

private:
	virtual int onHTTPUploadProgress(INT64 iNow, INT64 iTotal);

private:
	virtual int onHTTPHeaderContent(const char* pData, int nSize);

	//	全局参数配置
public:
	void setHTTPAgent(std::string agent);
	void setRefererUrl(std::string referer);
	void setCookies(std::string cookies);

public:
	std::string getCookies();

protected:
	void addGlobalHeaderOpt();

protected:
	void resetCookiesAfterPerform();

protected:
	std::string		m_strUserAgent;
	std::string		m_strRefererUrl;
	std::string		m_strCookies;

public:
	void setTimeout(int timeout);

public:
	virtual INT64 getRequestSize(std::string url);

	//	全局回调例程
protected:
	static size_t HTTPWriteCallback(void* data, size_t size, size_t nmemb, void* userp);

protected:
	static size_t HTTPReadCallback(void* data, size_t size, size_t nmemb, void* userp);

protected:
	static size_t HTTPProgressCallback(void *clientp,   curl_off_t dltotal,   curl_off_t dlnow,   curl_off_t ultotal,   curl_off_t ulnow);

protected:
	static size_t HTTPHeaderCallback(char *buffer,   size_t size,   size_t nitems,   void *userp);

};

#endif	//__HTTP_CORE_H__