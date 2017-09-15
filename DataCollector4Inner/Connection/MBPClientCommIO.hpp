//-----------------------------------------------------------------------------------------------------------------------------
//                `-.   \    .-'
//        ,-"`````""-\__ |  /							文件名称：MBPClientCommIO
//         '-.._    _.-'` '-o,							文件描述：MBP客户端通讯接口
//             _>--:{{<   ) |)							文件编写：Lumy
//         .-''      '-.__.-o`							创建日期：2016.10.18
//        '-._____..-/`  |  \							更新日期：2016.10.18
//                ,-'   /    `-.
//-----------------------------------------------------------------------------------------------------------------------------
#ifndef __MBPClientCommIO_HPP__
#define __MBPClientCommIO_HPP__
//-----------------------------------------------------------------------------------------------------------------------------
#include <winsock2.h>
//-----------------------------------------------------------------------------------------------------------------------------
typedef void tagFun_OnError(const char * szErrString);
//-----------------------------------------------------------------------------------------------------------------------------
typedef struct
{
	unsigned int						uiMaxLinkCount;			//支持的最大连接数量【仅异步方式使用】
	unsigned int						uiSendBufCount;			//发送资源数量【仅异步方式使用】
	unsigned int						uiThreadCount;			//线程数量【仅异步方式使用】
	unsigned int						uiPageSize;				//内存池页面大小【仅异步方式使用】
	unsigned int						uiPageCount;			//内存池页面数量【仅异步方式使用】
	bool								bSSL;					//是否启用SSL加密
	char								szCrtFileName[256];		//OpenSSL .crt证书文件路径，仅SSL使用
	char								szPfxFileName[256];		//OpenSSL .pfx证书文件路径，仅SSL使用
	char								szPfxFilePasswrod[32];	//OpenSSL .pfx证书密码，仅SSL使用
	bool								bDetailLog;				//是否打印详细日志
	tagFun_OnError					*	lpOnError;				//错误响应指针
	unsigned char						ucSSLType;				//SSL协议版本（0:不使用SSL 1:SSL3.0/2.0兼容协议 2:TLS1.0 3:TLS1.1 其他:TLS1.2）
	char								szCiperList[2048];		//SSL使用的密码套件（如"ECDHE-RSA-AES128-GCM-SHA256"、"AES256-SHA"）
	char								szReserved[2047];		//保留
} tagMBPClientIO_RunParam;
//.............................................................................................................................
typedef struct
{
	unsigned int						uiMaxLinkCount;			//最大链接数量
	unsigned int						uiConnectCount;			//正在连接数量
	unsigned int						uiCurLinkCount;			//当前连接数量
	unsigned int						uiSendBufPercent;		//发送缓冲占用百分比【百分比】
	unsigned int						uiMemoryPoolPercent;	//内存池占用百分比【百分比】
	unsigned int						uiRecvFreq;				//接收频率【次/秒】
	unsigned int						uiSendFreq;				//发送频率【次/秒】
	unsigned int						uiSendLostFreq;			//发送丢失频率【次/秒】
	unsigned int						uiRecvBandWidth;		//接收带宽【bps】
	unsigned int						uiSendBandWidth;		//发送带宽【bps】
	unsigned __int64					ui64RecvAmount;			//接收总量【字节】
	unsigned __int64					ui64SendAmount;			//发送总量【字节】
	double								dCompressRate;			//压缩率【百分比】
	unsigned int						uiWorkThreadCount;		//工作线程数量
} tagMBPClientIO_Status;
//-----------------------------------------------------------------------------------------------------------------------------
//异步通讯响应函数SPI
class MBPClientCommIO_Spi
{
public:
	//连接成功消息响应函数
	virtual void OnConnectSuc(void) = 0;
	//连接失败消息响应函数
	virtual void OnConnectFal(void) = 0;
	//连接断开消息响应函数
	virtual void OnDisconnect(void) = 0;
	//收到数据消息响应函数，返回false表示处理数据错误，断开连接
	virtual bool OnRecvData(unsigned short usMessageNo,unsigned short usFunctionID,bool bErrorFlag,const char * lpData,unsigned int uiSize) = 0;
	//关联API被销毁
	virtual void OnDestory(void) = 0;
};
//-----------------------------------------------------------------------------------------------------------------------------
//异步通讯API
class MBPClientCommIO_Api
{
public:
	//注册消息响应函数
	virtual void RegisterSpi(MBPClientCommIO_Spi * lpSpi) = 0;
	//连接服务器
	virtual int  Connect(const char * szIPAddr,unsigned int uiPort) = 0;
	//断开连接
	virtual void Disconnect(void) = 0;
	//发送数据
	virtual int  SendData(unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize,bool bCompress) = 0;
	//发送错误
	virtual int  SendError(unsigned short usMessageNo,unsigned short usFunctionID,const char * lpErrorInfo) = 0;
	//删除接口对象本身
	virtual void Release(void) = 0;
	//获取SOCKET编号
	virtual SOCKET GetSocket(void) = 0;
};
//-----------------------------------------------------------------------------------------------------------------------------
//同步通讯通讯函数
class MBPSyncClientIO
{
public:
	//参数：IP地址或域名，服务端口，连接超时时间（毫秒）
	virtual int  Connect(const char * szIPAddr,unsigned int uiPort,unsigned int uiTimeOut) = 0;
	//断开与服务器连接
	virtual void Disconnect(void) = 0;
	//发送数据
	//参数：功能号，发送数据缓冲，发送数据长度，是否需要压缩
	virtual int  SendData(unsigned short usMessageNo,unsigned short usFunctionID,const char * lpData,unsigned int uiSize,bool bCompress) = 0;
	//发送错误
	virtual int  SendError(unsigned short usMessageNo,unsigned short usFunctionID,const char * lpErrorInfo) = 0;
	//接收数据
	//参数：接收到数据的功能号，接收数据缓冲，接收数据缓冲大小
	virtual int  RecvData(unsigned short * lpMessageNo,unsigned short * lpFunctionID,bool * lpErrorFlag,char * lpOutBuf,unsigned int uiOutSize) = 0;
	//释放
	virtual void Release(void) = 0;
	//获取SOCKET编号
	virtual SOCKET GetSocket(void) = 0;
};
//-----------------------------------------------------------------------------------------------------------------------------
//动态链接库接口输出函数
//int  StartWork(const tagMBPClientIO_RunParam * lpParam);
//void EndWork(void);
//MBPClientCommIO_Api * CreateMBPClientCommIO_Api(void);
//MBPSyncClientIO * CreateMBPSyncClientIO(void);
//void GetStatus(tagMBPClientIO_Status * lpOut)
//.............................................................................................................................
//调用方法
//首先调用StartWork启动整个通讯模块，注意：如果要使用SSL加密，参数中证书路径必须填写，否则按照非加密方式进行通讯
//然后根据实际应用和连接数量，调用CreateMBPClientCommIO_Api创建异步通讯连接或调用CreateMBPSyncClientIO创建同步通讯连接
//整个通讯过程中，可以调用GetStatus获取当前通讯信息，如连接数量、发送数量、带宽等等
//需要停止通讯时，必须按照如下过程
//	1、将所有创建出来的对象，全部调用Release进行释放
//	2、在完成第1步的基础上，才调用EndWork关闭整个通讯过程
//	注意：第1步和第2步顺序不能颠倒
//-----------------------------------------------------------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------------------------------------------------------