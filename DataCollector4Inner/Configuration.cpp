#include <string>
#include <algorithm>
#include "Configuration.h"
#include "DataCollector4Inner.h"
#include "Infrastructure/IniFile.h"


HMODULE				g_oModule = NULL;


LinkConfig::LinkConfig()
 : m_nDataPos( 0 )
{
}

bool LinkConfig::GetConfig( std::string& sIP, unsigned int& nPort )
{
	if( m_vctSvrIP.size() <= 0 || m_vctSvrPort.size() <= 0 || m_vctSvrPort.size() != m_vctSvrIP.size() )
	{
		return false;
	}

	if( m_nDataPos >= m_vctSvrIP.size() )
	{
		m_nDataPos = 0;
	}

	sIP = m_vctSvrIP[m_nDataPos];
	nPort = m_vctSvrPort[m_nDataPos];
	m_nDataPos++;

	return true;
}


void OnError( const char* szErrString )
{
	QuoCollector::GetCollector()->OnLog( TLV_DETAIL, "MBPClientCommIO::OnError() : error info : %s", szErrString );
}

///< 解析并加载服务器连接登录配置信息
int ParseSvrConfig( inifile::IniFile& refIniFile, std::string sNodeName, LinkConfig& oConfig )
{
	int		nErrCode = 0;
	int		nIpCount = refIniFile.getIntValue( sNodeName, std::string("count"), nErrCode );

	for( int n = 0; n < nIpCount; n++ )
	{
		char			pszTmp[32] = { 0 };
		::sprintf( pszTmp, "%d", n );
		std::string		sIP = refIniFile.getStringValue( sNodeName, std::string("SvrIP_")+pszTmp, nErrCode );
		if( 0 != nErrCode )	{
			QuoCollector::GetCollector()->OnLog( TLV_ERROR, "Configuration::ParseSvrConfig() : invalid Server IP." );
			oConfig.m_vctSvrIP.clear();
			oConfig.m_vctSvrPort.clear();
			return -100 - n;
		}
		oConfig.m_vctSvrIP.push_back( sIP );

		unsigned int	nPort = refIniFile.getIntValue( sNodeName, std::string("SvrPort_")+pszTmp, nErrCode );
		if( 0 != nErrCode )	{
			QuoCollector::GetCollector()->OnLog( TLV_ERROR, "Configuration::ParseSvrConfig() : invalid Server Port." );
			oConfig.m_vctSvrIP.clear();
			oConfig.m_vctSvrPort.clear();
			return -200 - n;
		}
		oConfig.m_vctSvrPort.push_back( nPort );
	}

	return 0;
}


Configuration::Configuration()
{
	Release();
}

Configuration& Configuration::GetConfig()
{
	static Configuration	obj;

	return obj;
}

void Configuration::Release()
{
	m_sDumpFileFolder = "";
	::memset( &m_tagClientRunParam, 0, sizeof(m_tagClientRunParam) );
}

int Configuration::Initialize()
{
	std::string			sPath;
	inifile::IniFile	oIniFile;
	int					nErrCode = 0;
    char				pszTmp[1024] = { 0 };

	Release();

    ::GetModuleFileName( g_oModule, pszTmp, sizeof(pszTmp) );
    sPath = pszTmp;
    sPath = sPath.substr( 0, sPath.find(".dll") ) + ".ini";
	if( 0 != (nErrCode=oIniFile.load( sPath )) )
	{
		QuoCollector::GetCollector()->OnLog( TLV_ERROR, "Configuration::Initialize() : configuration file isn\'t exist. [%s], errorcode=%d", sPath.c_str(), nErrCode );
		return -1;
	}

	m_tagClientRunParam.uiMaxLinkCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("maxlinkcount"), nErrCode );
	if( 0 == m_tagClientRunParam.uiMaxLinkCount )	{
		m_tagClientRunParam.uiMaxLinkCount = 128;
	}
	m_tagClientRunParam.uiSendBufCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("sendbufcount"), nErrCode );
	if( 0 == m_tagClientRunParam.uiSendBufCount )	{
		m_tagClientRunParam.uiSendBufCount = m_tagClientRunParam.uiMaxLinkCount * 10;
	}
	m_tagClientRunParam.uiThreadCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("threadcount"), nErrCode );
	if( 0 == m_tagClientRunParam.uiThreadCount )	{
		m_tagClientRunParam.uiThreadCount = 10;
	}
	m_tagClientRunParam.uiPageSize = oIniFile.getIntValue( std::string("ServerIO"), std::string("pagesize"), nErrCode );
	if( 0 == m_tagClientRunParam.uiPageSize )	{
		m_tagClientRunParam.uiPageSize = 1024;
	}
	m_tagClientRunParam.uiPageCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("pagecount"), nErrCode );
	if( 0 == m_tagClientRunParam.uiPageCount )	{
		m_tagClientRunParam.uiPageCount = 1024 * 20;
	}
	m_tagClientRunParam.bSSL = false;
	int	nDetailLog = oIniFile.getIntValue( std::string("ServerIO"), std::string("isdetaillog"), nErrCode );
	m_tagClientRunParam.bDetailLog = nDetailLog==1?true:false;
	m_tagClientRunParam.lpOnError = OnError;

	///< 设置： 快照落盘目录(含文件名)
	m_sDumpFileFolder = oIniFile.getStringValue( std::string("SRV"), std::string("DumpFolder"), nErrCode );
	if( 0 != nErrCode )	{
		QuoCollector::GetCollector()->OnLog( TLV_WARN, "Configuration::Initialize() : shutdown dump function." );
	}

	if( 0 != ParseSvrConfig( oIniFile, "SRV", m_oHQConfigList ) )	{
		return -2;
	}

	return 0;
}

const std::string& Configuration::GetDumpFolder() const
{
	return m_sDumpFileFolder;
}

LinkConfig& Configuration::GetHQConfList()
{
	return m_oHQConfigList;
}

const tagMBPClientIO_RunParam& Configuration::GetRunParam() const
{
	return m_tagClientRunParam;
}








