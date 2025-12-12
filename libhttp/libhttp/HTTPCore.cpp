#include "HTTPCore.h"

BOOL	INetCore::m_gHttpContinue = TRUE;
rwmutex CHTTPCore::m_gHttpMutex;

CHTTPCore::CHTTPCore(void) {
	m_pUrlRes = curl_easy_init();

	m_nTimeout = HTTP_TIME_OUT_MS;
}


CHTTPCore::~CHTTPCore(void) {
	if (m_pUrlRes) {
		curl_easy_cleanup(m_pUrlRes);
		m_pUrlRes = NULL;
	}
}

void CHTTPCore::onHTTPResponseContent(const char* pData, int nSize) {
	return;
}

int CHTTPCore::onHTTPReadContent(char* pData, int size) {
	return 0;
}

int CHTTPCore::onHTTPDownloadProgress(INT64 iNow, INT64 iTotal) {
	return CURL_PROGRESSFUNC_CONTINUE;
}

int CHTTPCore::onHTTPUploadProgress(INT64 iNow, INT64 iTotal) {
	return CURL_PROGRESSFUNC_CONTINUE;
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

void CHTTPCore::addGlobalHeaderOpt() {

	if (!m_strUserAgent.empty()) {
		curl_easy_setopt(m_pUrlRes, CURLOPT_USERAGENT, m_strUserAgent.c_str());
	} else {
		curl_easy_setopt(m_pUrlRes, CURLOPT_USERAGENT, USER_ANGENT);
	}

	if (!m_strRefererUrl.empty())
	{
		curl_easy_setopt(m_pUrlRes, CURLOPT_REFERER, m_strRefererUrl.c_str());
	}

	if (!m_strCookies.empty())
	{
		curl_easy_setopt(m_pUrlRes, CURLOPT_COOKIE, m_strCookies.c_str());
	}
}

void CHTTPCore::resetCookiesAfterPerform() {

	if (m_pUrlRes == NULL)
	{
		return;
	}

	struct curl_slist* cookies = NULL;
	CURLcode resCode = curl_easy_getinfo(m_pUrlRes, CURLINFO_COOKIELIST, &cookies);
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

INT64 CHTTPCore::getRequestSize(std::string url) {
	if (m_pUrlRes == NULL) {
		return ERROR_CURL_LIB;
	}

	if (url.find(HTTPS_PROTOCOL_HEADER) == 0) {
		//	https protocol
		curl_easy_setopt(m_pUrlRes, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(m_pUrlRes, CURLOPT_SSL_VERIFYHOST, 0L);
	}

	curl_easy_setopt(m_pUrlRes, CURLOPT_URL, url.c_str());
	curl_easy_setopt(m_pUrlRes, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(m_pUrlRes, CURLOPT_HEADER, 1L);
	curl_easy_setopt(m_pUrlRes, CURLOPT_NOBODY, 1L);
	curl_easy_setopt(m_pUrlRes, CURLOPT_TIMEOUT, m_nTimeout);
	curl_easy_setopt(m_pUrlRes, CURLOPT_NOPROGRESS, 1L);

	curl_easy_setopt(m_pUrlRes, CURLOPT_WRITEDATA, NULL);
	curl_easy_setopt(m_pUrlRes, CURLOPT_WRITEFUNCTION, CHTTPCore::HTTPWriteCallback);

	curl_easy_setopt(m_pUrlRes, CURLOPT_XFERINFODATA, NULL);
	curl_easy_setopt(m_pUrlRes, CURLOPT_XFERINFOFUNCTION, CHTTPCore::HTTPProgressCallback);

	addGlobalHeaderOpt();

	CURLcode resCode = curl_easy_perform(m_pUrlRes);
	if (resCode == CURLE_OK) {
		curl_off_t contentSize = 0;
		if (curl_easy_getinfo(m_pUrlRes, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &contentSize)) {
			return contentSize;
		}
	}

	return 0;
}

size_t CHTTPCore::HTTPHeaderCallback(char *buffer, size_t size, size_t nitems, void *userp) {

	writeLock wl(m_gHttpMutex);

	if (userp)
	{
		CHTTPCore* pHTTPCore = (CHTTPCore*)userp;
		pHTTPCore->onHTTPHeaderContent(buffer, size * nitems);
	}

	return size * nitems;
}

size_t CHTTPCore::HTTPWriteCallback(void* data, size_t size, size_t nmemb, void* userp) {
	writeLock wl(m_gHttpMutex);

	
	if (userp) {
		CHTTPCore* pHTTPCore = (CHTTPCore*)userp;
		pHTTPCore->onHTTPResponseContent((const char*)data, size * nmemb);
	}

	if (m_gHttpContinue) {
		return (size * nmemb);
	}
	
	return 0;
}

size_t CHTTPCore::HTTPReadCallback(void* data, size_t size, size_t nmemb, void* userp) {
	writeLock wl(m_gHttpMutex);

	int readed = (size * nmemb);

	if (userp) {
		CHTTPCore* pHTTPCore = (CHTTPCore*)userp;
		readed = pHTTPCore->onHTTPReadContent((char*)data, (size * nmemb));
	}

	if (m_gHttpContinue) {
		return readed;
	}
	return CURL_READFUNC_ABORT;
}

size_t CHTTPCore::HTTPProgressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
	
	writeLock wl(m_gHttpMutex);

	if (clientp) {

		CHTTPCore* pHttpCore = (CHTTPCore*)clientp;
		if (dlnow > 0) {
			pHttpCore->onHTTPDownloadProgress(dlnow, dltotal);
		}

		if (ulnow > 0) {
			pHttpCore->onHTTPUploadProgress(ulnow, ultotal);
		}
	}

	if (m_gHttpContinue) {
		return CURL_PROGRESSFUNC_CONTINUE;
	} else {
		return -1;
	}
}