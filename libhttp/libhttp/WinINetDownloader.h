#pragma once
#include "wininetcore.h"
#include "HTTPGlobal.h"

class CWinINetDownloader :
	public CWinINetCore
{
public:
	CWinINetDownloader(void);
	~CWinINetDownloader(void);

public:
	int httpDownload(std::string url, std::string param, std::string szfn, progressFn progressCallback, void*lpUser);

private:
	virtual void onHeaderResponse(std::string strHeader);
	virtual void onBodyResponse(const char* pResponse, size_t nSize, INT64 i64Total);

private:
	void*		m_lpUserPointer;

private:
	progressFn	m_lpProgressFn;

private:
	UINT64		m_i64DownloadTotal;
	UINT64		m_i64DownloadNow;

private:
	HANDLE		m_lpStoreFile;
};
