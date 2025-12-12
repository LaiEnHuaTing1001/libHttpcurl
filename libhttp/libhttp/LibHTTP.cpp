// libhttp.cpp : 定义 DLL 应用程序的导出函数。
//

#include "libhttp.h"
#include "curl/curl.h"

#include "HTTPCore.h"
#include "HTTPRequest.h"
#include "HTTPDownload.h"
#include "HTTPUpload.h"

#include "WinINetRequest.h"
#include "WinINetCore.h"
#include "WinINetDownloader.h"
#include "WinINetUpload.h"

#pragma comment(lib, "libcurl_a.lib")

LIBHTTP_API int httpRequest(const char* szUrl, char** pResponse, char** pHeader, const char* szParam, const char* cookies, bool isPost, int timeoutMs, char* szHeader) {
	INetCore::m_gHttpContinue = TRUE;

	if (szUrl == NULL) {
		return ERROR_REQUEST_PARAM;
	}

	std::string strParam;
	if (szParam != NULL) {
		strParam.append(szParam);
	}

	std::string strHeader;
	std::string strResponse;


#ifdef USE_WININET

	CWinINetRequest	clWinInetRequest;
	clWinInetRequest.setTimeout(timeoutMs);

	int resCode = clWinInetRequest.httpRequest(szUrl, strParam, isPost);
	if (resCode != ERROR_SUCCESS)
	{
		return resCode;
	}

	strResponse = clWinInetRequest.getResponse();
	strHeader = clWinInetRequest.getHeader();

#else
	CHTTPRequest clHttpRequest;
	clHttpRequest.setTimeout(timeoutMs);
	clHttpRequest.addHeader(vecList);

	int resCode = clHttpRequest.httpRequest(szUrl, strParam, isPost);
	if (resCode != ERROR_SUCCESS)
	{
		return resCode;
	}

	strResponse = clHttpRequest.getResponse();
	strHeader = clHttpRequest.getHeader();
#endif	/*USE_WININET*/


	if (strResponse.empty())
	{
		return ERROR_RESPONSE_EMPTY;
	}

	if (pHeader)
	{
		if (!strHeader.empty())
		{
			*pHeader = new char[strHeader.length() + 2];
			memset(*pHeader, 0, strHeader.length() + 2);

			strcpy_s(*pHeader, strHeader.length() + 2, strHeader.c_str());
		}
	}

	if (pResponse)
	{
		*pResponse = new char[strResponse.length() + 2];
		memset(*pResponse, 0, strResponse.length() + 2);

		strcpy_s(*pResponse, strResponse.length() + 2, strResponse.c_str());
	}

	return ERROR_SUCCESS;
}

LIBHTTP_API int httpDownloader(const char* szUrl, const char* szFileName, const char* szParam, void* progressCallback, void* lpUser, int timeoutMs, char* szHeader) {
	INetCore::m_gHttpContinue = TRUE;

	if (szUrl == NULL) {
		return ERROR_REQUEST_PARAM;
	}

	if (szFileName == NULL)
	{
		return ERROR_REQUEST_PARAM;
	}

	std::string strParam;
	if (szParam != NULL) {
		strParam.append(szParam);
	}

#ifdef USE_WININET
	CWinINetDownloader clHttpDownloader;
	clHttpDownloader.setTimeout(timeoutMs);
	return clHttpDownloader.httpDownload(szUrl, strParam, szFileName, (progressFn)progressCallback, lpUser);

#else
	CHTTPDownload clHttpDownloader;
	clHttpDownloader.setTimeout(timeoutMs);
	clHttpDownloader.addHeader(vecList);
	return clHttpDownloader.httpDownload(szUrl, strParam, szFileName, (progressFn)progressCallback, lpUser);
#endif
}


LIBHTTP_API int httpFileUploader(const char* szUrl, const char* szFileName, const char* szParam, void* progressCallback, void* lpUser, char* szHeader) {
	INetCore::m_gHttpContinue = TRUE;
	if (szUrl == NULL) {
		return ERROR_REQUEST_PARAM;
	}

	if (szFileName == NULL)	{
		return ERROR_REQUEST_PARAM;
	}

	std::string strParam;
	if (szParam != NULL) {
		strParam.append(szParam);
	}

#ifdef USE_WININET
	CWinINetUpload clWinINetUploader;
	return clWinINetUploader.httpFileSubmit(szUrl, strParam, szFileName, (progressFn)progressCallback, lpUser);
#else
	CHTTPUpload clHttpUploader;
	clHttpUploader.addHeader(vecList);
	return clHttpUploader.httpFileSubmit(szUrl, strParam, szFileName, (progressFn)progressCallback, lpUser);
#endif
}

LIBHTTP_API int httpFilePost(const char* szUrl, const char* szFileName, const char* szParam, void* progressCallback, void* lpUser) {

	INetCore::m_gHttpContinue = TRUE;

	if (szUrl == NULL)
	{
		return ERROR_REQUEST_PARAM;
	}

	if (szFileName == NULL)
	{
		return ERROR_REQUEST_PARAM;
	}

	std::string strParam;
	if (szParam != NULL)
	{
		strParam.append(szParam);
	}

	CHTTPUpload clHttpUploader;
	return clHttpUploader.httpFilePost(szUrl, strParam, szFileName, (progressFn)progressCallback, lpUser);
}



LIBHTTP_API void httpRelease(void* pResponse) {
	if (pResponse) {
		delete[] pResponse;
		pResponse = NULL;
	}
}

LIBHTTP_API void abortAllRequest(){

	INetCore::m_gHttpContinue = FALSE;
}