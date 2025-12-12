#include "WinINetDownloader.h"

CWinINetDownloader::CWinINetDownloader(void)
{
	m_lpUserPointer = NULL;
	m_lpProgressFn = NULL;

	m_i64DownloadTotal = 0;
	m_i64DownloadNow = 0;

	m_lpStoreFile = INVALID_HANDLE_VALUE;
}

CWinINetDownloader::~CWinINetDownloader(void)
{

}

void CWinINetDownloader::onHeaderResponse(std::string strHeader)
{
	
}

void CWinINetDownloader::onBodyResponse(const char* pResponse, size_t nSize, INT64 i64Total)
{
	if (m_i64DownloadTotal == 0)
	{
		m_i64DownloadTotal = i64Total;
	}

	if (m_lpStoreFile && (m_lpStoreFile != INVALID_HANDLE_VALUE))
	{
		DWORD dwWriten = 0;
		WriteFile(m_lpStoreFile, pResponse, nSize, &dwWriten, NULL);

		m_i64DownloadNow += dwWriten;
	}

	m_lpProgressFn(m_i64DownloadNow, m_i64DownloadTotal, m_lpUserPointer);
}

int CWinINetDownloader::httpDownload(std::string url, std::string param, std::string szfn, progressFn progressCallback, void*lpUser)
{
	writeLock wl(m_gRequestMutex);


	m_lpUserPointer = lpUser;
	m_lpProgressFn = progressCallback;

	if (m_InternelHandle == NULL)
	{
		return ERROR_WININET_LIB;
	}

	m_lpStoreFile = CreateFileA(szfn.c_str(), FILE_WRITE_DATA, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_lpStoreFile == INVALID_HANDLE_VALUE)
	{
		return ERROR_CREATE_FILE;
	}

	int requestSuc = openRequest(url, false);
	if (requestSuc != ERROR_SUCCESS)
	{
		CloseHandle(m_lpStoreFile);
		m_lpStoreFile = INVALID_HANDLE_VALUE;
		DeleteFileA(szfn.c_str());

		closeRequest();
		return requestSuc;
	}

	requestSuc = startRequest(param);

	CloseHandle(m_lpStoreFile);
	m_lpStoreFile = INVALID_HANDLE_VALUE;

	closeRequest();

	return requestSuc;

}