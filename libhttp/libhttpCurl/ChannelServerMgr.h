#pragma once
#ifndef __C_CHANNEL_SERVER_MGR_H__
#define __C_CHANNEL_SERVER_MGR_H__
#include <boost/serialization/singleton.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <string>

#include "CLogPrint.h"
#include "HTTPGlobal.h"

#define HTTP_REQUEST_SERVER_TIME	"%s://%s:%d/getServerTime?uuid=%s&tenant_id=%s"
#define HTTP_REQUEST_AUTH	"%s://%s:%d/getauthorize?secret=%s&sign=%s&ts=%s&tenant_id=%s"
#define AUTH_CHANNEL_NAME   "admin"
#define AUTH_CHANNEL_PWD    "E657A5ADD722D7413F67D127E607E33A"

class CChannelServerMgr : public boost::serialization::singleton<CChannelServerMgr>
{
protected:
	CChannelServerMgr();

public:
	void setDefaultTimeout(int timeout);
	void setChannelHTTPHostInfo(std::string strIpv4, int nPortv4, std::string strIpv6, int nPortv6);
	void setIsUseHTTPS(bool isUsedHttps = true);
    void setChannelUUIDInfo(std::string strUUID, std::string strTenantId);
    void getChannelUUIDInfo(std::string& strUUID, std::string& strTenantId);
    std::string CheckAndSetUrl(const char* pszUrl);
    bool CheckReConnectServer();

public:
	bool isChannelUsedHTTPS();

public:
	void reDetectServerHost();

public:
	std::string getChannelIp();
	int getChannelPort();

public:
	std::string getAuthLocalIp();
	std::string getAuthirizeToken();
    std::string getUrlHead();

public:
	int getDefaultTimeout();

public:
	int httpRequestAuthirize();

private:
	int detectIpPort(const char* szIp, int nPort, int timeOut);

private:
	httpMutex	m_rwAuthirizeMutex;
	bool		m_isUsedHTTPS;

private:
	std::string m_strHostIpv4;
	int			m_nHostPortv4;
	std::string m_strHostIpv6;
	int			m_nHostPortv6;

private:
	std::string m_strChannelIp;
	int			m_nChannelPort;

private:
	std::string m_strAuthLocalIp;
	std::string m_strAuthirize;

    std::string m_strUUID;
    std::string m_strTenantId;

private:
	int			m_nDefaultTimeout;
    int         m_iLastTime;
	bool        m_bIsFirstGetIp;
};

#endif