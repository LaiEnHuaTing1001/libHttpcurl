#include "WinINetUpload.h"
#include "INetCore.h"

CWinINetUpload::CWinINetUpload(void)
{
	m_lpUserPointer = NULL;
	m_pfnProgressCallback = NULL;

	m_hPostSrcFile = INVALID_HANDLE_VALUE;
	m_i64FileUploadPos = 0;
	m_i64FileUploadTotal = 0;
}

CWinINetUpload::~CWinINetUpload(void)
{
	if (m_hPostSrcFile != NULL && m_hPostSrcFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hPostSrcFile);
		m_hPostSrcFile = INVALID_HANDLE_VALUE;
	}
}

bool CWinINetUpload::doReadRequest(char* pBuffer, DWORD nSize, LPDWORD pdwReaded)
{
	if (m_hPostSrcFile && m_hPostSrcFile != INVALID_HANDLE_VALUE)
	{
		return ReadFile(m_hPostSrcFile, pBuffer, nSize, pdwReaded, NULL);
	}
	return false;
}

void CWinINetUpload::onUploadProgress(INT64 pos, INT64 total)
{
	if (m_pfnProgressCallback)
	{
		m_pfnProgressCallback(pos, total, m_lpUserPointer);
	}
}

int CWinINetUpload::httpFileSubmit(std::string url, std::string param, std::string szfn, progressFn progressCallback, void* lpUser)
{
	writeLock wl(m_gRequestMutex);

	m_pfnProgressCallback = progressCallback;
	m_lpUserPointer = lpUser;

	if (m_InternelHandle == NULL)
	{
		return ERROR_WININET_LIB;
	}

	std::string fileBaseName;
	if (szfn.rfind("\\") == std::string::npos)
	{
		fileBaseName = szfn;
	}
	else
	{
		fileBaseName = szfn.substr(szfn.rfind("\\") + 1);
	}

	m_hPostSrcFile = CreateFileA(szfn.c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hPostSrcFile == INVALID_HANDLE_VALUE)
	{
		return ERROR_CREATE_FILE;
	}

	
	LARGE_INTEGER lgFileSize;
	GetFileSizeEx(m_hPostSrcFile, &lgFileSize);
	m_i64FileUploadTotal = lgFileSize.QuadPart;

	int requestSuc = openRequest(url, true);
	if (requestSuc != ERROR_SUCCESS)
	{
		CloseHandle(m_hPostSrcFile);
		m_hPostSrcFile = INVALID_HANDLE_VALUE;
		return requestSuc;
	}
	
	startSubmitPost(fileBaseName, "", lgFileSize.QuadPart);

	closeRequest();

	CloseHandle(m_hPostSrcFile);
	m_hPostSrcFile = INVALID_HANDLE_VALUE;
	return ERROR_SUCCESS;
}
