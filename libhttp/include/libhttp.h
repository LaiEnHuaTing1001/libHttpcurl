#ifndef __LIB_HTTP_H__
#define __LIB_HTTP_H__


#ifdef LIBHTTP_STATIC		/**< 以静态库的方式编译，如果外部使用此头文件时，也要定义此宏，表示以静态的方式引用 */
#define LIBHTTP_API
#elif LIBHTTP_EXPORTS
#define LIBHTTP_API __declspec(dllexport)
#else
#define LIBHTTP_API __declspec(dllimport)
#endif

#include "HTTPGlobal.h"
#include "IHTTPResponse.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

/**
/**< 错误码 
#define ERROR_SUCCESS			0L		/**< 执行成功 
#define ERROR_HTTP_HOST			-1L		/**< HTTP主机返回错误信息 
#define ERROR_CREATE_FILE		-2L		/**< 创建本地文件失败 
#define ERROR_REQUEST_PARAM		-3L		/**< 参数错误 
#define ERROR_CURL_LIB			-4L		/**< curl库出现异常 
#define ERROR_USER_ABORT		-5L		/**< 用户取消 
#define ERROR_CURL_TIMEOUT		-6L		/**< 超时 
#define ERROR_RESPONSE_EMPTY	-7L		/**< 服务器返回内容为空 
#define ERROR_WININET_LIB		-8L		/**< wininet返回错误 
#define ERROR_MEMORY_MALLOC		-9L		/**< 内存异常 
*/

/**
*	@fun			progressFn
*	@brief			下载进度回调类型
*  	@param
*			pos		当前进度
*			total	总大小，这个地方有可能为零，调用者使用时需要注意
*			lpUser	用户指针
*	@date    2020/12/02 16:05
*
**/
typedef void (__stdcall *progressFn)(INT64 pos, void* lpUser);

extern "C"
{
	LIBHTTP_API int LIBHTTPSetChannelInfo(const char* szIpv4, int nPortv4, const char* szIpv6, int nPortv6, bool isUsedHttps, int timeout);
	LIBHTTP_API bool LIBHTTPChannelUseHTTPS();
	LIBHTTP_API void LIBHTTPGetChannelIpInfo(char* szChnanelIp, int& nChannelPort);
	LIBHTTP_API void LIBHTTPSetLogListener(ILogListener* pLogListener);
	LIBHTTP_API int LIBHTTPGetAuthLocalIp(char* szLocalIp);
	LIBHTTP_API int LIBHTTPRequestLocalIp(const char* szUrl, char* szLocalIp);
	LIBHTTP_API void LIBHTTPReDetectChannelIp();
    LIBHTTP_API IHTTPResponse* LIBHTTPRequestWithoutToken(const char* szUrl, const char* szSendData, int method, int timeout, const char* szHeader);
	LIBHTTP_API IHTTPResponse* LIBHTTPRequest(const char* szUrl, const char* szSendData, bool isNeedAuth = true, int method = HTTP_METHOD_POST, int timeout = 0, const char* szHeader = NULL);
	LIBHTTP_API IHTTPResponse* LIBHTTPDownload(const char* szUrl, 
                                               const char* szSendData, 
                                               const char* szLocalFile, 
                                               bool isNeedAuth = true, 
                                               int method = HTTP_METHOD_GET, 
                                               int timeout = 0, 
                                               const char* szHeader = NULL);
    LIBHTTP_API IHTTPResponse* LIBHTTPDownload2(const char* szUrl,
                                               const char* szSendData,
                                               const char* szLocalFile,
                                               bool isNeedAuth = true,
                                               int method = HTTP_METHOD_GET,
                                               int timeout = 0,
                                               const char* szHeader = NULL,
                                               void* pProgressCallback = NULL,
                                               void* lpUser = NULL);
	LIBHTTP_API IHTTPResponse* LIBHTTPUpload(const char* szUrl, const char* szLocalFile, bool isNeedAuth = true, int method = HTTP_METHOD_POST, int timeout = 0, const char* szCustomHeader = NULL);


	LIBHTTP_API void LIBHTTPSetTranssionMissSpeeds(int downloadSpeedMax, int uploadSpeedMax, bool applyExistTrans);
    LIBHTTP_API void LIBHTTPSetTranssionMissSpeeds2(unsigned int downloadSpeedMax,
                                                    unsigned int uploadSpeedMax,
                                                    unsigned int& oldDownloadSpeedMax,
                                                    unsigned int& oldUploadSpeedMax);
    LIBHTTP_API void LIBHTTPGetTranssionMissSpeeds(unsigned int& oldDownloadSpeedMax,
                                                   unsigned int& oldUploadSpeedMax);
    LIBHTTP_API void LIBHTTPStopCurrentDownloadTask();
    LIBHTTP_API void LIBHTTPSetUUIDInfo(const char* pszUUID, const char* pszTenantId);
    LIBHTTP_API void LIBHTTPCheckAndSetUrl(char* pszUrl, int iBuffLen);
    LIBHTTP_API void LIBHTTPGetAuthirizeToken(char* pszToken, int iTokenLen);
    LIBHTTP_API void LIBHTTPSetProxy(const char* pstrProxyIp,
                                     const int iProxyPort,
                                     const char* pstrProxyUserName,
                                     const char* pstrProxyPassword);
}

#endif	//__LIB_HTTP_H__