#include "HTTPRequest.h"

CHTTPRequest::CHTTPRequest(void){
}

CHTTPRequest::~CHTTPRequest(void){
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
	writeLock wl(m_gRequestMutex);

	m_strResponseContent.clear();

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

	curl_easy_setopt(m_pUrlRes, CURLOPT_HEADER, 0L);
	curl_easy_setopt(m_pUrlRes, CURLOPT_NOBODY, 0L);
	curl_easy_setopt(m_pUrlRes, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(m_pUrlRes, CURLOPT_TCP_KEEPINTVL, 30L);

	curl_easy_setopt(m_pUrlRes, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(m_pUrlRes, CURLOPT_WRITEFUNCTION, CHTTPCore::HTTPWriteCallback);

	curl_easy_setopt(m_pUrlRes, CURLOPT_HEADERDATA, this);
	curl_easy_setopt(m_pUrlRes, CURLOPT_HEADERFUNCTION, CHTTPCore::HTTPHeaderCallback);

	curl_easy_setopt(m_pUrlRes, CURLOPT_TIMEOUT_MS, m_nTimeout);
	if (isPost) {
		curl_easy_setopt(m_pUrlRes, CURLOPT_POST, 1L);
	} else {
		curl_easy_setopt(m_pUrlRes, CURLOPT_POST, 0L);
	}

	if (!param.empty()) {
		curl_easy_setopt(m_pUrlRes, CURLOPT_POSTFIELDSIZE, param.length());
		curl_easy_setopt(m_pUrlRes, CURLOPT_POSTFIELDS, param.c_str());
	}

	addGlobalHeaderOpt();

	CURLcode resCode = curl_easy_perform(m_pUrlRes);

	resetCookiesAfterPerform();

	if (resCode != CURLE_OK) {
		LOGE("curl perform failed:%d\n", resCode);
		if (resCode == CURLE_OPERATION_TIMEDOUT) {
			return ERROR_CURL_TIMEOUT;
		}
		else if (resCode == CURLE_ABORTED_BY_CALLBACK) {
			return ERROR_USER_ABORT;
		} else {
			return ERROR_HTTP_HOST;
		}
	}



	return ERROR_SUCCESS;
}