#ifndef __HTTP_GLOBAL_H__
#define __HTTP_GLOBAL_H__

#include <stdio.h>
#include <Windows.h>
#include <string>
#include <map>

#include "boost/thread/thread.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/function.hpp"
#include <boost/bind/bind.hpp>
#include <boost/serialization/singleton.hpp>


typedef boost::shared_mutex httpMutex; 
typedef boost::shared_lock<httpMutex> httpReadLock;
typedef boost::unique_lock<httpMutex> httpWriteLock;


#define ERROR_SUCCESS			0L		/**< 执行成功 */
#define ERROR_HTTP_HOST			-1L		/**< HTTP主机返回错误信息 */
#define ERROR_CREATE_FILE		-2L		/**< 创建本地文件失败 */
#define ERROR_REQUEST_PARAM		-3L		/**< 参数错误 */
#define ERROR_CURL_LIB			-4L		/**< curl库出现异常 */
#define ERROR_USER_ABORT		-5L		/**< 用户取消 */
#define ERROR_CURL_TIMEOUT		-6L		/**< 超时 */
#define ERROR_RESPONSE_EMPTY	-7L		/**< 服务器返回内容为空 */
#define ERROR_WININET_LIB		-8L		/**< wininet返回错误 */
#define ERROR_MEMORY_MALLOC		-9L		/**< 内存异常 */
#define ERROR_HOST_UNRECOGNIZE		-10L	/**< 域名不可识别(IP探测使用) */
#define ERROR_SOCKET_INVALID	-11L	/**< SOCK出现异常 */

#define HTTP_AUTHORIZE_FAIL_CODE 401

#define HTTP_REQUEST_GET		0		/**< 使用GET的方式传参 */
#define HTTP_REQUEST_POST		1		/**< 使用POST的方式传参 */

#define HTTP_TIME_OUT_MS		(30 * 1000)

#define USER_ANGENT			"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.67 Safari/537.36 Edg/87.0.664.52"

#define AUTHORIZE_HEADER	"token"

typedef boost::function<void(int, const char*)> FuncHTTPLogCallback;

#endif	//__HTTP_GLOBAL_H__
