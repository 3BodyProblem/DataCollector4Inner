#ifndef __CTP_SESSION_H__
#define __CTP_SESSION_H__


#pragma warning(disable:4786)
#include <set>
#include <string>
#include <stdexcept>
#include "../Configuration.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/DateTime.h"


#pragma pack(1)


/**
 * @class							tagPackageHead
 * @brief							数据包的包头结构定义
 * @author							barry
 */
typedef struct
{
	unsigned __int64				nSeqNo;				///< 自增序号
	unsigned int					nMarketID;			///< 市场编号
	unsigned int					nBodyLen;			///< 数据部分长度
	unsigned int					nMsgCount;			///< 包内的Message数量
} tagPackageHead;


/**
 * @class							tagBlockHead
 * @brief							数据块的头部的定义
 * @author							barry
 */
typedef struct
{
	unsigned int					nDataType;			///< 数据块类型
	unsigned int					nDataLen;			///< 数据块长度
} tagBlockHead;


#pragma pack()


/**
 * @class			CTPWorkStatus
 * @brief			CTP工作状态管理
 * @author			barry
 */
class CTPWorkStatus
{
public:
	/**
	 * @brief				应状态值映射成状态字符串
	 */
	static	std::string&	CastStatusStr( enum E_SS_Status eStatus );

public:
	/**
	 * @brief			构造
	 * @param			eMkID			市场编号
	 */
	CTPWorkStatus();
	CTPWorkStatus( const CTPWorkStatus& refStatus );

	/**
	 * @brief			赋值重载
						每次值变化，将记录日志
	 */
	CTPWorkStatus&		operator= ( enum E_SS_Status eWorkStatus );

	/**
	 * @brief			重载转换符
	 */
	operator			enum E_SS_Status();

private:
	CriticalObject		m_oLock;				///< 临界区对象
	enum E_SS_Status	m_eWorkStatus;			///< 行情工作状态
};


/**
 * @class			MkQuotation
 * @brief			CTP会话管理对象
 * @detail			封装了针对商品期货期权各市场的初始化、管理控制等方面的方法
 * @author			barry
 */
class MkQuotation : public MBPClientCommIO_Spi
{
public:
	MkQuotation();
	~MkQuotation();

	/**
	 * @brief				初始化ctp行情接口
	 * @return				>=0			成功
							<0			错误
	 * @note				整个对象的生命过程中，只会启动时真实的调用一次
	 */
	int						Activate() throw(std::runtime_error);

	/**
	 * @brief				释放ctp行情接口
	 */
	int						Destroy();

	/**
	 * @brief				暂停行情采集
	 */
	int						Halt();

	/**
	 * @brief				获取会话状态信息
	 */
	CTPWorkStatus&			GetWorkStatus();

	/**
	 * @brief				获取市场编号
	 */
	unsigned int			GetMarketID() const;

protected:///< CThostFtdcMdSpi的回调接口
	virtual void			OnConnectSuc();	//连接成功消息响应函数
	virtual void			OnConnectFal();	//连接失败消息响应函数
	virtual void			OnDisconnect();	//连接断开消息响应函数
	virtual void			OnDestory();	//关联API被销毁

	/**
	 * @brief				收到数据消息响应函数
	 * @param[in]			usMessageNo				消息ID
	 * @param[in]			usFunctionID			功能号
	 * @param[in]			bErrorFlag				错误标识
	 * @param[in]			lpData					收到的数据缓存地址
	 * @param[in]			uiSize					收到的数据大小
	 * @return				返回false表示处理数据错误，断开连接
	 */
	virtual bool			OnRecvData( unsigned short usMessageNo, unsigned short usFunctionID, bool bErrorFlag, const char* lpData, unsigned int uiSize );

protected:
	/** 
	 * @brief				刷数据到发送缓存
 	 * @param[in]			usMessageNo				消息ID
	 * @param[in]			usFunctionID			功能号
	 * @param[in]			lpData					收到的数据缓存地址
	 * @param[in]			uiSize					收到的数据大小
	 * @return				返回false表示处理数据错误，断开连接
	 */
	bool					OnQuotation( unsigned short usMessageNo, unsigned short usFunctionID, const char* lpData, unsigned int uiSize );

	/**
	 * @brief				根据新的码表重新订阅行情
	 * @return				>=0			成功
	 */
	int						PrepareDumpFile();

private:
	MBPClientCommIO_Api*	m_pCommIOApi;			///< 通讯管理器指针
	MBPClientCommIO			m_oCommIO;				///< 通讯管理插件

private:
	unsigned int			m_nMarketID;			///< 市场编号
	CTPWorkStatus			m_oWorkStatus;			///< 工作状态
};




#endif






