#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include "HTTPGlobal.h"
#include "httpcore.h"

class CHTTPRequest : public CHTTPCore {

public:
	CHTTPRequest(void);
	~CHTTPRequest(void);

private:
	std::string	m_strResponseContent;

private:
	std::string m_strHeaderContent;

private:
	void onHTTPResponseContent(const char* pData, int nSize);

private:
	int onHTTPHeaderContent(const char* pData, int nSize);

public:
	std::string getResponse();

public:
	std::string getHeader();

public:
	int httpRequest(std::string url, std::string param, bool isPost);
};

#endif