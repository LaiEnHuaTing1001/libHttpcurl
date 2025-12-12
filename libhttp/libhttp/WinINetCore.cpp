#include "INetCore.h"
#include "HTTPGlobal.h"
#include "WinINetCore.h"

#define SUBMIT_BOUNDARY "------------------------bccef7fe2fe684cb"

#define SUBMIT_HEADER_L "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.67 Safari/537.36 Edg/87.0.664.52\r\nAccept: */*\r\nContent-Length: %lld\r\nContent-Type: multipart/form-data; boundary=%s\r\nExpect: 100-continue\r\n\r\n"
#define SUBMIT_MARK_L	"--%s\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\nContent-Type: application/octet-stream\r\n\r\n"
#define SUBMIT_TAIL_MARK_L "\r\n--%s--\r\n"

CWinINetCore::CWinINetCore(void)
{
	writeLock rw(m_gRequestMutex);
	m_ConnectHandle = NULL;
	m_RequestHandle = NULL;

	m_nTimeout = 0;
	m_nPort = 0;
	m_strHost.clear();
	m_strScheme.clear();
	m_strPath.clear();

	m_InternelHandle = InternetOpenA(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
}

CWinINetCore::~CWinINetCore(void)
{
	writeLock rw(m_gRequestMutex);

	if (m_InternelHandle)
	{
		InternetCloseHandle(m_InternelHandle);
		m_InternelHandle = NULL;
	}


}

void CWinINetCore::setTimeout(int timeout)
{
	m_nTimeout = (timeout == 0) ? HTTP_TIME_OUT_MS : timeout;

	InternetSetOptionA(m_InternelHandle, INTERNET_OPTION_CONNECT_TIMEOUT, &m_nTimeout, sizeof(DWORD));
}

bool CWinINetCore::doReadRequest(char* pBuffer, DWORD nSize, LPDWORD pdwReaded)
{
	return true;
}

void CWinINetCore::onUploadProgress(INT64 pos, INT64 total)
{

}

//	http & https only,default is http
void CWinINetCore::crackUrl(std::string& url, std::string& scheme, std::string& host, int& port, std::string& objName)
{
	if (url.empty())
	{
		return;
	}

	std::string waitUrl = "";
	if (url.find("://") == std::string::npos)
	{
		//	default http://
		waitUrl = std::string("http://") + url;
	}
	else
	{
		waitUrl = url;
	}

	if ((waitUrl.find("https://") != 0) && (waitUrl.find("http://") != 0))
	{
		return;
	}

	char tempScheme[10] = {0};
	std::auto_ptr<char> apHost(new char[url.length()]);
	std::auto_ptr<char> apUrlPath(new char[url.length()]);
	std::auto_ptr<char> apExtraInfo(new char[url.length()]);

	URL_COMPONENTSA urlComponents;
	memset(&urlComponents, 0, sizeof(URL_COMPONENTSA));
	urlComponents.dwStructSize = sizeof(URL_COMPONENTSA);

	urlComponents.dwSchemeLength = waitUrl.length();
	urlComponents.dwHostNameLength = waitUrl.length();
	urlComponents.dwExtraInfoLength = waitUrl.length();
	urlComponents.dwUrlPathLength = waitUrl.length();
	urlComponents.lpszScheme = tempScheme;
	urlComponents.lpszHostName = apHost.get();
	urlComponents.lpszExtraInfo = apExtraInfo.get();
	urlComponents.lpszUrlPath = apUrlPath.get();
	
	InternetCrackUrlA(waitUrl.c_str(), waitUrl.length(), ICU_DECODE, &urlComponents);

	scheme = std::string(urlComponents.lpszScheme);
	host = std::string(urlComponents.lpszHostName);
	port = urlComponents.nPort;
	objName = std::string(urlComponents.lpszUrlPath) + std::string(urlComponents.lpszExtraInfo);

	return;
}

void CWinINetCore::parseHeaderResponse(char* pHeader, unsigned int uSize, std::string& strHeader)
{
	if (pHeader == NULL || uSize == 0)
	{
		return;
	}

	strHeader.clear();


	char* tempHeader = pHeader;
	do 
	{
		strHeader += tempHeader;
		strHeader += "\r\n";

		tempHeader = tempHeader + strlen(tempHeader) + 1;
	} while (strlen(tempHeader));

	strHeader += "\r\n\r\n";
	return;
}

int CWinINetCore::openRequest(std::string url, bool isPost) 
{
	if (m_InternelHandle == NULL)
	{
		return ERROR_WININET_LIB;
	}

	crackUrl(url, m_strScheme, m_strHost, m_nPort, m_strPath);
	if (m_strHost.empty() || m_nPort == 0)
	{
		return ERROR_REQUEST_PARAM;
	}

	m_ConnectHandle = InternetConnectA(m_InternelHandle, m_strHost.c_str(), m_nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (m_ConnectHandle == NULL)
	{
		if (GetLastError() == ERROR_INTERNET_TIMEOUT)
		{
			return ERROR_CURL_TIMEOUT;
		}
		return ERROR_HTTP_HOST;
	}

	InternetSetOptionA(m_ConnectHandle, INTERNET_OPTION_CONNECT_TIMEOUT, &m_nTimeout, sizeof(DWORD));

	std::string verb = isPost ? "POST" : "GET";

	DWORD dwRequestFlag = INTERNET_FLAG_KEEP_CONNECTION|INTERNET_FLAG_NO_AUTO_REDIRECT|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_NO_COOKIES;
	if (m_strScheme.compare("https") == 0)
	{
		dwRequestFlag = dwRequestFlag | INTERNET_FLAG_SECURE;
	}
	m_RequestHandle = HttpOpenRequestA(m_ConnectHandle, verb.c_str(), m_strPath.c_str(), "HTTP/1.1", NULL, NULL, dwRequestFlag, NULL);
	if (m_RequestHandle == NULL)
	{
		InternetCloseHandle(m_ConnectHandle);
		m_ConnectHandle = NULL;

		if (GetLastError() == ERROR_INTERNET_TIMEOUT)
		{
			return ERROR_CURL_TIMEOUT;
		}
		return ERROR_HTTP_HOST;
	}

	InternetSetOptionA(m_RequestHandle, INTERNET_OPTION_CONNECT_TIMEOUT, &m_nTimeout, sizeof(DWORD));
	return ERROR_SUCCESS;
}

int CWinINetCore::closeRequest()
{
	if (m_RequestHandle)
	{
		HttpEndRequest(m_RequestHandle, NULL, 0, NULL);
		InternetCloseHandle(m_RequestHandle);
		m_RequestHandle = NULL;
	}

	if (m_ConnectHandle)
	{
		InternetCloseHandle(m_ConnectHandle);
		m_ConnectHandle = NULL;
	}

	return ERROR_SUCCESS;
}

int CWinINetCore::startRequest(std::string param)
{
	if (m_RequestHandle == NULL)
	{
		return ERROR_HTTP_HOST;
	}

	if (!HttpSendRequestA(m_RequestHandle, NULL, 0, (LPVOID)param.c_str(), param.length()))
	{
		if (GetLastError() == ERROR_INTERNET_TIMEOUT)
		{
			return ERROR_CURL_TIMEOUT;
		}
		return ERROR_HTTP_HOST;
	}

	DWORD dwRetCode = 0;
	DWORD dwSize = sizeof(DWORD);
	DWORD dwIndex = 0;
	HttpQueryInfoA(m_RequestHandle, HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER, &dwRetCode, &dwSize, &dwIndex);
	if (dwRetCode != HTTP_STATUS_OK)
	{
		if (GetLastError() == ERROR_INTERNET_TIMEOUT)
		{
			return ERROR_CURL_TIMEOUT;
		}
		return ERROR_HTTP_HOST;
	}

	INT64 contentLen = 0;
	char szContentLen[MAX_PATH] = {0};
	dwSize = MAX_PATH;
	if (HttpQueryInfoA(m_RequestHandle, HTTP_QUERY_CONTENT_LENGTH, szContentLen, &dwSize, NULL))
	{
		contentLen = _atoi64(szContentLen);
	}

	DWORD dwHeaderSize = 0;
	HttpQueryInfoA(m_RequestHandle, HTTP_QUERY_RAW_HEADERS, NULL, &dwHeaderSize, NULL);
	if (dwHeaderSize > 0)
	{
		char* pHeaderBuffer = new char[dwHeaderSize + 2];
		if (pHeaderBuffer)
		{
			memset(pHeaderBuffer, 0, dwHeaderSize + 2);
			HttpQueryInfoA(m_RequestHandle, HTTP_QUERY_RAW_HEADERS, pHeaderBuffer, &dwHeaderSize, NULL);

			std::string strHeader;
			parseHeaderResponse(pHeaderBuffer, dwHeaderSize, strHeader);

			onHeaderResponse(strHeader);

			delete[] pHeaderBuffer;
		}
	}

	char* pReadedBuffer = new char[MAX_READONCE_SIZE + 2];
	if (pReadedBuffer == NULL)
	{
		return ERROR_MEMORY_MALLOC;
	}
	memset(pReadedBuffer, 0, MAX_READONCE_SIZE + 2);

	int resCode = ERROR_SUCCESS;
	while (INetCore::m_gHttpContinue)
	{
		DWORD dwReadedSize = 0;
		bool res = InternetReadFile(m_RequestHandle, pReadedBuffer, MAX_READONCE_SIZE, &dwReadedSize);
		if (res == TRUE && dwReadedSize == 0)
		{
			break;
		}
		else if (res == FALSE)
		{
			resCode = ERROR_HTTP_HOST;
			break;
		}

		onBodyResponse(pReadedBuffer, dwReadedSize, contentLen);
	}

	delete[] pReadedBuffer;
	pReadedBuffer = NULL;
	return resCode;
	
}

int CWinINetCore::startSubmitPost(std::string fileBaseName, std::string keyName, __int64 lgFileSize)
{
	char szHeaderMark[1024] = {0};
	sprintf_s(szHeaderMark, 1024, SUBMIT_MARK_L, SUBMIT_BOUNDARY, keyName.c_str(), fileBaseName.c_str());

	char szTall[1024] = {0};
	sprintf_s(szTall, 1024, SUBMIT_TAIL_MARK_L, SUBMIT_BOUNDARY);

	char szHeaderLine[1024] = {0};
	sprintf_s(szHeaderLine, 1024, SUBMIT_HEADER_L, lgFileSize + strlen(szHeaderMark) + strlen(szTall), SUBMIT_BOUNDARY);

	INTERNET_BUFFERSA internetBuffer;
	memset(&internetBuffer, 0, sizeof(INTERNET_BUFFERSA));
	internetBuffer.dwStructSize = sizeof(INTERNET_BUFFERSA);
	internetBuffer.lpcszHeader = szHeaderLine;
	internetBuffer.dwHeadersLength = strlen(szHeaderLine);
	internetBuffer.lpvBuffer = szHeaderMark;
	internetBuffer.dwBufferLength = strlen(szHeaderMark);
	internetBuffer.dwBufferTotal = lgFileSize + strlen(szHeaderMark) + strlen(szTall);
	bool res = HttpSendRequestExA(m_RequestHandle, &internetBuffer, NULL, 0, NULL);
	if (!res)
	{
		if (GetLastError() == ERROR_INTERNET_TIMEOUT)
		{
			return ERROR_CURL_TIMEOUT;
		}
		return ERROR_HTTP_HOST;
	}

	char* lpWritenBuffer = new char[MAX_READONCE_SIZE];
	if (lpWritenBuffer == NULL)
	{
		return ERROR_MEMORY_MALLOC;
	}

	int		nResult = ERROR_SUCCESS;
	DWORD	dwReadedSize = 0;
	INT64	i64UploadPos = 0;
	while (INetCore::m_gHttpContinue)
	{
		dwReadedSize = 0;
		memset(lpWritenBuffer, 0, MAX_READONCE_SIZE);

		res = doReadRequest(lpWritenBuffer, MAX_READONCE_SIZE, &dwReadedSize);
		if (res == false)
		{
			nResult = ERROR_USER_ABORT;
			break;
		}
		if (dwReadedSize == 0)
		{
			break;
		}

		InternetWriteFile(m_RequestHandle, lpWritenBuffer, dwReadedSize, &dwReadedSize);

		i64UploadPos += dwReadedSize;
		onUploadProgress(i64UploadPos, lgFileSize);
	}

	InternetWriteFile(m_RequestHandle, (LPVOID)szTall, strlen(szTall), &dwReadedSize);

	delete[] lpWritenBuffer;
	return ERROR_SUCCESS;
}