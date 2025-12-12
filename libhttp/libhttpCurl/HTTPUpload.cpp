
#include "HTTPUpload.h"
#include "util.hpp"
#include <boost/algorithm/string.hpp>

CURL* CHTTPUpload::m_pUrlUploadHandle = NULL;

CHTTPUpload::CHTTPUpload(void) {
	m_pfnProgressCallback = NULL;
	m_lpUserPointer = NULL;
	m_hPostSrcFile = NULL;

    if (m_pUrlUploadHandle == NULL) {
        m_pUrlUploadHandle = curl_easy_init();
    }
    
}

CHTTPUpload::~CHTTPUpload(void) {

	if (m_hPostSrcFile != INVALID_HANDLE_VALUE && m_hPostSrcFile != NULL)
	{
		CloseHandle(m_hPostSrcFile);
		m_hPostSrcFile = NULL;
	}
}

CURL* CHTTPUpload::getUrlResHandle()
{
	return m_pUrlUploadHandle;
}

int CHTTPUpload::onHTTPReadContent(char* pData, int size)
{
	if (m_hPostSrcFile != INVALID_HANDLE_VALUE && m_hPostSrcFile != NULL)
	{
		DWORD readed = 0;
		BOOL res = ReadFile(m_hPostSrcFile, pData, size, &readed, NULL);
		if (res)
		{
			return readed;
		}
	}

	return CURL_READFUNC_ABORT;
}

int CHTTPUpload::httpFilePost(std::string url, std::string strfn, progressFn progressCallback, void* lpUser)
{
	curlOptInit();

	m_pfnProgressCallback = progressCallback;
	m_lpUserPointer = lpUser;

	std::string strFileNameGBK = strfn;
	if (keyword_search::util::is_utf8(strfn.c_str(), strfn.length()))
	{
		strFileNameGBK = keyword_search::util::from_utf8(strfn.c_str());
	}

	if (getUrlResHandle() == NULL) {
		LOGE("[%s]http upload, url handle is null, return -4", __FUNCTION__);
		return ERROR_CURL_LIB;
	}

	if (strFileNameGBK.empty())
	{
		LOGE("[%s]local file is empty, return -3", __FUNCTION__);
		return ERROR_REQUEST_PARAM;
	}

	m_hPostSrcFile = CreateFileA(strFileNameGBK.c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hPostSrcFile == INVALID_HANDLE_VALUE) {
		LOGE("[%s]create local file failed, path:%s, ec:%d, return -2", __FUNCTION__, strFileNameGBK.c_str(), GetLastError());
		return ERROR_CREATE_FILE;
	}
	
	std::string filename = keyword_search::util::filename_without_path(strfn);
	char *output = curl_easy_escape(m_pUrlUploadHandle, filename.c_str(), filename.size());
	if(output)
	{
		boost::algorithm::replace_all(url, filename, output);
	}
	
	setTransmissionSpeeds();
	curl_easy_setopt(getUrlResHandle(), CURLOPT_URL, url.c_str());

	curl_easy_setopt(getUrlResHandle(), CURLOPT_POST, 1L);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_CONNECTTIMEOUT_MS, m_nTimeout);

	curl_easy_setopt(getUrlResHandle(), CURLOPT_READDATA, this);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_READFUNCTION, CHTTPCore::HTTPReadCallback);

	curl_easy_setopt(getUrlResHandle(), CURLOPT_WRITEDATA, this);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_WRITEFUNCTION, CHTTPCore::HTTPWriteCallback);

	LARGE_INTEGER lgFileSize;
	GetFileSizeEx(m_hPostSrcFile, &lgFileSize);
	curl_easy_setopt(getUrlResHandle(), CURLOPT_POSTFIELDSIZE_LARGE, lgFileSize.QuadPart);

	char szContentLength[MAX_PATH] = {0};
	sprintf_s(szContentLength, MAX_PATH, "Content-Length: %I64d", lgFileSize.QuadPart);
	m_listHeader = curl_slist_append(m_listHeader, szContentLength);
	m_listHeader = curl_slist_append(m_listHeader, "Content-Type: application/octet-stream");
	m_listHeader = curl_slist_append(m_listHeader, "Expect:");
	curl_easy_setopt(getUrlResHandle(), CURLOPT_HTTPHEADER, m_listHeader);

    setProxy();

	CURLcode resCode = curl_easy_perform(getUrlResHandle());
	long statusCode = 200;
	curl_easy_getinfo(getUrlResHandle(), CURLINFO_RESPONSE_CODE, &statusCode);
	LOGD("[%s]http upload finished, resCode:%d, response code:%d , url :%s", __FUNCTION__, resCode, statusCode,url.c_str());

	char* szRequestLocalIp = NULL;	//	for libcurl document, the point must not free
	CURLcode localIpRes = curl_easy_getinfo(getUrlResHandle(), CURLINFO_LOCAL_IP, &szRequestLocalIp);
	if (localIpRes == CURLE_OK)
	{
		m_strLastRequestLocalIp = std::string(szRequestLocalIp);
	}

	if (m_hPostSrcFile != INVALID_HANDLE_VALUE && m_hPostSrcFile != NULL)
	{
		CloseHandle(m_hPostSrcFile);
		m_hPostSrcFile = NULL;
	}
	
	if(output)
		curl_free(output);

	if (resCode == CURLE_OK && statusCode != 200)
	{
		return statusCode;
	}

	if (resCode != CURLE_OK) {
		LOGE("curl perform failed:%d\n", resCode);
		if (resCode == CURLE_OPERATION_TIMEDOUT) {
			LOGE("[%s]http download timeout, return -6", __FUNCTION__);
			return ERROR_CURL_TIMEOUT;
		} else if (resCode == CURLE_ABORTED_BY_CALLBACK) {
			LOGE("[%s]http download aborted, return -5", __FUNCTION__);
			return ERROR_USER_ABORT;
		} else {
			LOGE("[%s]http download failed, res:%d return -1", __FUNCTION__, resCode);
			return resCode;
		}
	}

	return ERROR_SUCCESS;

}

void CHTTPUpload::onHTTPResponseContent(const char* pData, int nSize)
{
	if (pData)
	{
		m_strResponseContent.append(pData);
	}
}

std::string CHTTPUpload::getResponse()
{
	return m_strResponseContent;
}