#include "CHTTPResponse.h"

CHTTPResponse::CHTTPResponse(int respCode, std::string resp, std::string localip)
{
	this->m_nResponseCode = respCode;
	this->m_strLocalIp = localip;
	this->m_strResponse = resp;
}

int CHTTPResponse::getResponseCode()
{
	return this->m_nResponseCode;
}

const char* CHTTPResponse::getResponse()
{
	return this->m_strResponse.c_str();
}

const char* CHTTPResponse::getLocalIp()
{
	return this->m_strLocalIp.c_str();
}


void CHTTPResponse::Release()
{
	delete this;
}

IHTTPResponse* CHTTPResponse::createResponse(int respCode, std::string response, std::string localIp)
{
	return new CHTTPResponse(respCode, response, localIp);
}