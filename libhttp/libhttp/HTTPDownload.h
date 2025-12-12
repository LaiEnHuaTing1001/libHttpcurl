#ifndef __HTTP_DOWNLOAD_H__
#define __HTTP_DOWNLOAD_H__

#include "httpcore.h"

class CHTTPDownload : public CHTTPCore {

public:
	CHTTPDownload(void);
	~CHTTPDownload(void);

private:
	void*	m_lpUserPointer;

private:
	progressFn	m_lpProgressFn;

private:
	HANDLE	m_lpStoreFile;

private:
	INT64	m_i64DownloadTotal;
	INT64	m_i64DownloadNow;

private:
	int onHTTPDownloadProgress(INT64 iNow, INT64 iTotal);

private:
	void onHTTPResponseContent(const char* pData, int nSize);

public:
	int httpDownload(std::string url, std::string param, std::string szfn, progressFn progressCallback, void* lpUser);
};

#endif	//__HTTP_DOWNLOAD_H__