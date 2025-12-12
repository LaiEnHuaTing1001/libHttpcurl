#include "WinINetRequest.h"

CWinINetRequest::CWinINetRequest(void)
{
}

CWinINetRequest::~CWinINetRequest(void)
{
}

std::string CWinINetRequest::getHeader()
{
	return m_ResponseHeader;
}

std::string CWinINetRequest::getResponse()
{
	return m_ResponseBody;
}

void CWinINetRequest::onHeaderResponse(std::string strHeader)
{
	m_ResponseHeader = strHeader;
}

void CWinINetRequest::onBodyResponse(const char* pResponse, size_t nSize, INT64 i64Total)
{
	m_ContentLength = i64Total;
	m_ResponseBody.append(pResponse);
}


int CWinINetRequest::httpRequest(std::string url, std::string param, bool isPost)
{
	writeLock rw(m_gRequestMutex);

	int requestSuc = openRequest(url, isPost);
	if (requestSuc != ERROR_SUCCESS)
	{
		return requestSuc;
	}

	requestSuc = startRequest(param);


	closeRequest();
	return requestSuc;

}