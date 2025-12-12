#pragma once

#ifndef __CLOG_PRINT_H__
#define __CLOG_PRINT_H__

#include "HTTPGlobal.h"
#include "IHTTPResponse.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define HTTP_LOG_ERROR		3
#define HTTP_LOG_WARN		4
#define HTTP_LOG_DEBUG		7

#define SINGLE_LOG_LENGTH	2048
class CLogPrint: public boost::serialization::singleton<CLogPrint>
{
protected:
	CLogPrint();
public:
	void setLogCallback(ILogListener* pListener);

public:
	void printf(int level, const char* format, ...);

protected:
	ILogListener* m_pLogListener;

protected:
	char* m_pLogBuffer;
protected:
	httpMutex	m_mtxCallback;
};

#define LOGD(fmt, ...) CLogPrint::get_mutable_instance().printf(HTTP_LOG_DEBUG, fmt, __VA_ARGS__)
#define LOGW(fmt, ...) CLogPrint::get_mutable_instance().printf(HTTP_LOG_WARN, fmt, __VA_ARGS__)
#define LOGE(fmt, ...) CLogPrint::get_mutable_instance().printf(HTTP_LOG_ERROR, fmt, __VA_ARGS__)

#endif

