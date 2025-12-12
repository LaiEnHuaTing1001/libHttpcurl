
#include "HTTPUpload.h"

CHTTPUpload::CHTTPUpload(void) {
	m_pfnProgressCallback = NULL;
	m_lpUserPointer = NULL;
}

CHTTPUpload::~CHTTPUpload(void) {

}

int CHTTPUpload::onHTTPUploadProgress(INT64 iNow, INT64 iTotal) {
	if (m_pfnProgressCallback) {
		m_pfnProgressCallback(iNow, iTotal, m_lpUserPointer);
	}

	return 0;
}

int CHTTPUpload::onHTTPReadContent(char* pData, int size)
{
	if (m_hPostSrcFile != INVALID_HANDLE_VALUE && m_hPostSrcFile != NULL)
	{
		DWORD readed = 0;
		BOOL res = ReadFile(m_hPostSrcFile, pData, size, &readed, NULL);
		if (res && readed < size)
		{
			return 0;	// transfer end!!!
		}

		if (res)
		{
			return readed;
		}
	}

	return CURL_READFUNC_ABORT;
}

int CHTTPUpload::httpFileSubmit(std::string url, std::string param, std::string szfn, progressFn progressCallback, void* lpUser) {
	writeLock wl(m_gRequestMutex);

	m_pfnProgressCallback = progressCallback;
	m_lpUserPointer = lpUser;

	if (m_pUrlRes == NULL) {
		return ERROR_CURL_LIB;
	}

	curl_easy_setopt(m_pUrlRes, CURLOPT_URL, url.c_str());	
	curl_easy_setopt(m_pUrlRes, CURLOPT_VERBOSE, 0L);

	curl_easy_setopt(m_pUrlRes, CURLOPT_TCP_KEEPALIVE, 1L);
	if (url.find(HTTPS_PROTOCOL_HEADER) == 0) {
		//	https protocol
		curl_easy_setopt(m_pUrlRes, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(m_pUrlRes, CURLOPT_SSL_VERIFYHOST, 0L);

	}

	curl_easy_setopt(m_pUrlRes, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(m_pUrlRes, CURLOPT_XFERINFODATA, this);
	curl_easy_setopt(m_pUrlRes, CURLOPT_XFERINFOFUNCTION, CHTTPCore::HTTPProgressCallback);
	curl_easy_setopt(m_pUrlRes, CURLOPT_POST, 1L);
	curl_easy_setopt(m_pUrlRes, CURLOPT_TIMEOUT, m_nTimeout);

	if (!param.empty())	{
		curl_easy_setopt(m_pUrlRes, CURLOPT_POSTFIELDSIZE, param.length());
		curl_easy_setopt(m_pUrlRes, CURLOPT_POSTFIELDS, param.c_str());
	}

	//struct curl_slist* headerList = NULL;
	//headerList = curl_slist_append(headerList, "Expect:");
	//curl_easy_setopt(m_pUrlRes, CURLOPT_HTTPHEADER, headerList);

	struct curl_httppost* fromPost = NULL;
	struct curl_httppost* lastPost = NULL;


	curl_formadd(
			&fromPost, 
			&lastPost,
			CURLFORM_COPYNAME, "",
			CURLFORM_FILE, szfn.c_str(),
			CURLFORM_END);

	curl_easy_setopt(m_pUrlRes, CURLOPT_HTTPPOST, fromPost);

	addGlobalHeaderOpt();
	CURLcode resCode = curl_easy_perform(m_pUrlRes);

	if (fromPost)
	{
		curl_formfree(fromPost);
	}
	//if (headerList)
	{
	//	curl_slist_free_all(headerList);
	}

	if (resCode != CURLE_OK) {
		LOGE("curl perform failed:%d\n", resCode);

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

int CHTTPUpload::httpFilePost(std::string url, std::string param, std::string szfn, progressFn progressCallback, void* lpUser)
{
	writeLock wl(m_gRequestMutex);

	m_pfnProgressCallback = progressCallback;
	m_lpUserPointer = lpUser;

	if (m_pUrlRes == NULL) {
		return ERROR_CURL_LIB;
	}

	if (szfn.empty())
	{
		return ERROR_REQUEST_PARAM;
	}

	m_hPostSrcFile = CreateFileA(szfn.c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hPostSrcFile == INVALID_HANDLE_VALUE) {
		return ERROR_CREATE_FILE;
	}

	curl_easy_setopt(m_pUrlRes, CURLOPT_URL, url.c_str());	
	curl_easy_setopt(m_pUrlRes, CURLOPT_VERBOSE, 0L);

	curl_easy_setopt(m_pUrlRes, CURLOPT_TCP_KEEPALIVE, 1L);
	if (url.find(HTTPS_PROTOCOL_HEADER) == 0) {
		//	https protocol
		curl_easy_setopt(m_pUrlRes, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(m_pUrlRes, CURLOPT_SSL_VERIFYHOST, 0L);

	}

	curl_easy_setopt(m_pUrlRes, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(m_pUrlRes, CURLOPT_XFERINFODATA, this);
	curl_easy_setopt(m_pUrlRes, CURLOPT_XFERINFOFUNCTION, CHTTPCore::HTTPProgressCallback);
	curl_easy_setopt(m_pUrlRes, CURLOPT_POST, 1L);
	curl_easy_setopt(m_pUrlRes, CURLOPT_TIMEOUT, m_nTimeout);

	if (!param.empty())	{
		curl_easy_setopt(m_pUrlRes, CURLOPT_POSTFIELDSIZE, param.length());
		curl_easy_setopt(m_pUrlRes, CURLOPT_POSTFIELDS, param.c_str());
	}

	curl_easy_setopt(m_pUrlRes, CURLOPT_READDATA, this);
	curl_easy_setopt(m_pUrlRes, CURLOPT_READFUNCTION, CHTTPCore::HTTPReadCallback);

	LARGE_INTEGER lgFileSize;
	GetFileSizeEx(m_hPostSrcFile, &lgFileSize);

	char szContentLength[MAX_PATH] = {0};
	sprintf_s(szContentLength, MAX_PATH, "Content-Length: %I64d", lgFileSize.QuadPart + param.length());
	struct curl_slist* slist = NULL;
	slist = curl_slist_append(slist, szContentLength);
	curl_easy_setopt(m_pUrlRes, CURLOPT_HTTPHEADER, slist);

	addGlobalHeaderOpt();

	CURLcode resCode = curl_easy_perform(m_pUrlRes);

	if (m_hPostSrcFile != INVALID_HANDLE_VALUE && m_hPostSrcFile != NULL)
	{
		CloseHandle(m_hPostSrcFile);
		m_hPostSrcFile = NULL;
	}

	if (slist)
	{
		curl_slist_free_all(slist);
		slist = NULL;
	}

	if (resCode != CURLE_OK) {
		LOGE("curl perform failed:%d\n", resCode);
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