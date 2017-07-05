#ifndef __CTP_SESSION_H__
#define __CTP_SESSION_H__


#pragma warning(disable:4786)
#include <set>
#include <string>
#include <stdexcept>
#include "DataDump.h"
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
	unsigned char					nMarketID;			///< 市场编号
	unsigned short					nMsgLength;			///< 数据部分长度
	unsigned short					nMsgCount;			///< Message数量
} tagPackageHead;


/**
 * @class							tagLoginData
 * @brief							登录数据块
 * @author							barry
 */
typedef struct
{
	char							pszActionKey[20];	///< 指令字符串: login:请求登录 success:登录成功 failure:登录失败
	char							pszUserName[32];	///< 用户名
	char							pszPassword[64];	///< 密码
	unsigned int					nReqDBSerialNo;		///< 请求多少流水号之后的增量数据
	char							Reserve[1024];		///< 保留
} tagCommonLoginData_LF299;


#pragma pack()


/**
 * @class			WorkStatus
 * @brief			工作状态管理
 * @author			barry
 */
class WorkStatus
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
	WorkStatus();
	WorkStatus( const WorkStatus& refStatus );

	/**
	 * @brief			赋值重载
						每次值变化，将记录日志
	 */
	WorkStatus&			operator= ( enum E_SS_Status eWorkStatus );

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
 * @brief			会话管理对象
 * @detail			封装了针对商品期货期权各市场的初始化、管理控制等方面的方法
 * @author			barry
 */
class MkQuotation : public MBPClientCommIO_Spi
{
public:
	MkQuotation();
	~MkQuotation();

public:///< 构建接口
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

public:///< 行情获取接口
	/**
	 * @brief				重新连接请求行情快照和推送
	 * @return				==0							成功
							!=0							出错
	 */
	int						Connect2Server();

	/**
	 * @brief				断开连接
	 */
	void					CloseConnection();

public:///< 功能函数
	/**
	 * @brief				获取会话状态信息
	 */
	WorkStatus&				GetWorkStatus();

	/**
	 * @brief				获取市场编号
	 */
	unsigned int			GetMarketID() const;

	/**
	 * @brief				获取端口封装对象
	 */
	MBPClientCommIO&		GetCommIO();

protected:///< CThostFtdcMdSpi的回调接口
	/**
	 * @brief				连接成功消息回调
	 */
	virtual void			OnConnectSuc();

	/**
	 * @brief				连接失败消息回调
	 */
	virtual void			OnConnectFal();

	/**
	 * @brief				连接断开消息回调
	 */
	virtual void			OnDisconnect();

	/**
	 * @brief				关联API被销毁消息回调
	 */
	virtual void			OnDestory();

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

protected:///< 收到的行情数据处理方法
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
	 * @return				>=0						成功
	 */
	int						PrepareDumpFile();

	/**
	 * @brief				发送登录行情服务数据包
	 * @return				true					发送登录成功
	 */
	bool					SendLoginPkg();

private:
	MBPClientCommIO_Api*	m_pCommIOApi;			///< 通讯管理器指针
	MBPClientCommIO			m_oCommIO;				///< 通讯管理插件

private:
	unsigned int			m_nMarketID;			///< 市场编号
	WorkStatus				m_oWorkStatus;			///< 工作状态
	MemoDumper<char>		m_oQuotDumper;			///< 行情落盘对象
};




#endif





