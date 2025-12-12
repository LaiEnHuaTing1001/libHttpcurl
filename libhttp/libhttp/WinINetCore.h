#pragma once
#include <Windows.h>
#include <WinInet.h>
#include "HTTPGlobal.h"
#include "INetCore.h"

#pragma comment(lib, "Wininet.lib")

#define MAX_READONCE_SIZE 4096

class CWinINetCore : public INetCore
{
public:
	CWinINetCore(void);
	~CWinINetCore(void);

protected:
	HINTERNET	m_InternelHandle;
	HINTERNET	m_ConnectHandle;
	HINTERNET	m_RequestHandle;

private:
	int m_nPort;
	std::string m_strHost;
	std::string m_strScheme;
	std::string m_strPath;

private:
	virtual void onHeaderResponse(std::string strHeader) = 0;

private:
	virtual void onBodyResponse(const char* pResponse, size_t nSize, INT64 i64Total) = 0;

private:
	virtual bool doReadRequest(char* pBuffer, DWORD nSize, LPDWORD pdwReaded);

private:
	virtual void onUploadProgress(INT64 pos, INT64 total);


protected:
	void parseHeaderResponse(char* pHeader, unsigned int uSize, std::string& strHeader);

protected:
	void crackUrl(std::string& url, std::string& scheme, std::string& host, int& port, std::string& objName);

public:
	void setTimeout(int timeout);

protected:
	int openRequest(std::string url, bool isPost);

protected:
	int closeRequest();

protected:
	int startRequest(std::string param);

protected:
	int startSubmitPost(std::string fileBaseName, std::string keyName, __int64 lgFileSize);

};
