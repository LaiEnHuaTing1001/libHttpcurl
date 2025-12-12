#ifndef __I_NET_CORE_H__
#define __I_NET_CORE_H__
#include <Windows.h>
#include "HTTPGlobal.h"

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
typedef void (__stdcall *progressFn)(INT64 pos, INT64 total, void* lpUser);


class INetCore {

public:
	static BOOL		m_gHttpContinue;	/**< 是否继续请求，当请求为false时，所有回调返回取消指令 */

protected:
	rwmutex			m_gRequestMutex;	/**< 访问锁，保证每一个对象同一时间只能产生依次请求 */

protected:
	int		m_nTimeout;

public:
	virtual void setTimeout(int timeout) = 0;
};
#endif
