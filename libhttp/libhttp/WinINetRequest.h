#pragma once
#include "wininetcore.h"

class CWinINetRequest :
	public CWinINetCore
{
public:
	CWinINetRequest(void);
	~CWinINetRequest(void);

public:
	std::string getResponse();
	std::string getHeader();

public:
	int httpRequest(std::string url, std::string param, bool isPost);

private:
	virtual void onHeaderResponse(std::string strHeader);
	virtual void onBodyResponse(const char* pResponse, size_t nSize, INT64 i64Total);

private:
	INT64		m_ContentLength;
	std::string m_ResponseBody;
	std::string m_ResponseHeader;
};
