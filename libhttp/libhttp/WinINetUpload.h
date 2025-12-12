#pragma once
#include "wininetcore.h"
#include "HTTPGlobal.h"

class CWinINetUpload :
	public CWinINetCore
{
public:
	CWinINetUpload(void);
	~CWinINetUpload(void);


public:
	int httpFileSubmit(std::string url, std::string param, std::string szfn, progressFn progressCallback, void* lpUser);

private:
	virtual void onHeaderResponse(std::string strHeader){}
	virtual void onBodyResponse(const char* pResponse, size_t nSize, INT64 i64Total){}

private:
	virtual bool doReadRequest(char* pBuffer, DWORD nSize, LPDWORD pdwReaded);

private:
	virtual void onUploadProgress(INT64 pos, INT64 total);

private:
	void* m_lpUserPointer;
	progressFn m_pfnProgressCallback;

private:
	HANDLE m_hPostSrcFile;

private:
	INT64	m_i64FileUploadPos;
	INT64	m_i64FileUploadTotal;
};
