// libhttpCurl.cpp : 定义 DLL 应用程序的导出函数。
//

#include "libhttp.h"
#include "curl/curl.h"
#include "util.hpp"

#include "HTTPCore.h"
#include "HTTPRequest.h"
#include "HTTPDownload.h"
#include "HTTPUpload.h"

#include "HTTPGlobal.h"
#include "CLogPrint.h"
#include "ChannelServerMgr.h"
#include "CHTTPResponse.h"
#include <EdrUuid.hpp>
#include <configMgr.hpp>

#pragma comment(lib, "libcurl.lib")

httpMutex g_RequestMutex;
httpMutex g_RequestDownloadMutex;
httpMutex g_RequestUploadMutex;

LIBHTTP_API int LIBHTTPSetChannelInfo(const char* szIpv4, int nPortv4, const char* szIpv6, int nPortv6, bool isUsedHttps, int timeout)
{
	LOGD("[%s]update set channel info; ipv4:%s, portv4:%d, ipv6:%s, portv6:%d, https:%d, timeout:%d", __FUNCTION__, (szIpv4 != NULL) ? szIpv4 : "", nPortv4, (szIpv6 != NULL) ? szIpv6:"", nPortv6, isUsedHttps, timeout);
	std::string strIpv4 = std::string();
	std::string strIpv6 = std::string();

	if (szIpv4)
	{
		strIpv4 = std::string(szIpv4);
	}

	if (szIpv6)
	{
		strIpv6 = std::string(szIpv6);
	}
	CChannelServerMgr::get_mutable_instance().setDefaultTimeout(timeout);
	CChannelServerMgr::get_mutable_instance().setIsUseHTTPS(isUsedHttps);
	CChannelServerMgr::get_mutable_instance().setChannelHTTPHostInfo(strIpv4, nPortv4, strIpv6, nPortv6);
	return 0;
}

LIBHTTP_API void LIBHTTPSetUUIDInfo(const char* pszUUID, const char* pszTenantId) {
    if (pszUUID == NULL) {
        return;
    }

    if (pszTenantId == NULL) {
        return;
    }

    CChannelServerMgr::get_mutable_instance().setChannelUUIDInfo(pszUUID, pszTenantId);
}

LIBHTTP_API bool LIBHTTPChannelUseHTTPS()
{
	return CChannelServerMgr::get_mutable_instance().isChannelUsedHTTPS();
}

LIBHTTP_API void LIBHTTPGetChannelIpInfo(char* szChnanelIp, int& nChannelPort)
{
    httpWriteLock wl(g_RequestMutex);
	std::string channelIp = CChannelServerMgr::get_mutable_instance().getChannelIp();
	int channelPort = CChannelServerMgr::get_mutable_instance().getChannelPort();

	if (channelIp.empty() || channelPort == 0)
	{
		CChannelServerMgr::get_mutable_instance().reDetectServerHost();
		CChannelServerMgr::get_mutable_instance().httpRequestAuthirize();

		channelIp = CChannelServerMgr::get_mutable_instance().getChannelIp();
		channelPort = CChannelServerMgr::get_mutable_instance().getChannelPort();
	}

	strcpy_s(szChnanelIp, 64, channelIp.c_str());
	nChannelPort = channelPort;

	return;
}

LIBHTTP_API	void LIBHTTPSetLogListener(ILogListener* pLogListener)
{
	CLogPrint::get_mutable_instance().setLogCallback(pLogListener);
}

LIBHTTP_API int LIBHTTPGetAuthLocalIp(char* szLocalIp)
{
	std::string strLocalIp = CChannelServerMgr::get_mutable_instance().getAuthLocalIp();
	strcpy_s(szLocalIp, 64, strLocalIp.c_str());
	return 0;
}

LIBHTTP_API int LIBHTTPRequestLocalIp(const char* szUrl, char* szLocalIp)
{
    httpWriteLock wl(g_RequestMutex);
	if ((szUrl == NULL) || (strlen(szUrl) == 0))
	{
		LOGW("[%s]szUrl is empty", __FUNCTION__);
		return ERROR_HTTP_HOST;
	}

	LOGD("[%s]request url:%s", __FUNCTION__, szUrl);

	CHTTPRequest clHttpRequest;
	clHttpRequest.setTimeout(CChannelServerMgr::get_mutable_instance().getDefaultTimeout());
	int resCode = clHttpRequest.httpRequest(szUrl, std::string(), HTTP_METHOD_GET);
	if (resCode < ERROR_SUCCESS)
	{
		LOGW("[%s]request failed, ec:%d", __FUNCTION__, resCode);
		return resCode;
	}

	std::string localIp = clHttpRequest.getLastRequestLocalIp();

	strcpy_s(szLocalIp, 64, localIp.c_str());
	return ERROR_SUCCESS;
}

LIBHTTP_API void LIBHTTPReDetectChannelIp()
{
    httpWriteLock wl(g_RequestMutex);
	CChannelServerMgr::get_mutable_instance().reDetectServerHost();
	CChannelServerMgr::get_mutable_instance().httpRequestAuthirize();
}

LIBHTTP_API void LIBHTTPSetTranssionMissSpeeds(int downloadSpeedMax, int uploadSpeedMax, bool applyExistTrans)
{
	CHTTPCore::ms_uMaxDownloadSpeeds = downloadSpeedMax;
	CHTTPCore::ms_uMaxUploadSpeeds = uploadSpeedMax;

	if (applyExistTrans)
	{
		CHTTPCore::updateExistTransmisSpeeds(CHTTPRequest::m_pUrlRequestHandle, CHTTPUpload::m_pUrlUploadHandle, CHTTPDownload::m_pUrlDownloadHandle);
	}
}

LIBHTTP_API void LIBHTTPSetTranssionMissSpeeds2(unsigned int downloadSpeedMax,
                                                unsigned int uploadSpeedMax,
                                                unsigned int& oldDownloadSpeedMax,
                                                unsigned int& oldUploadSpeedMax) {
    oldDownloadSpeedMax = CHTTPCore::ms_uMaxDownloadSpeeds;
    oldUploadSpeedMax = CHTTPCore::ms_uMaxUploadSpeeds;
    CHTTPCore::ms_uMaxDownloadSpeeds = downloadSpeedMax;
    CHTTPCore::ms_uMaxUploadSpeeds = uploadSpeedMax;
}

LIBHTTP_API void LIBHTTPGetTranssionMissSpeeds(unsigned int& oldDownloadSpeedMax,
                                               unsigned int& oldUploadSpeedMax) {
    oldDownloadSpeedMax = CHTTPCore::ms_uMaxDownloadSpeeds;
    oldUploadSpeedMax = CHTTPCore::ms_uMaxUploadSpeeds;
}

LIBHTTP_API IHTTPResponse* LIBHTTPRequestWithoutToken(const char* szUrl, const char* szSendData, int method, int timeout, const char* szHeader)
{
    httpWriteLock wl(g_RequestMutex);
	if ((szUrl == NULL) || (strlen(szUrl) == 0))
	{
		LOGW("[%s]request url is empty", __FUNCTION__);
		return NULL;
	}

	LOGD("[%s]request url:%s", __FUNCTION__, szUrl);
	std::string strPostData;
	if (szSendData)
	{
		strPostData = std::string(szSendData);
	}

	std::list<std::string> listHeader = std::list<std::string>();
	CHTTPRequest clHttpRequest;
	if (szHeader)
	{
		std::string strHeaderMix = std::string(szHeader);
		clHttpRequest.parseHeaderFromString(strHeaderMix, listHeader);
	}

	clHttpRequest.setTimeout(timeout);
	clHttpRequest.addHeader(listHeader);
	int resCode = clHttpRequest.httpRequest(std::string(szUrl), strPostData, method);
	if (resCode < 0)
	{
		LOGW("[%s]request response code:%d", __FUNCTION__, resCode);
		return CHTTPResponse::createResponse(resCode);
	}

	std::string strResponse = clHttpRequest.getResponse();
	std::string strLocalIp = clHttpRequest.getLastRequestLocalIp();
	LOGD("[%s]request response:%s, localip:%s", __FUNCTION__, strResponse.c_str(), strLocalIp.c_str());
	return CHTTPResponse::createResponse(resCode, strResponse, strLocalIp);
}

LIBHTTP_API IHTTPResponse* LIBHTTPRequest(const char* szUrl, const char* szSendData, bool isNeedAuth, int method, int timeout, const char* szHeader)
{
    httpWriteLock wl(g_RequestMutex);
	if ((szUrl == NULL) || (strlen(szUrl) == 0))
	{
		LOGW("[%s]request url is empty", __FUNCTION__);
		return NULL;
	}

	LOGD("[%s]request url:%s", __FUNCTION__, szUrl);
	std::string strPostData;
	if (szSendData)
	{
		strPostData = std::string(szSendData);
	}

	std::list<std::string> listHeader = std::list<std::string>();
	CHTTPRequest clHttpRequest;
    if (CChannelServerMgr::get_mutable_instance().CheckReConnectServer()) {
        clHttpRequest.Close();
    }

	if (szHeader)
	{
		std::string strHeaderMix = std::string(szHeader);
		clHttpRequest.parseHeaderFromString(strHeaderMix, listHeader);
	}
	if (isNeedAuth)
	{
		std::string tokenAuth = CChannelServerMgr::get_mutable_instance().getAuthirizeToken();
		if (tokenAuth.empty())
		{
			CChannelServerMgr::get_mutable_instance().reDetectServerHost();
			CChannelServerMgr::get_mutable_instance().httpRequestAuthirize();

			tokenAuth = CChannelServerMgr::get_mutable_instance().getAuthirizeToken();
			if (tokenAuth.empty())
			{
				LOGW("[%s]http auth request failed, return -1", __FUNCTION__);
				return CHTTPResponse::createResponse(ERROR_HTTP_HOST);
			}
		}
		listHeader.push_back(std::string(AUTHORIZE_HEADER) + std::string(":") + tokenAuth);
	}

	clHttpRequest.setTimeout(timeout);
	clHttpRequest.addHeader(listHeader);

    std::string strUrl = CChannelServerMgr::get_mutable_instance().CheckAndSetUrl(szUrl);

	int resCode = clHttpRequest.httpRequest(strUrl, strPostData, method);
	if (resCode < 0)
	{
		LOGW("[%s]request response code:%d", __FUNCTION__, resCode);
		return CHTTPResponse::createResponse(resCode);
	}

	if (resCode == HTTP_AUTHORIZE_FAIL_CODE)
	{
		CChannelServerMgr::get_mutable_instance().reDetectServerHost();
		CChannelServerMgr::get_mutable_instance().httpRequestAuthirize();
	}

	std::string strResponse = clHttpRequest.getResponse();
	std::string strLocalIp = clHttpRequest.getLastRequestLocalIp();
	LOGD("[%s]request response:%s, localip:%s", __FUNCTION__, strResponse.c_str(), strLocalIp.c_str());
	return CHTTPResponse::createResponse(resCode, strResponse, strLocalIp);

}

LIBHTTP_API IHTTPResponse* LIBHTTPDownload(const char* szUrl,
                                           const char* szSendData,
                                           const char* szLocalFile,
                                           bool isNeedAuth /* = true */,
                                           int method /* = HTTP_METHOD_GET */,
                                           int timeout /* = 0 */,
                                           const char* szHeader /* = NULL */) {
    return LIBHTTPDownload2(szUrl, 
                            szSendData, 
                            szLocalFile, 
                            isNeedAuth,
                            method, 
                            timeout, 
                            szHeader,
                            NULL,
                            NULL);
}

LIBHTTP_API IHTTPResponse* LIBHTTPDownload2(const char* szUrl, 
                                           const char* szSendData, 
                                           const char* szLocalFile,
                                           bool isNeedAuth /* = true */, 
                                           int method /* = HTTP_METHOD_GET */, 
                                           int timeout /* = 0 */, 
                                           const char* szHeader /* = NULL */,
                                           void* pProgressCallback /* = NULL */,
                                           void* lpUser /* = NULL */)
{
    httpWriteLock wl(g_RequestDownloadMutex);
	if ((szUrl == NULL) || (strlen(szUrl) == 0))
	{
		LOGW("[%s]request url is empty", __FUNCTION__);
		return NULL;
	}

	if (szLocalFile == NULL || strlen(szLocalFile) == 0)
	{
		LOGW("[%s]download local file is empty", __FUNCTION__);
		return NULL;
	}

    if (CHTTPCore::m_bStopCurrentTask) {
        LOGW("[%s] stop flag is true", __FUNCTION__);
        return NULL;
    }

	LOGD("[%s]request url:%s, post:%s", __FUNCTION__, szUrl, szSendData);
	std::string strPostData;
	if (szSendData)
	{
		strPostData = std::string(szSendData);
	}

	std::list<std::string> listHeader = std::list<std::string>();

	CHTTPDownload clHttpDownload;
	if (szHeader)
	{
		std::string strHeaderMix = std::string(szHeader);
		clHttpDownload.parseHeaderFromString(strHeaderMix, listHeader);
	}

	if (isNeedAuth)
	{
		std::string tokenAuth = CChannelServerMgr::get_mutable_instance().getAuthirizeToken();
		if (tokenAuth.empty())
		{
            httpWriteLock wl(g_RequestMutex);
			CChannelServerMgr::get_mutable_instance().reDetectServerHost();
			CChannelServerMgr::get_mutable_instance().httpRequestAuthirize();

			tokenAuth = CChannelServerMgr::get_mutable_instance().getAuthirizeToken();
			if (tokenAuth.empty())
			{
				LOGW("[%s]http auth request failed, return -1", __FUNCTION__);
				return CHTTPResponse::createResponse(ERROR_HTTP_HOST);
			}
		}
		listHeader.push_back(std::string(AUTHORIZE_HEADER) + std::string(":") + tokenAuth);
	}
	clHttpDownload.addHeader(listHeader);
	clHttpDownload.setTimeout(timeout);

    std::string strUrl = CChannelServerMgr::get_mutable_instance().CheckAndSetUrl(szUrl);

	int resCode = clHttpDownload.httpDownload(strUrl, strPostData, szLocalFile, (progressFn)pProgressCallback, lpUser);
	if (resCode < 0)
	{
		LOGW("[%s]download failed, response code:%d", __FUNCTION__, resCode);
		return CHTTPResponse::createResponse(resCode);
	}

	if (resCode == HTTP_AUTHORIZE_FAIL_CODE)
	{
        httpWriteLock wl(g_RequestMutex);
		CChannelServerMgr::get_mutable_instance().reDetectServerHost();
		CChannelServerMgr::get_mutable_instance().httpRequestAuthirize();
	}

	std::string strLocalIp = clHttpDownload.getLastRequestLocalIp();
	LOGD("[%s]request localip:%s", __FUNCTION__, strLocalIp.c_str());
	return CHTTPResponse::createResponse(resCode, std::string(), strLocalIp);
}

LIBHTTP_API IHTTPResponse* LIBHTTPUpload(const char* szUrl, const char* szLocalFile, bool isNeedAuth /* = true */, int method /* = HTTP_METHOD_POST */, int timeout /* = 0 */, const char* szCustomHeader /* = NULL */)
{
    httpWriteLock wl(g_RequestUploadMutex);
	if ((szUrl == NULL) || (strlen(szUrl) == 0))
	{
		LOGW("[%s]request url is empty", __FUNCTION__);
		return NULL;
	}

	if (szLocalFile == NULL || strlen(szLocalFile) == 0)
	{
		LOGW("[%s]upload local file is empty", __FUNCTION__);
		return NULL;
	}

	LOGD("[%s]request url:%s", __FUNCTION__, szUrl);

	std::list<std::string> listHeader = std::list<std::string>();

	CHTTPUpload clHttpUpload;
	if (szCustomHeader)
	{
		std::string strHeaderMix = std::string(szCustomHeader);
		clHttpUpload.parseHeaderFromString(strHeaderMix, listHeader);
	}

	if (isNeedAuth)
	{
		std::string tokenAuth = CChannelServerMgr::get_mutable_instance().getAuthirizeToken();
		if (tokenAuth.empty())
		{
            httpWriteLock wl(g_RequestMutex);
			CChannelServerMgr::get_mutable_instance().reDetectServerHost();
			CChannelServerMgr::get_mutable_instance().httpRequestAuthirize();

			tokenAuth = CChannelServerMgr::get_mutable_instance().getAuthirizeToken();
			if (tokenAuth.empty())
			{
				LOGW("[%s]http auth request failed, return -1", __FUNCTION__);
				return CHTTPResponse::createResponse(ERROR_HTTP_HOST);
			}
		}
		listHeader.push_back(std::string(AUTHORIZE_HEADER) + std::string(":") + tokenAuth);
	}

	clHttpUpload.addHeader(listHeader);
	clHttpUpload.setTimeout(timeout);

    std::string strUrl = CChannelServerMgr::get_mutable_instance().CheckAndSetUrl(szUrl);

	int resCode = clHttpUpload.httpFilePost(strUrl, szLocalFile, NULL, NULL);
	if (resCode < 0)
	{
		LOGW("[%s]download failed, response code:%d", __FUNCTION__, resCode);
		return CHTTPResponse::createResponse(resCode);
	}

	if (resCode == HTTP_AUTHORIZE_FAIL_CODE)
	{
        httpWriteLock wl(g_RequestMutex);
		CChannelServerMgr::get_mutable_instance().reDetectServerHost();
		CChannelServerMgr::get_mutable_instance().httpRequestAuthirize();
	}

	std::string strResponse = clHttpUpload.getResponse();
	std::string strLocalIp = clHttpUpload.getLastRequestLocalIp();

	LOGD("[%s]request response:%s localip:%s", __FUNCTION__, strResponse.c_str(), strLocalIp.c_str());
	return CHTTPResponse::createResponse(resCode, strResponse, strLocalIp);
}

LIBHTTP_API void LIBHTTPStopCurrentDownloadTask() {
    CHTTPCore::m_bStopCurrentTask = true;
}

LIBHTTP_API void LIBHTTPCheckAndSetUrl(char* pszUrl, int iBuffLen) {
    std::string strUrl = CChannelServerMgr::get_mutable_instance().CheckAndSetUrl(pszUrl);
    if (strUrl.length() < iBuffLen) {
        strcpy_s(pszUrl, iBuffLen, strUrl.c_str());
    }
}

LIBHTTP_API void LIBHTTPSetProxy(const char* pstrProxyIp, 
                                 const int iProxyPort,
                                 const char* pstrProxyUserName,
                                 const char* pstrProxyPassword) {
    CHTTPCore::m_strProxyIp = pstrProxyIp;
    CHTTPCore::m_iProxyPort = iProxyPort;
    
    CHTTPCore::m_strProxyUserName = pstrProxyUserName;
    CHTTPCore::m_strProxyPassword = pstrProxyPassword;
}

LIBHTTP_API void LIBHTTPGetAuthirizeToken(char* pszToken, int iTokenLen) {
    std::string strToken = CChannelServerMgr::get_mutable_instance().getAuthirizeToken();
    if (strToken.length() < iTokenLen) {
        strcpy_s(pszToken, iTokenLen, strToken.c_str());
    }
}