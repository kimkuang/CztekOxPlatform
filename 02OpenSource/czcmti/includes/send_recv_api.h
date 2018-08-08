#pragma once
#include <stdint.h>
#include <string>
using namespace std;
extern "C" {
enum NETWORK_RPC_LOG_LEVEL
{
	LOG_NONE = 0,
	LOG_ERROR ,
	LOG_INFO,
	LOG_DEBUG
};

#ifdef EXPORT_DLL

#ifdef WIN32
#define DLL_PUBLIC __declspec(dllexport)
#else
#define DLL_PUBLIC  __attribute__ ((visibility("default")))
#endif

#else

#ifdef WIN32
#define DLL_PUBLIC __declspec(dllimport)
#else
#define DLL_PUBLIC  
#endif

#endif


#define MAX_PKG_LEN (150*1024*1024)



//注意：服务端使用的接口  不管请求包是否合法，都需要调sendRsp，因为涉及资源的释放
//初始化服务器端，程式运行时只需要初始化一次
DLL_PUBLIC int InitServer(NETWORK_RPC_LOG_LEVEL level, const char *szListenIp, uint16_t wListenPort);


//获取请求包，阻塞式的
/*
	pRsp:收到的包的首地址，业务层可以直接使用
	dwRspLen：请求包的长度
	ddwSessionId：该包的sessionID，唯一表示每个请求包，发响应时需要回填
*/
DLL_PUBLIC int GetReqBufAndLen(char *&pRsp, uint32_t & dwRspLen,  uint64_t &ddwSessionId);

//发送响应
/*
	pBuf:需要回给客户端的响应包buff首地址
	dwRspLen：响应包的长度
	ddwSessionId：包的sessionID，唯一表示每个请求包，发响应时回填获取时得到的sessionID
	//注意：服务端使用的接口  不管请求包是否合法，都需要调sendRsp，因为涉及资源的释放
*/
#if 0
DLL_PUBLIC int SendRsp(uint64_t ddwSessionId, char *pBuf, uint32_t dwRspLen);
#else
DLL_PUBLIC int SendResponse(uint64_t ddwSessionId, char *pBuf, uint32_t dwRspLen);
DLL_PUBLIC int FreeCurProcessPkg(uint64_t ddwSessionId);
#endif


//客户端使用的接口
//初始化客户端，程式运行时只需要初始化一次
DLL_PUBLIC int InitClient(NETWORK_RPC_LOG_LEVEL level);
//
/*发送请求
输入参数:
	pszReqBuf:请求包buff首地址
	dwReqLen：请求包的长度
	ddwSessionId：包的sessionID，唯一表示每个请求包，发响应时回填获取时得到的sessionID
	pszDstIP：服务器IP
	wDstPort：服务器端口号
	dwTimeOutMillSec：本次请求调用等待的超时时间
输出参数：
	pResult：结果缓冲区，，
输入输出参数：
	dwResultBufLen：结果缓冲区的长度，调用时，填结果缓冲区pResult长度，如果调用成功，会修改为调用结果的长度，注意，会被修改！！！
*/
DLL_PUBLIC int RpcSendReq(char *pszReqBuf, uint32_t dwReqLen,  char *pszDstIP, uint16_t wDstPort, uint32_t dwTimeOutMillSec, char*pResult, uint32_t &dwResultBufLen);

DLL_PUBLIC void Destory();
}