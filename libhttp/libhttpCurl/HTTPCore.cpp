#include "HTTPCore.h"

httpMutex CHTTPCore::m_gHttpMutex;


//CURL* CHTTPCore::m_pUrlRes = NULL;

CURLcode CHTTPCore::m_nGlobalInitCode = CURL_LAST;

curl_off_t CHTTPCore::ms_uMaxDownloadSpeeds = 0;
curl_off_t CHTTPCore::ms_uMaxUploadSpeeds = 0;

std::string		CHTTPCore::m_strProxyIp = "";
int		        CHTTPCore::m_iProxyPort = 0;
std::string		CHTTPCore::m_strProxyUserName = "";
std::string		CHTTPCore::m_strProxyPassword = "";

CHTTPCore::CHTTPCore(void) {
	if (m_nGlobalInitCode == CURL_LAST)
	{
		m_nGlobalInitCode = curl_global_init(CURL_GLOBAL_ALL);
	}

	m_nTimeout = HTTP_TIME_OUT_MS;

	m_listHeader = NULL;
}


CHTTPCore::~CHTTPCore(void) {
	if (m_listHeader)
	{
		curl_slist_free_all(m_listHeader);
		m_listHeader = NULL;
	}
}

void CHTTPCore::updateExistTransmisSpeeds(CURL* pRequestUrl, CURL* pUploadUrl, CURL* pDownloadUrl)
{
	if (pRequestUrl)
	{
		curl_easy_setopt(pRequestUrl, CURLOPT_MAX_RECV_SPEED_LARGE, ms_uMaxDownloadSpeeds);
		curl_easy_setopt(pRequestUrl, CURLOPT_MAX_SEND_SPEED_LARGE, ms_uMaxUploadSpeeds);
	}

	if (pUploadUrl)
	{
		curl_easy_setopt(pUploadUrl, CURLOPT_MAX_RECV_SPEED_LARGE, ms_uMaxDownloadSpeeds);
		curl_easy_setopt(pUploadUrl, CURLOPT_MAX_SEND_SPEED_LARGE, ms_uMaxUploadSpeeds);
	}

	if (pDownloadUrl)
	{
		curl_easy_setopt(pDownloadUrl, CURLOPT_MAX_RECV_SPEED_LARGE, ms_uMaxDownloadSpeeds);
		curl_easy_setopt(pDownloadUrl, CURLOPT_MAX_SEND_SPEED_LARGE, ms_uMaxUploadSpeeds);
	}
}

void CHTTPCore::curlOptInit()
{
	if (getUrlResHandle() == NULL)
	{
		return;
	}

	curl_easy_setopt(getUrlResHandle(), CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_TCP_KEEPIDLE, 30L);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_TCP_KEEPINTVL, 20L);

	curl_easy_setopt(getUrlResHandle(), CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_SSL_VERIFYHOST, 0L);

	curl_easy_setopt(getUrlResHandle(), CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_HEADER, 0L);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_NOBODY, 0L);

	curl_easy_setopt(getUrlResHandle(), CURLOPT_WRITEDATA, NULL);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_HEADERDATA, NULL);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_HEADERFUNCTION, NULL);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_XFERINFODATA, NULL);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_XFERINFOFUNCTION, NULL);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_READDATA, NULL);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_READFUNCTION, NULL);
}

void CHTTPCore::onHTTPResponseContent(const char* pData, int nSize) {
	return;
}

int CHTTPCore::onHTTPReadContent(char* pData, int size) {
	return 0;
}

int CHTTPCore::onHTTPHeaderContent(const char* pData, int nSize) {

	return nSize;
}

void CHTTPCore::setHTTPAgent(std::string agent) {

	m_strUserAgent = agent;
}

void CHTTPCore::setRefererUrl(std::string referer) {

	m_strRefererUrl = referer;
}

void CHTTPCore::setCookies(std::string cookies) {

	m_strCookies = cookies;
}

std::string CHTTPCore::getCookies() {

	return m_strCookies;
}

void CHTTPCore::setTimeout(int timeout) {
	m_nTimeout = (timeout == 0)?HTTP_TIME_OUT_MS : timeout;
}

void CHTTPCore::setTransmissionSpeeds()
{
	if ((ms_uMaxUploadSpeeds == 0) && (ms_uMaxDownloadSpeeds == 0))
	{
		curl_easy_setopt(getUrlResHandle(), CURLOPT_BUFFERSIZE, CURL_MAX_WRITE_SIZE);
		curl_easy_setopt(getUrlResHandle(), CURLOPT_MAX_RECV_SPEED_LARGE, 0);
		curl_easy_setopt(getUrlResHandle(), CURLOPT_MAX_SEND_SPEED_LARGE, 0);
	}
	else
	{
		curl_off_t minSpeeds = 0;
		if (ms_uMaxUploadSpeeds == 0)
		{
			minSpeeds = ms_uMaxDownloadSpeeds;
		}
		else if (ms_uMaxDownloadSpeeds == 0)
		{
			minSpeeds = ms_uMaxUploadSpeeds;
		}
		else
		{
			minSpeeds = (ms_uMaxUploadSpeeds < ms_uMaxDownloadSpeeds) ? ms_uMaxUploadSpeeds : ms_uMaxDownloadSpeeds;
		}
		
		if ((minSpeeds < CURL_MAX_WRITE_SIZE) && (minSpeeds != 0))
		{
			curl_easy_setopt(getUrlResHandle(), CURLOPT_BUFFERSIZE, minSpeeds);
		}
		else
		{
			curl_easy_setopt(getUrlResHandle(), CURLOPT_BUFFERSIZE, CURL_MAX_WRITE_SIZE);
		}

		curl_easy_setopt(getUrlResHandle(), CURLOPT_MAX_RECV_SPEED_LARGE, ms_uMaxDownloadSpeeds);
		curl_easy_setopt(getUrlResHandle(), CURLOPT_MAX_SEND_SPEED_LARGE, ms_uMaxUploadSpeeds);
	}
}

void CHTTPCore::setProxy() {
    if (!CHTTPCore::m_strProxyIp.empty() && CHTTPCore::m_iProxyPort != 0) {
        char szProxy[512] = {0};
        if (CHTTPCore::m_strProxyIp.find(':') != CHTTPCore::m_strProxyIp.npos) {
            sprintf_s(szProxy, "socks5h://[%s]:%d", CHTTPCore::m_strProxyIp.c_str(), CHTTPCore::m_iProxyPort);
        }
        else {
            sprintf_s(szProxy, "socks5h://%s:%d", CHTTPCore::m_strProxyIp.c_str(), CHTTPCore::m_iProxyPort);
        }
        
        curl_easy_setopt(getUrlResHandle(), CURLOPT_PROXY, szProxy);

        if (!CHTTPCore::m_strProxyUserName.empty()) {
            curl_easy_setopt(getUrlResHandle(), CURLOPT_PROXYUSERNAME, CHTTPCore::m_strProxyUserName.c_str());
        }

        if (!CHTTPCore::m_strProxyPassword.empty()) {
            curl_easy_setopt(getUrlResHandle(), CURLOPT_PROXYPASSWORD, CHTTPCore::m_strProxyPassword.c_str());
        }
    }
}

void CHTTPCore::parseHeaderFromString(std::string strHeaderMix, std::list<std::string>& listHeader)
{
	if (strHeaderMix.empty())
	{
		return;
	}

	int prePos = 0, findPos = 0;
	do
	{
		prePos = findPos;
		findPos = strHeaderMix.find("\r\n", prePos);
		if (findPos == std::string::npos)
		{
			break;
		}

		std::string singleHeader = strHeaderMix.substr(prePos, findPos - prePos);
		findPos += strlen("\r\n");

		if (!singleHeader.empty())
		{
			listHeader.push_back(singleHeader);
		}
	} while (findPos != std::string::npos);

	if (prePos < strHeaderMix.size())
	{
		std::string singleHeader = strHeaderMix.substr(prePos);
		if (!singleHeader.empty())
		{
			listHeader.push_back(singleHeader);
		}
	}

	return;
}

void CHTTPCore::addHeader(std::map<std::string, std::string>& mapHeader) {
	if (mapHeader.empty())
	{
		return;
	}

	if (m_listHeader)
	{
		curl_slist_free_all(m_listHeader);
		m_listHeader = NULL;
	}

	std::map<std::string, std::string>::iterator itor = mapHeader.begin();
	for (itor; itor != mapHeader.end(); itor++)
	{
		std::string singleHeader = itor->first + std::string(":") + itor->second;
		m_listHeader = curl_slist_append(m_listHeader, singleHeader.c_str());
	}
}

void CHTTPCore::addHeader(std::list<std::string>& listHeader)
{
	if (listHeader.empty())
	{
		return;
	}

	if (m_listHeader)
	{
		curl_slist_free_all(m_listHeader);
		m_listHeader = NULL;
	}

	std::list<std::string>::iterator itor = listHeader.begin();
	for (itor; itor != listHeader.end(); itor++)
	{
		std::string singleHeader = *itor;
		m_listHeader = curl_slist_append(m_listHeader, singleHeader.c_str());
	}
}

void CHTTPCore::resetCookiesAfterPerform() {

	if (getUrlResHandle() == NULL)
	{
		return;
	}

	struct curl_slist* cookies = NULL;
	CURLcode resCode = curl_easy_getinfo(getUrlResHandle(), CURLINFO_COOKIELIST, &cookies);
	if (resCode == CURLE_OK && cookies != NULL)	{

		m_strCookies.clear();

		struct curl_slist* itor = cookies;
		while (itor) {

			if (!m_strCookies.empty())
			{
				m_strCookies.append(";");
			}

			m_strCookies.append(itor->data);

			itor = itor->next;	
		}
		curl_slist_free_all(cookies);
	}
}

size_t CHTTPCore::HTTPHeaderCallback(char *buffer, size_t size, size_t nitems, void *userp) {

	httpWriteLock wl(m_gHttpMutex);

	if (userp)
	{
		CHTTPCore* pHTTPCore = (CHTTPCore*)userp;
		pHTTPCore->onHTTPHeaderContent(buffer, size * nitems);
	}

	return size * nitems;
}

size_t CHTTPCore::HTTPWriteCallback(void* data, size_t size, size_t nmemb, void* userp) {
	httpWriteLock wl(m_gHttpMutex);

    if (m_bStopCurrentTask) {
        m_bStopCurrentTask = false;
        return 0;
    }
	
	if (userp) {
		CHTTPCore* pHTTPCore = (CHTTPCore*)userp;
		pHTTPCore->onHTTPResponseContent((const char*)data, size * nmemb);
	}

	return (size * nmemb);
}

static DWORD dwReadLengthTotal = 0;

size_t CHTTPCore::HTTPReadCallback(void* data, size_t size, size_t nmemb, void* userp) {
	httpWriteLock wl(m_gHttpMutex);

	int readed = (size * nmemb);

	if (userp) {
		CHTTPCore* pHTTPCore = (CHTTPCore*)userp;
		readed = pHTTPCore->onHTTPReadContent((char*)data, (size * nmemb));
	}

	dwReadLengthTotal = dwReadLengthTotal + readed;

	char szLog[1024] = {0};
	sprintf_s(szLog, "[%s]readed:%d, total:%d", __FUNCTION__, readed, dwReadLengthTotal);
	OutputDebugStringA(szLog);
	return readed;
}

std::string CHTTPCore::getLastRequestLocalIp()
{
	return m_strLastRequestLocalIp;
}