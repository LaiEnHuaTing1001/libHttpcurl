// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "HTTPGlobal.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			curl_global_init(CURL_GLOBAL_ALL);
			break;
		}
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			curl_global_cleanup();
			break;
		}
	}
	return TRUE;
}

