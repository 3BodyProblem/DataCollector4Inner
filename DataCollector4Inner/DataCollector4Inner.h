#ifndef __DATA_TABLE_PAINTER_H__
#define	__DATA_TABLE_PAINTER_H__


#include "Interface.h"
#include "Configuration.h"
#include "Infrastructure/Lock.h"
#include "Connection/Quotation.h"
#include "Infrastructure/Thread.h"


/**
 * @class						T_LOG_LEVEL
 * @brief						日志类型
 * @author						barry
 */
enum T_LOG_LEVEL
{
	TLV_INFO = 0,
	TLV_WARN = 1,
	TLV_ERROR = 2,
	TLV_DETAIL = 3,
};


/**
 * @class						QuoCollector
 * @brief						自适合行情数据采集模块主干类
 * @detail						本模块适合于任何市场的行情数据结构;
								&
								作为传输模块将保持任何时间的连接状态（除了初始化过程时段）
 * @date						2017/5/15
 * @author						barry
 */
class QuoCollector : public SimpleTask
{
protected:
	QuoCollector();

public:
	/**
	 * @brief					初始化数据采集器
	 * @detail					加载配置 + 设置消息回调 + 激活对上的通讯模块 + 启动断开重连线程
	 * @param[in]				pIDataHandle				行情回调接口
	 * @return					==0							初始化成功
								!=0							出错
	 */
	int							Initialize( I_DataHandle* pIDataHandle );

	/**
	 * @brief					释放资源
	 */
	void						Release();

public:
	/**
	 * @brief					重新连接请求行情快照和推送
	 * @return					==0							成功
								!=0							出错
	 */
	int							RecoverQuotation();

	/**
	 * @brief					暂时停止自动自动重连功能
	 */
	void						Halt();

public:
	/**
	 * @brief					取得采集模块的当前状态
 	 * @param[out]				pszStatusDesc				返回出状态描述串
	 * @param[in,out]			nStrLen						输入描述串缓存长度，输出描述串有效内容长度
	 * @return					返回模块当前状态值
	 */
	enum E_SS_Status			GetCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief					获取市场编号
	 */
	unsigned int				GetMarketID() const;

	/**
	 * @brief					获取数据采集对象的单键引用
	 */
	static QuoCollector&		GetCollector();

	/**
	 * @brief					重载返回行情回调接口地址
	 * @return					行情回调接口指针地址
	 */
	I_DataHandle*				operator->();

protected:
	/**
	 * @brief					任务函数(内循环，自动断开重连上级行情源)
	 * @return					==0					成功
								!=0					失败
	 */
	virtual int					Execute();

protected:
	I_DataHandle*				m_pCbDataHandle;			///< 数据(行情/日志回调接口)
	MkQuotation					m_oQuotationData;			///< 实时行情数据会话对象
};





/**
 * @brief						DLL导出接口
 * @author						barry
 * @date						2017/4/1
 */
extern "C"
{
	/**
	 * @brief								初始化数据采集模块
	 */
	__declspec(dllexport) int __stdcall		Initialize( I_DataHandle* pIDataHandle );

	/**
	 * @brief								释放数据采集模块
	 */
	__declspec(dllexport) void __stdcall	Release();

	/**
	 * @brief								重新初始化并加载行情数据
	 */
	__declspec(dllexport) int __stdcall		RecoverQuotation();

	/**
	 * @brief								暂时数据采集
	 */
	__declspec(dllexport) void __stdcall	HaltQuotation();

	/**
	 * @brief								获取模块的当前状态
	 * @param[out]							pszStatusDesc				返回出状态描述串
	 * @param[in,out]						nStrLen						输入描述串缓存长度，输出描述串有效内容长度
	 * @return								返回模块当前状态值
	 */
	__declspec(dllexport) int __stdcall		GetStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief								获取市场编号
	 */
	__declspec(dllexport) int __stdcall		GetMarketID();

	/**
	 * @brief								是否为行情传输的采集器
	 */
	__declspec(dllexport) bool __stdcall	IsProxy();

	/**
	 * @brief								单元测试导出函数
	 */
	__declspec(dllexport) void __stdcall	ExecuteUnitTest();
}




#endif





