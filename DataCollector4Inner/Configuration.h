#ifndef __DATA_COLLECTOR_H__
#define __DATA_COLLECTOR_H__


#pragma warning(disable: 4786)
#include <vector>
#include <string>
#include "Infrastructure/Lock.h"
#include "Connection/MBPClientCommIO.h"


extern	HMODULE						g_oModule;				///< 当前dll模块变量


/**
 * @class							LinkConfig
 * @brief							一组连接信息
 * @date							2017/5/15
 * @author							barry
 */
class LinkConfig
{
public:
	LinkConfig();

	/**
	 * @brief						获取数据源链接配置
	 */
	bool							GetConfig( std::string& sIP, unsigned int& nPort );

	/**
	 * @brief						获取登录信息
	 */
	bool							GetLoginInfo( std::string& sUserName, std::string& sPassword );

public:
	std::vector<std::string>		m_vctSvrIP;				///< 服务器地址
	std::vector<unsigned int>		m_vctSvrPort;			///< 服务器端口
	std::vector<std::string>		m_vctUserName;			///< 用户名
	std::vector<std::string>		m_vctPassword;			///< 密码

protected:
	int								m_nDataPos;				///< 数据位置
};


/**
 * @class							Configuration
 * @brief							配置信息
 * @date							2017/5/15
 * @author							barry
 */
class Configuration
{
protected:
	Configuration();

public:
	/**
	 * @brief						获取配置对象的单键引用对象
	 */
	static Configuration&			GetConfig();

	/**
	 * @brief						加载配置项
	 * @return						==0						成功
									!=						出错
	 */
    int								Initialize();

	/**
	 * @brief						释放资源
	 */
	void							Release();

public:
	/**
	 * @brief						获取各市场行情链路配置表
	 */
	LinkConfig&						GetHQConfList();

	/**
	 * @brief						运行参数
	 */
	const tagMBPClientIO_RunParam&	GetRunParam() const;

	/**
	 * @brief						取得行情数据落盘目录(含文件名)
	 */
	const std::string&				GetDumpFolder() const;

	/**
	 * @brief						是否需要落盘行情
	 */
	bool							IsDumpModel() const;

	/**
	 * @brief						是否为播放模式
	 * @return						true				行情自动播放模式
	 */
	bool							IsBroadcastModel() const;

	/**
	 * @brief						获取行情文件的路径
	 */
	std::string						GetQuotationFilePath() const;

	/**
	 * @brief						获取正常速度播放的开始时间
	 */
	unsigned int					GetBroadcastBeginTime() const;

	/**
	 * @brief						压缩模块插件路径
	 */
	const std::string&				GetCompressPluginPath() const;

	/**
	 * @brief						数据采集插件配置
	 */
	const std::string&				GetCompressPluginCfg() const;

private:
	std::string						m_sCompressPluginPath;		///< 数据压缩插件所在路径
	std::string						m_sCompressPluginConfig;	///< 数据压缩插件配置文件
private:
	tagMBPClientIO_RunParam			m_tagClientRunParam;		///< API运行参数
	bool							m_bDumpFile;				///< 是否落盘
	std::string						m_sDumpFileFolder;			///< 行情落盘路径
	LinkConfig						m_oHQConfigList;			///< 行情服务器连接配置列表
private:
	bool							m_bBroadcastModel;			///< 数据自动播放模式
	std::string						m_sBcQuotationFile;			///< 播放的实时文件路径
	unsigned int					m_nBcBeginTime;				///< 正常速度的播放时间
};





#endif






