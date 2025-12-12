#include "ChannelServerMgr.h"
#include "HTTPRequest.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/reader.h>

#include <WinSock2.h>
#include <MSWSock.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <Ws2tcpip.h>
#include <EdrUuid.hpp>
#include <GenSecret.hpp>
#include <configMgr.hpp>

CChannelServerMgr::CChannelServerMgr()
{
	m_isUsedHTTPS = true;
	m_strHostIpv4.clear();
	m_nHostPortv4 = 0;

	m_strHostIpv6.clear();
	m_nHostPortv6 = 0;

	m_strChannelIp.clear();
	m_nChannelPort = 0;

    m_iLastTime = 0;

	m_bIsFirstGetIp = false;
}

void CChannelServerMgr::setDefaultTimeout(int timeout)
{
	m_nDefaultTimeout = timeout;
}

void CChannelServerMgr::setChannelUUIDInfo(std::string strUUID, std::string strTenantId) {
    if (!strUUID.empty()) {
        m_strUUID = strUUID;
    }
    
    if (!strTenantId.empty()) {
        m_strTenantId = strTenantId;
    }
}

void CChannelServerMgr::getChannelUUIDInfo(std::string& strUUID, std::string& strTenantId) {
    if (m_strUUID.empty()) {
        m_strUUID = configMgr::GetUUID();
        if (m_strUUID.empty()) {
            int iUUIDType = 0;
            if (EdrUUid::IsSysUuid()) {
                iUUIDType = 1;
            }
            m_strUUID = EdrUUid::GetUUid(iUUIDType);
        }
    }

    if (m_strTenantId.empty()) {
        m_strTenantId = configMgr::GetTenantId();
        if (m_strTenantId.empty()) {
            m_strTenantId = EdrUUid::GetTenantId();
        }
    }

    strUUID = m_strUUID;
    strTenantId = m_strTenantId;
}

void CChannelServerMgr::setChannelHTTPHostInfo(std::string strIpv4, int nPortv4, std::string strIpv6, int nPortv6)
{
	m_strHostIpv4 = strIpv4;
	m_nHostPortv4 = nPortv4;

	m_strHostIpv6 = strIpv6;
	m_nHostPortv6 = nPortv6;

	reDetectServerHost();
	//httpRequestAuthirize();
}

void CChannelServerMgr::setIsUseHTTPS(bool isUsedHttps /* = true */)
{
	m_isUsedHTTPS = isUsedHttps;
}

bool CChannelServerMgr::isChannelUsedHTTPS()
{
	return m_isUsedHTTPS;
}

std::string CChannelServerMgr::getChannelIp()
{
	return m_strChannelIp;
}

int CChannelServerMgr::getChannelPort()
{
	return m_nChannelPort;
}

std::string CChannelServerMgr::getAuthLocalIp()
{
    /*std::string strRet = m_strAuthLocalIp;
    if (strRet.empty()) {
        strRet = "127.0.0.1";
    }
	return strRet;*/

	std::string strRet;
	{
		httpWriteLock wl(m_rwAuthirizeMutex);
		strRet = m_strAuthLocalIp;
	}
	if (strRet.empty() == true)
	{
		if (!m_bIsFirstGetIp) 
		{
			httpRequestAuthirize();
			m_bIsFirstGetIp = true;
		}
	}
	strRet = m_strAuthLocalIp;
	return strRet;
}

std::string CChannelServerMgr::getAuthirizeToken()
{
	return m_strAuthirize;
}

std::string CChannelServerMgr::CheckAndSetUrl(const char* pszUrl) {
    std::string strUrl = pszUrl;

    if (pszUrl == NULL ||
        pszUrl[0] == '\0') {
        return strUrl;
    }

    if (strnicmp(pszUrl, "https://", 8) != 0) {
        if (strnicmp(pszUrl, "http://", 7) != 0) {
            std::string strUrlHead = getUrlHead();
            if (pszUrl[0] == '/') {
                strUrl = strUrlHead + pszUrl;
            } else {
                strUrl = strUrlHead + "/" + pszUrl;
            }
        }
    }

    std::string strUUID;
    std::string strTenantId;
    getChannelUUIDInfo(strUUID, strTenantId);

    if (strstr(pszUrl, "?uuid=") == NULL &&
        strstr(pszUrl, "&uuid=") == NULL) {
        if (strchr(pszUrl, '?') == NULL) {
            strUrl += "?uuid=";
            strUrl += strUUID;
        } else {
            strUrl += "&uuid=";
            strUrl += strUUID;
        }
    }


    if (strstr(pszUrl, "&tenant_id=") == NULL) {
        strUrl += "&tenant_id=";
        strUrl += strTenantId;
    }

    return strUrl;
}

std::string CChannelServerMgr::getUrlHead() {
    char szUrl[MAX_PATH] = {0};
    std::string httpProtocol;
    if (m_isUsedHTTPS) {
        httpProtocol = std::string("https");
    } else {
        httpProtocol = std::string("http");
    }

    if (m_strChannelIp.empty()) {
        return "";
    }

    if (m_nChannelPort <= 0) {
        return "";
    }

    sprintf_s(szUrl,
              "%s://%s:%d",
              httpProtocol.c_str(),
              m_strChannelIp.c_str(),
              m_nChannelPort);
    return szUrl;
}

int CChannelServerMgr::getDefaultTimeout()
{
	return m_nDefaultTimeout;
}

void CChannelServerMgr::reDetectServerHost()
{
	LOGD("[%s]ip4:%s, p4:%d, ip6:%s, p6:%d", __FUNCTION__, m_strHostIpv4.c_str(), m_nHostPortv4, m_strHostIpv6.c_str(), m_nHostPortv6);
	if ((m_strHostIpv4.empty() || m_nHostPortv4 == 0) && (m_strHostIpv6.empty() || m_nHostPortv6 == 0))
	{
		LOGD("[%s]re detect server host, but invalid v4 or v6, ", __FUNCTION__);
		return;
	}

	std::string proxyIp = CHTTPCore::m_strProxyIp;
	int proxyPort = CHTTPCore::m_iProxyPort;

	if (!proxyIp.empty() && proxyPort != 0)
	{
		if (detectIpPort(proxyIp.c_str(), proxyPort, 5000) == ERROR_SUCCESS)
		{
			if (proxyIp.find(":") != std::string::npos)
			{
				m_strChannelIp = std::string("[") + m_strHostIpv6 + std::string("]");
				m_nChannelPort = m_nHostPortv6;
			}
			else {
				m_strChannelIp = m_strHostIpv4;
				m_nChannelPort = m_nHostPortv4;
			}
			LOGD("[%s]ip proxy %s:%d connect success, used", __FUNCTION__, proxyIp.c_str(), proxyPort);
			return;
		}
	}

	if (m_strHostIpv6.empty() || m_nHostPortv6 == 0)
	{
		m_strChannelIp = m_strHostIpv4;
		m_nChannelPort = m_nHostPortv4;
		LOGD("[%s]ipv6 info is empty, use ip4;", __FUNCTION__);
		return;
	}

	if (detectIpPort(m_strHostIpv4.c_str(), m_nHostPortv4, 5000) == ERROR_SUCCESS)
	{
		m_strChannelIp = m_strHostIpv4;
		m_nChannelPort = m_nHostPortv4;
		LOGD("[%s]ipv4 connect success, used", __FUNCTION__);
		return;
	}

	if (detectIpPort(m_strHostIpv6.c_str(), m_nHostPortv6, 5000) == ERROR_SUCCESS)
	{
		m_strChannelIp = std::string("[") + m_strHostIpv6 + std::string("]");
		m_nChannelPort = m_nHostPortv6;

		LOGD("[%s]ipv6 connect success, used", __FUNCTION__);
		return;
	}

	m_strChannelIp = m_strHostIpv4;
	m_nChannelPort = m_nHostPortv4;
	LOGD("[%s]ipv6 and ipv4 all connect failed, use ipv4;", __FUNCTION__);
	return;
}

bool CChannelServerMgr::CheckReConnectServer() {
    __time32_t llTime = _time32(NULL);
    if (abs(llTime - m_iLastTime) > 60) {
        m_strAuthirize = "";
        m_iLastTime = llTime;
        return true;
    }

    return false;
}

int CChannelServerMgr::httpRequestAuthirize()
{
	httpWriteLock wl(m_rwAuthirizeMutex);

	if (m_strChannelIp.empty() || m_nChannelPort == 0)
	{
		reDetectServerHost();
		if (m_strChannelIp.empty() || m_nChannelPort == 0)
		{
			LOGE("[%s]check connect ip detect false, return timeout", __FUNCTION__);
			return ERROR_CURL_TIMEOUT;
		}
	}

	std::string httpProtocol;
	if (m_isUsedHTTPS)
	{
		httpProtocol = std::string("https");
	}
	else {
		httpProtocol = std::string("http");
	}

    std::string strServerTime = "";
    
    std::string strUUID;
    std::string strTenantId;
    getChannelUUIDInfo(strUUID, strTenantId);

    {
        boost::format strServerTimeUrl(HTTP_REQUEST_SERVER_TIME);
        strServerTimeUrl% httpProtocol% m_strChannelIp% m_nChannelPort% m_strUUID% m_strTenantId;
        std::string strUrl = strServerTimeUrl.str();
        CHTTPRequest httpRequest;
        httpRequest.setTimeout(m_nDefaultTimeout);
        int resCode = httpRequest.httpRequest(strUrl, std::string(), false);
        if (resCode != ERROR_SUCCESS) {
            LOGE("[%s]http auth request failed; ec:%d", __FUNCTION__, resCode);
            return resCode;
        }

        m_strAuthLocalIp = httpRequest.getLastRequestLocalIp();
        std::string strResponse = httpRequest.getResponse();

        rapidjson::Document rootDoc;
        rootDoc.Parse(strResponse.c_str());
        if (rootDoc.HasParseError()) {
            LOGD("[%s]http response parse failed; response:%s", __FUNCTION__, strResponse.c_str());
            return ERROR_HTTP_HOST;
        }

        if (rootDoc.HasMember("data")) {
            const char* pszTime = rootDoc["data"].GetString();
            if (pszTime != NULL) {
                strServerTime = pszTime;
                m_iLastTime = _time32(NULL);
            }

            if (strServerTime.empty()) {
                LOGD("[%s]response server time error:%s", __FUNCTION__, strResponse.c_str());
                return ERROR_HTTP_HOST;
            }
        } else {
            LOGD("[%s]http response server time failed;%s", __FUNCTION__, strResponse.c_str());
            return ERROR_HTTP_HOST;
        }
    }

    std::string strSecret = GenSecret::GetSecret(m_strUUID, "e657a5add722d7413f67d127e607e33a", strServerTime);

	boost::format strAuthUrl(HTTP_REQUEST_AUTH);
	strAuthUrl% httpProtocol% m_strChannelIp% m_nChannelPort% strSecret% m_strUUID% strServerTime% m_strTenantId;
	std::string strUrl = strAuthUrl.str();

	//int ret = LibHTTPRequest(strUrl, std::string(), std::map<std::string, std::string>(), strResponse, m_strAuthLocalIp, HTTP_METHOD_POST, m_nDefaultTimeout);
	CHTTPRequest httpRequest;
	httpRequest.setTimeout(m_nDefaultTimeout);
	int resCode = httpRequest.httpRequest(strUrl, std::string(), false);
	if (resCode != ERROR_SUCCESS)
	{
		LOGE("[%s]http auth request failed; ec:%d", __FUNCTION__, resCode);
		return resCode;
	}

    if (m_strAuthLocalIp.empty()) {
        m_strAuthLocalIp = httpRequest.getLastRequestLocalIp();
    }

	std::string strResponse = httpRequest.getResponse();

	rapidjson::Document rootDoc;
	rootDoc.Parse(strResponse.c_str());
	if (rootDoc.HasParseError())
	{
		LOGD("[%s]http response parse failed; response:%s", __FUNCTION__, strResponse.c_str());
		return ERROR_HTTP_HOST;
	}

	if (rootDoc.HasMember("data"))
	{
		m_strAuthirize = rootDoc["data"].GetString();
		LOGD("[%s]response new auth:%s", __FUNCTION__, m_strAuthirize.c_str());
	}
	else
	{
		LOGD("[%s]response miss data member, response:%s", __FUNCTION__, strResponse.c_str());
	}

	if (m_strAuthirize.empty())
	{
		LOGD("[%s]request finish, but authorize is empty; response:%s", __FUNCTION__, strResponse.c_str());
		return ERROR_HTTP_HOST;
	}
	return ERROR_SUCCESS;
}

int CChannelServerMgr::detectIpPort(const char* szIp, int nPort, int timeOut)
{
	CHAR szPort[20] = { 0 };
	sprintf_s(szPort, 20, "%d", nPort);

	WSADATA	wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		return ERROR_SOCKET_INVALID;
	}

	ADDRINFOA simpleServer = { 0 };
	simpleServer.ai_family = AF_UNSPEC;
	simpleServer.ai_socktype = SOCK_STREAM;
	simpleServer.ai_flags = AI_PASSIVE;

	ADDRINFOA* lpAddrRes = NULL;
	int nRes = getaddrinfo(szIp, szPort, &simpleServer, &lpAddrRes);
	if (nRes != ERROR_SUCCESS || lpAddrRes == NULL)
	{
		WSACleanup();
		return ERROR_HOST_UNRECOGNIZE;
	}

	SOCKET connectSocket = socket(lpAddrRes->ai_family, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
	{
		freeaddrinfo(lpAddrRes);
		WSACleanup();
		return ERROR_SOCKET_INVALID;
	}

	if (timeOut > 0)
	{
		unsigned long iMode = 1;
		iResult = ioctlsocket(connectSocket, FIONBIO, &iMode);
		if (iResult != NO_ERROR)
		{
		}
	}

	if (connect(connectSocket, (SOCKADDR*)lpAddrRes->ai_addr, lpAddrRes->ai_addrlen) == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		if (errCode != WSAEWOULDBLOCK || timeOut <= 0)
		{
			closesocket(connectSocket);
			freeaddrinfo(lpAddrRes);
			WSACleanup();
			return ERROR_SOCKET_INVALID;
		}

		unsigned long iMode = 0;
		TIMEVAL	timeOutVal;
		timeOutVal.tv_sec = (timeOut / 1000);
		timeOutVal.tv_usec = (timeOut % 1000);
		iResult = ioctlsocket(connectSocket, FIONBIO, &iMode);

		fd_set Write, Err;
		FD_ZERO(&Write);
		FD_ZERO(&Err);
		FD_SET(connectSocket, &Write);
		FD_SET(connectSocket, &Err);
		select(0, NULL, &Write, &Err, &timeOutVal);
		int error_code;
		int error_code_size = sizeof(error_code);
		int iRes = getsockopt(connectSocket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&error_code, &error_code_size);
		if (error_code < 0)
		{
			closesocket(connectSocket);
			freeaddrinfo(lpAddrRes);
			WSACleanup();
			return ERROR_CURL_TIMEOUT;
		}
	}

	closesocket(connectSocket);
	freeaddrinfo(lpAddrRes);
	WSACleanup();
	return ERROR_SUCCESS;
}