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
 * @brief						行情数据采集模块主干类
 * @date						2017/5/15
 * @author						barry
 */
class QuoCollector : public SimpleTask
{
protected:
	QuoCollector();

public:
	/**
	 * @brief					获取数据采集对象的单键引用
	 */
	static QuoCollector&		GetCollector();

	/**
	 * @brief					初始化数据采集器
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
	 * @brief					重载返回行情回调接口地址
	 * @return					行情回调接口指针地址
	 */
	I_DataHandle*				operator->();

	/**
	 * @brief					从本地文件或行情端口重新恢复加载所有行情数据
	 * @return					==0							成功
								!=0							出错
	 */
	int							RecoverQuotation();

	/**
	 * @brief					取得采集模块的当前状态
	 */
	enum E_SS_Status			GetCollectorStatus();

	/**
	 * @brief					停止
	 */
	void						Halt();

	/**
	 * @brief					获取市场编号
	 */
	unsigned int				GetMarketID() const;

protected:
	/**
	 * @brief					任务函数(内循环)
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
	 * @brief								获取模块的当前状态
	 */
	__declspec(dllexport) int __stdcall		GetStatus();

	/**
	 * @brief								获取市场编号
	 */
	__declspec(dllexport) int __stdcall		GetMarketID();

	/**
	 * @brief								单元测试导出函数
	 */
	__declspec(dllexport) void __stdcall	ExecuteUnitTest();
}




#endif





