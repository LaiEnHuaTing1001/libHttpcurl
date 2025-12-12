#include "HTTPDownload.h"

CHTTPDownload::CHTTPDownload(void) {
	m_lpUserPointer = NULL;
	m_lpProgressFn  = NULL;

	m_lpStoreFile = NULL;

	m_i64DownloadTotal = 0;
	m_i64DownloadNow = 0;
}

CHTTPDownload::~CHTTPDownload(void) {
}

void CHTTPDownload::onHTTPResponseContent(const char* pData, int nSize) {
	if (m_lpStoreFile != INVALID_HANDLE_VALUE) {
		DWORD dwWriten = 0;
		WriteFile(m_lpStoreFile, pData, nSize, &dwWriten, NULL);

		if (m_i64DownloadTotal != 0 && m_lpProgressFn) {
			m_i64DownloadNow += dwWriten;

			m_lpProgressFn(m_i64DownloadNow, m_i64DownloadTotal, m_lpUserPointer);
		}
	}
}

int CHTTPDownload::onHTTPDownloadProgress(INT64 iNow, INT64 iTotal) {
	if (m_lpProgressFn && (m_i64DownloadTotal == 0)) {	//	当没有获取到文件大小时，才使用curl返回的进度

		m_lpProgressFn(iNow, iTotal, m_lpUserPointer);
	}
	return 0;
}

int CHTTPDownload::httpDownload(std::string url, std::string param, std::string szfn, progressFn progressCallback, void* lpUser) {
	writeLock wl(m_gRequestMutex);

	m_lpProgressFn = progressCallback;
	m_lpUserPointer = lpUser;

	if (m_pUrlRes == NULL) {
		return ERROR_CURL_LIB;
	}

	m_i64DownloadTotal = getRequestSize(url.c_str());
	

	m_lpStoreFile = CreateFileA(szfn.c_str(), FILE_WRITE_DATA, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_lpStoreFile == INVALID_HANDLE_VALUE) {
		return ERROR_CREATE_FILE;
	}

	if (url.find(HTTPS_PROTOCOL_HEADER) == 0) {
		//	https protocol
		curl_easy_setopt(m_pUrlRes, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(m_pUrlRes, CURLOPT_SSL_VERIFYHOST, 0L);

	}

	curl_easy_setopt(m_pUrlRes, CURLOPT_URL, url.c_str());
	curl_easy_setopt(m_pUrlRes, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(m_pUrlRes, CURLOPT_TIMEOUT_MS, m_nTimeout);

	curl_easy_setopt(m_pUrlRes, CURLOPT_HEADER, 0L);
	curl_easy_setopt(m_pUrlRes, CURLOPT_NOBODY, 0L);

	curl_easy_setopt(m_pUrlRes, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(m_pUrlRes, CURLOPT_WRITEFUNCTION, CHTTPCore::HTTPWriteCallback);

	curl_easy_setopt(m_pUrlRes, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(m_pUrlRes, CURLOPT_XFERINFODATA, this);
	curl_easy_setopt(m_pUrlRes, CURLOPT_XFERINFOFUNCTION, CHTTPCore::HTTPProgressCallback);

	if (!param.empty()) {
		curl_easy_setopt(m_pUrlRes, CURLOPT_POSTFIELDSIZE, param.length());
		curl_easy_setopt(m_pUrlRes, CURLOPT_POSTFIELDS, param.c_str());
	}

	addGlobalHeaderOpt();

	CURLcode resCode = curl_easy_perform(m_pUrlRes);

	if (m_lpStoreFile != INVALID_HANDLE_VALUE && m_lpStoreFile != NULL)
	{
		CloseHandle(m_lpStoreFile);
	}

	if (resCode != CURLE_OK) {
		LOGE("curl perform failed:%d\n", resCode);
		//return ERROR_HTTP_HOST;
		if (resCode == CURLE_OPERATION_TIMEDOUT) {
			return ERROR_CURL_TIMEOUT;
		} else if (resCode == CURLE_ABORTED_BY_CALLBACK) {
			return ERROR_USER_ABORT;
		} else {
			return ERROR_HTTP_HOST;
		}
	}

	return ERROR_SUCCESS;
}