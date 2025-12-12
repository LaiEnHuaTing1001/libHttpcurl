#ifndef __HTTP_CORE_H__
#define __HTTP_CORE_H__

#include "HTTPGlobal.h"
#include "CLogPrint.h"

#include "curl/curl.h"

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
class CHTTPCore{

public:
	CHTTPCore(void);
	~CHTTPCore(void);

public:
	static void updateExistTransmisSpeeds(CURL* pRequestUrl, CURL* pUploadUrl, CURL* pDownloadUrl);

	//	全局参数配置
public:
	void setHTTPAgent(std::string agent);
	void setRefererUrl(std::string referer);
	void setCookies(std::string cookies);

public:
	void curlOptInit();

public:
	std::string getCookies();

public:
	void setTimeout(int timeout);
protected:
	void setTransmissionSpeeds();

    void setProxy();

public:
	void parseHeaderFromString(std::string strHeaderMix, std::list<std::string>& listHeader);

public:
	void addHeader(std::map<std::string, std::string>& mapHeader);
	void addHeader(std::list<std::string>& listHeader);

public:
	std::string getLastRequestLocalIp();

protected:
	void resetCookiesAfterPerform();

	//	libcurl执行的回调
private:
	virtual void onHTTPResponseContent(const char* pData, int nSize);

private:
	virtual int onHTTPReadContent(char* pData, int size);

private:
	virtual int onHTTPHeaderContent(const char* pData, int nSize);

protected:
	virtual CURL* getUrlResHandle() = 0;

protected:
	int			m_nTimeout;

protected:
	//static CURL*		m_pUrlRes;
	static	CURLcode	m_nGlobalInitCode;

protected:
	static httpMutex	m_gHttpMutex;		/**< 回调锁，所有回调共享 */

public:
	static curl_off_t ms_uMaxUploadSpeeds;
	static curl_off_t ms_uMaxDownloadSpeeds;
    static bool m_bStopCurrentTask;

protected:
	std::string		m_strUserAgent;
	std::string		m_strRefererUrl;
	std::string		m_strCookies;

public:
    static std::string		m_strProxyIp;
    static int		        m_iProxyPort;
    static std::string		m_strProxyUserName;
    static std::string		m_strProxyPassword;

protected:
	std::string		m_strLastRequestLocalIp;

protected:
	curl_slist*		m_listHeader;

	//	全局回调例程
protected:
	static size_t HTTPWriteCallback(void* data, size_t size, size_t nmemb, void* userp);

protected:
	static size_t HTTPReadCallback(void* data, size_t size, size_t nmemb, void* userp);

protected:
	static size_t HTTPHeaderCallback(char *buffer,   size_t size,   size_t nitems,   void *userp);

};

#endif	//__HTTP_CORE_H__