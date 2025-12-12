#pragma once
#ifndef __C_HTTP_RESPONSE_H__
#define __C_HTTP_RESPONSE_H__

#include "IHTTPResponse.h"

#include <string>
class CHTTPResponse : public IHTTPResponse
{
private:
	CHTTPResponse(int respCode, std::string resp, std::string localip);

public:
	int getResponseCode() override;

public:
	const char* getResponse() override;

public:
	const char* getLocalIp() override;

public:
	void Release() override;

private:
	int				m_nResponseCode;
	std::string		m_strResponse;
	std::string		m_strLocalIp;


public:
	static IHTTPResponse* createResponse(int respCode, std::string response = std::string(), std::string localIp = std::string());
};


#endif