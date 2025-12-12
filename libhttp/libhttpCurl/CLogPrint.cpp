#include "CLogPrint.h"

CLogPrint::CLogPrint()
{
	m_pLogListener = NULL;
	m_pLogBuffer = NULL;
}

void CLogPrint::setLogCallback(ILogListener* pListener)
{
	httpWriteLock wl(m_mtxCallback);

	m_pLogListener = pListener;
}

void CLogPrint::printf(int level, const char* format, ...)
{
	httpWriteLock wl(m_mtxCallback);

	if (m_pLogListener == NULL || format == NULL)
	{
		return;
	}

	if (m_pLogBuffer == NULL)
	{
		m_pLogBuffer = new(std::nothrow) char[SINGLE_LOG_LENGTH];
		if (m_pLogBuffer == NULL)
		{
			return;
		}
	}

	ZeroMemory(m_pLogBuffer, SINGLE_LOG_LENGTH);

	sprintf_s(m_pLogBuffer, SINGLE_LOG_LENGTH, "%d|%d> ", GetCurrentThreadId(), level);

	int nLogHeaderLen = strlen(m_pLogBuffer);
	char* pCostomBuffer = m_pLogBuffer + nLogHeaderLen;
	va_list vlArgs;
	va_start(vlArgs, format);
	_vsnprintf_s(pCostomBuffer, SINGLE_LOG_LENGTH - nLogHeaderLen - 1, _TRUNCATE, format, vlArgs);
	va_end(vlArgs);

	m_pLogListener->onLog(level, m_pLogBuffer);
	OutputDebugStringA(m_pLogBuffer);
}