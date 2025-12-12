#include "HTTPDownload.h"
#include "util.hpp"

CURL* CHTTPDownload::m_pUrlDownloadHandle = NULL;
bool CHTTPCore::m_bStopCurrentTask = false;

CHTTPDownload::CHTTPDownload(void) {
	m_lpUserPointer = NULL;
	m_lpProgressFn  = NULL;

	m_lpStoreFile = NULL;

	m_i64DownloadTotal = 0;
	m_i64DownloadNow = 0;
	m_tmLastDownloadTranslate = _time32(NULL);

    if (m_pUrlDownloadHandle == NULL) {
        m_pUrlDownloadHandle = curl_easy_init();
    }
}

CHTTPDownload::~CHTTPDownload(void) {
	if (m_lpStoreFile != INVALID_HANDLE_VALUE && m_lpStoreFile != NULL)
	{
		CloseHandle(m_lpStoreFile);
		m_lpStoreFile = NULL;
	}
}

CURL* CHTTPDownload::getUrlResHandle()
{
	return m_pUrlDownloadHandle;
}

void CHTTPDownload::onHTTPResponseContent(const char* pData, int nSize) {
	if (m_lpStoreFile && m_lpStoreFile != INVALID_HANDLE_VALUE) {
		DWORD dwWriten = 0;
		WriteFile(m_lpStoreFile, pData, nSize, &dwWriten, NULL);

		if (nSize > 0)
		{
			m_tmLastDownloadTranslate = _time32(NULL);
		}

		if (m_lpProgressFn) {
			m_i64DownloadNow += dwWriten;

			m_lpProgressFn(m_i64DownloadNow, m_lpUserPointer);
		}
	}
}

int CHTTPDownload::onHTTPDownloadProgress(double dlTotal, double dlNow, double ulTotal, double ulNow)
{
	__time32_t tmNow = _time32(NULL);
	if (abs(tmNow - m_tmLastDownloadTranslate) > (60 * 15))
	{
		LOGE("[%s]download progress translate timeout, last:%d, now:%d", __FUNCTION__, m_tmLastDownloadTranslate, tmNow);
		return CURLE_ABORTED_BY_CALLBACK;
	}

	return CURL_PROGRESSFUNC_CONTINUE;
}

int CHTTPDownload::httpDownload(std::string url, std::string param, std::string strfn, progressFn progressCallback, void* lpUser) {
	m_tmLastDownloadTranslate = _time32(NULL);

	curlOptInit();

	std::string strFileNameGBK = strfn;
	if (keyword_search::util::is_utf8(strfn.c_str(), strfn.length()))
	{
		strFileNameGBK = keyword_search::util::from_utf8(strfn.c_str());
	}

	m_lpProgressFn = progressCallback;
	m_lpUserPointer = lpUser;

	if (getUrlResHandle() == NULL) {
		LOGE("[%s]http download, url handle is null, return -4", __FUNCTION__);
		return ERROR_CURL_LIB;
	}	

	m_lpStoreFile = CreateFileA(strFileNameGBK.c_str(), FILE_WRITE_DATA, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_lpStoreFile == INVALID_HANDLE_VALUE) {
		LOGE("[%s]create local file failed, path:%s, ec:%d, return -2", __FUNCTION__, strFileNameGBK.c_str(), GetLastError());
		return ERROR_CREATE_FILE;
	}

    m_i64DownloadNow = 0;
	setTransmissionSpeeds();
	curl_easy_setopt(getUrlResHandle(), CURLOPT_URL, url.c_str());
	curl_easy_setopt(getUrlResHandle(), CURLOPT_CONNECTTIMEOUT_MS, m_nTimeout);

	curl_easy_setopt(getUrlResHandle(), CURLOPT_WRITEDATA, this);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_WRITEFUNCTION, CHTTPCore::HTTPWriteCallback);

	curl_easy_setopt(getUrlResHandle(), CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_PROGRESSFUNCTION, CHTTPDownload::HTTPProgressCallback);

	if (!param.empty()) {
		curl_easy_setopt(getUrlResHandle(), CURLOPT_POSTFIELDSIZE, param.length());
		curl_easy_setopt(getUrlResHandle(), CURLOPT_POSTFIELDS, param.c_str());
	}

	curl_easy_setopt(getUrlResHandle(), CURLOPT_HTTPHEADER, m_listHeader);

    setProxy();

	CURLcode resCode = curl_easy_perform(getUrlResHandle());
	long statusCode = 200;
	curl_easy_getinfo(getUrlResHandle(), CURLINFO_RESPONSE_CODE, &statusCode);

	LOGD("[%s]http download finished, resCode:%d, responseCode:%d", __FUNCTION__, resCode, statusCode);
	if (m_lpStoreFile != INVALID_HANDLE_VALUE && m_lpStoreFile != NULL)
	{
		CloseHandle(m_lpStoreFile);
		m_lpStoreFile = NULL;
	}

	char* szRequestLocalIp = NULL;	//	for libcurl document, the point must not free
	CURLcode localIpRes = curl_easy_getinfo(getUrlResHandle(), CURLINFO_LOCAL_IP, &szRequestLocalIp);
	if (localIpRes == CURLE_OK && szRequestLocalIp)
	{
		m_strLastRequestLocalIp = std::string(szRequestLocalIp);
	}

	if (resCode == CURLE_OK && statusCode != 200)
	{
		DeleteFileA(strFileNameGBK.c_str());

		if (m_lpStoreFile != INVALID_HANDLE_VALUE && m_lpStoreFile != NULL)
		{
			CloseHandle(m_lpStoreFile);
			m_lpStoreFile = NULL;
		}
		return statusCode;
	}

	if (resCode != CURLE_OK) {
		//return ERROR_HTTP_HOST;
		DeleteFileA(strFileNameGBK.c_str());
		if (resCode == CURLE_OPERATION_TIMEDOUT) {
			LOGE("[%s]http download timeout, return -6", __FUNCTION__);
			return ERROR_CURL_TIMEOUT;
		} else if (resCode == CURLE_ABORTED_BY_CALLBACK) {
			LOGE("[%s]http download aborted, return -5", __FUNCTION__);
			return ERROR_USER_ABORT;
		} else {
			LOGE("[%s]http download failed, res:%d return -1", __FUNCTION__, resCode);
			return ERROR_HTTP_HOST;
		}
	}

	// m_lpStoreFile析构时释放，避免写操作未完成
	return ERROR_SUCCESS;
}

int CHTTPDownload::HTTPProgressCallback(void* userp, double dlTotle, double dlNow, double ulTotal, double ulNow)
{
	httpWriteLock wl(m_gHttpMutex);

	if (userp)
	{
		CHTTPDownload* pHTTPDownload = (CHTTPDownload*)userp;
		return pHTTPDownload->onHTTPDownloadProgress(dlTotle, dlTotle, ulTotal, ulNow);
	}
	else
	{
		return CURL_PROGRESSFUNC_CONTINUE;
	}
}