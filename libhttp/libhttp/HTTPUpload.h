#ifndef __HTTP_UPLOAD_H__
#define __HTTP_UPLOAD_H__

#include "httpcore.h"

class CHTTPUpload : public CHTTPCore {

public:
	CHTTPUpload(void);
	~CHTTPUpload(void);

public:
	int httpFileSubmit(std::string url, std::string param, std::string szfn, progressFn progressCallback, void* lpUser);

public:
	int httpFilePost(std::string url, std::string param, std::string szfn, progressFn progressCallback, void* lpUser);

private:
	progressFn	m_pfnProgressCallback;

private:
	void*		m_lpUserPointer;

private:
	HANDLE		m_hPostSrcFile;

private:
	int onHTTPUploadProgress(INT64 iNow, INT64 iTotal);

private:
	int onHTTPReadContent(char* pData, int size);
};

#endif	//__HTTP_UPLOAD_H__