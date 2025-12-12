#include "HTTPRequest.h"

CURL* CHTTPRequest::m_pUrlRequestHandle = NULL;

CHTTPRequest::CHTTPRequest(void){
	if (m_pUrlRequestHandle == NULL)
	{
		m_pUrlRequestHandle = curl_easy_init();
		if (m_pUrlRequestHandle == NULL)
		{
			return;
		}
	}
}

CHTTPRequest::~CHTTPRequest(void){
}

void CHTTPRequest::Close() {
    if (m_pUrlRequestHandle != NULL) {
        curl_easy_cleanup(m_pUrlRequestHandle);
        m_pUrlRequestHandle = NULL;
    }
}

CURL* CHTTPRequest::getUrlResHandle()
{
    if (m_pUrlRequestHandle == NULL) {
        m_pUrlRequestHandle = curl_easy_init();
    }

	return m_pUrlRequestHandle;
}

std::string CHTTPRequest::getResponse(){
	return m_strResponseContent;
}

std::string CHTTPRequest::getHeader() {
	return m_strHeaderContent;
}

void CHTTPRequest::onHTTPResponseContent(const char* pData, int nSize) {
	if (pData) {
		m_strResponseContent.append(pData);
	}
}

int CHTTPRequest::onHTTPHeaderContent(const char* pData, int nSize) {

	if (pData)
	{
		m_strHeaderContent.append(pData);
	}

	return nSize;
}

int CHTTPRequest::httpRequest(std::string url, std::string param, bool isPost) {
	curlOptInit();
	m_strResponseContent.clear();
	if (getUrlResHandle() == NULL) {
		LOGE("[%s]request error, urlres handle is null", __FUNCTION__);
		return ERROR_CURL_LIB;
	}
	if (url.find(HTTPS_PROTOCOL_HEADER) == 0) {
		//	https protocol
		curl_easy_setopt(getUrlResHandle(), CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(getUrlResHandle(), CURLOPT_SSL_VERIFYHOST, 0L);

	}
	setTransmissionSpeeds();
	LOGD("[%s]request real url:%s", __FUNCTION__, url.c_str());
	curl_easy_setopt(getUrlResHandle(), CURLOPT_URL, url.c_str());

	curl_easy_setopt(getUrlResHandle(), CURLOPT_WRITEDATA, this);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_WRITEFUNCTION, CHTTPCore::HTTPWriteCallback);

	curl_easy_setopt(getUrlResHandle(), CURLOPT_HEADERDATA, this);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_HEADERFUNCTION, CHTTPCore::HTTPHeaderCallback);
	int checkTimeout = m_nTimeout;
	if (CHTTPCore::ms_uMaxUploadSpeeds != 0)
	{
		int transmitTimeout = (((url.length() + param.length()) / CHTTPCore::ms_uMaxUploadSpeeds) + 3) * 1000;
		if (transmitTimeout >= m_nTimeout)
		{
			checkTimeout = transmitTimeout;
		}
	}
	curl_easy_setopt(getUrlResHandle(), CURLOPT_CONNECTTIMEOUT_MS, checkTimeout);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_TIMEOUT_MS, checkTimeout);

	if (isPost) {
		curl_easy_setopt(getUrlResHandle(), CURLOPT_POST, 1L);
	} else {
		curl_easy_setopt(getUrlResHandle(), CURLOPT_POST, 0L);
	}

	if (!param.empty()) {
		curl_easy_setopt(getUrlResHandle(), CURLOPT_POSTFIELDSIZE, param.length());
		curl_easy_setopt(getUrlResHandle(), CURLOPT_POSTFIELDS, param.c_str());
	}
	else 
	{
		curl_easy_setopt(getUrlResHandle(), CURLOPT_POSTFIELDSIZE, 0);
	}

	curl_easy_setopt(getUrlResHandle(), CURLOPT_HTTPHEADER, m_listHeader);

    setProxy();

	CURLcode resCode = curl_easy_perform(getUrlResHandle());
	long statusCode = 200;
	curl_easy_getinfo(getUrlResHandle(), CURLINFO_RESPONSE_CODE, &statusCode);
	resetCookiesAfterPerform();

	LOGD("[%s]request finished, rescode:%d, response code:%d, timeout:%d", __FUNCTION__, resCode, statusCode, checkTimeout);
	char* szRequestLocalIp = NULL;	//	for libcurl document, the point must not free
	CURLcode infoRes = curl_easy_getinfo(getUrlResHandle(), CURLINFO_LOCAL_IP, &szRequestLocalIp);
	LOGD("[%s]get local ip, rescode:%d", __FUNCTION__, infoRes);
	if (infoRes == CURLE_OK && szRequestLocalIp)
	{
		m_strLastRequestLocalIp = std::string(szRequestLocalIp);
	}

	if (resCode == CURLE_OK	&& statusCode != 200)
	{
		return statusCode;
	}

	if (resCode != CURLE_OK) {
		LOGE("curl perform failed:%d\n", resCode);
		if (resCode == CURLE_OPERATION_TIMEDOUT) {
			return ERROR_CURL_TIMEOUT;
		}
		else if (resCode == CURLE_ABORTED_BY_CALLBACK) {
			return ERROR_USER_ABORT;
		} else {
			LOGD("[%s]rescode:%d,but return -1", __FUNCTION__, resCode);
			return ERROR_HTTP_HOST;
		}
	}



	return ERROR_SUCCESS;
}