#ifndef __HTTP_DOWNLOAD_H__
#define __HTTP_DOWNLOAD_H__

#include "httpcore.h"
#include "libhttp.h"

class CHTTPDownload : public CHTTPCore {

public:
	CHTTPDownload(void);
	~CHTTPDownload(void);

public:
    static CURL* m_pUrlDownloadHandle;

protected:
	CURL* getUrlResHandle();

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
	__time32_t m_tmLastDownloadTranslate;

private:
	void onHTTPResponseContent(const char* pData, int nSize);

protected:
	int onHTTPDownloadProgress(double dlTotal, double dlNow, double ulTotal, double ulNow);

public:
	int httpDownload(std::string url, std::string param, std::string strfn, progressFn progressCallback, void* lpUser);

protected:
	static int HTTPProgressCallback(void* userp, double dlTotle, double dlNow, double ulTotal, double ulNow);
};

#endif	//__HTTP_DOWNLOAD_H__