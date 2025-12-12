#ifndef __HTTP_UPLOAD_H__
#define __HTTP_UPLOAD_H__

#include "httpcore.h"
#include "libhttp.h"

class CHTTPUpload : public CHTTPCore {

public:
	CHTTPUpload(void);
	~CHTTPUpload(void);

public:
	static CURL* m_pUrlUploadHandle;

protected:
	CURL* getUrlResHandle();

public:
	int httpFilePost(std::string url, std::string strfn, progressFn progressCallback, void* lpUser);

public:
	std::string getResponse();

private:
	progressFn	m_pfnProgressCallback;

private:
	void*		m_lpUserPointer;

private:
	HANDLE		m_hPostSrcFile;

private:
	std::string	m_strResponseContent;

private:
	void onHTTPResponseContent(const char* pData, int nSize);

private:
	int onHTTPReadContent(char* pData, int size);
};

#endif	//__HTTP_UPLOAD_H__