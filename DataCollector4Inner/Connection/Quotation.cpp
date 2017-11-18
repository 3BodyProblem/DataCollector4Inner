#include <algorithm>
#include "Quotation.h"
#include "../DataCollector4Inner.h"
#include "time.h"


WorkStatus::WorkStatus()
: m_eWorkStatus( ET_SS_UNACTIVE )
{
}

WorkStatus::WorkStatus( const WorkStatus& refStatus )
{
	CriticalLock	section( m_oLock );

	m_eWorkStatus = refStatus.m_eWorkStatus;
}

WorkStatus::operator enum E_SS_Status()
{
	CriticalLock	section( m_oLock );

	return m_eWorkStatus;
}

std::string& WorkStatus::CastStatusStr( enum E_SS_Status eStatus )
{
	static std::string	sUnactive = "未激活";
	static std::string	sDisconnected = "断开状态";
	static std::string	sConnected = "连通状态";
	static std::string	sLogin = "登录成功";
	static std::string	sInitialized = "初始化中";
	static std::string	sWorking = "推送行情中";
	static std::string	sUnknow = "不可识别状态";

	switch( eStatus )
	{
	case ET_SS_UNACTIVE:
		return sUnactive;
	case ET_SS_DISCONNECTED:
		return sDisconnected;
	case ET_SS_CONNECTED:
		return sConnected;
	case ET_SS_LOGIN:
		return sLogin;
	case ET_SS_INITIALIZING:
		return sInitialized;
	case ET_SS_WORKING:
		return sWorking;
	default:
		return sUnknow;
	}
}

WorkStatus&	WorkStatus::operator= ( enum E_SS_Status eWorkStatus )
{
	CriticalLock	section( m_oLock );

	if( m_eWorkStatus != eWorkStatus )
	{
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "WorkStatus::operator=() : Exchange Session Status [%s]->[%s]"
											, CastStatusStr(m_eWorkStatus).c_str(), CastStatusStr(eWorkStatus).c_str() );

		m_eWorkStatus = eWorkStatus;
	}

	return *this;
}


///< ----------------------------------------------------------------


MkQuotation::MkQuotation()
 : m_nMarketID( 0 )
{
}

MkQuotation::~MkQuotation()
{
	Destroy();
}

WorkStatus& MkQuotation::GetWorkStatus()
{
	return m_oWorkStatus;
}

int MkQuotation::Destroy()
{
	if( NULL != m_pCommIOApi )
	{
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::Destroy() : ............ Destroying .............." );

		m_oWorkStatus = ET_SS_UNACTIVE;	///< 更新MkQuotation会话的状态0
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::Destroy() : m_pCommIOApi->RegisterSpi( NULL )" );
		m_pCommIOApi->RegisterSpi( NULL );
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::Destroy() : m_pCommIOApi->Disconnect()" );
		m_pCommIOApi->Disconnect();
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::Destroy() : m_pCommIOApi->Release()" );
		m_pCommIOApi->Release();
		m_pCommIOApi = NULL;
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::Destroy() : m_oCommIO.Release()" );
		m_oCommIO.Release();
		m_oDecoder.Release();

		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::Destroy() : ............ Destroyed! .............." );
	}

	return 0;
}

int MkQuotation::Activate()
{
	int									nErrorCode = 0;

	if( GetWorkStatus() == ET_SS_UNACTIVE )
	{
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::Activate() : ............ Link Session Activating............" );
		const tagMBPClientIO_RunParam&		refRunParam = Configuration::GetConfig().GetRunParam();

		Destroy();
		if( (nErrorCode = m_oCommIO.Instance( &refRunParam )) < 0 )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "MkQuotation::Activate() : error occur while initializing Communication Plugin, errorcode=%d", nErrorCode );
			return -1;
		}

		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::Activate() : Communication Plugin Version 【%d】", m_oCommIO.GetVersion() );
		if( NULL == (m_pCommIOApi = m_oCommIO.CreateMBPClientCommIO_Api()) )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "MkQuotation::Activate() : error occur while creating Link control api pointer(NULL)" );
			return -2;
		}

		if( 0 != m_oDecoder.Initialize( Configuration::GetConfig().GetCompressPluginPath(), Configuration::GetConfig().GetCompressPluginCfg(), 1024*1024*30 ) )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "MkQuotation::Activate() : failed 2 initialize decoder plugin." );
			return -3;
		}

		m_pCommIOApi->RegisterSpi( this );
		m_oWorkStatus = ET_SS_DISCONNECTED;				///< 更新MkQuotation会话的状态::GetStatus()

		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::Activate() : ............ MkQuotation Activated!.............." );
	}

	return 0;
}

int MkQuotation::Connect2Server()
{
	if( true == Configuration::GetConfig().IsBroadcastModel() )
	{
		return SimpleTask::Activate();
	}

	if( m_oWorkStatus == ET_SS_DISCONNECTED )
	{
		int									nErrorCode = 0;
		std::string							sIP = "";
		unsigned int						nPort = 0;

		if( false == Configuration::GetConfig().GetHQConfList().GetConfig( sIP, nPort ) )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "MkQuotation::Connect2Server() : invalid data source connection configuration." );
			return -1;
		}

		if( (nErrorCode = m_pCommIOApi->Connect( sIP.c_str(), nPort )) < 0 )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "MkQuotation::Connect2Server() : failed 2 establish connection, errorcode = %d", nErrorCode );
			return -2;
		}

		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::Connect2Server() : connection --> %s : %u", sIP.c_str(), nPort );
	}

	return 0;
}

int MkQuotation::Execute()
{
	return LoadDataFile( Configuration::GetConfig().GetQuotationFilePath().c_str() );
}

int MkQuotation::LoadDataFile( std::string sFilePath )
{
	QuotationRecover		oDataRecover;

	m_nMarketID = 0;
	if( 0 != oDataRecover.OpenFile( sFilePath.c_str(), Configuration::GetConfig().GetBroadcastBeginTime() ) )
	{
		QuoCollector::GetCollector()->OnLog( TLV_ERROR, "CTPQuotation::LoadDataFile() : failed 2 open snap file : %s", sFilePath.c_str() );
		return -1;
	}

	while( true )
	{
		int					nLength = 1024*1024*20;
		unsigned short		nMessageNo = 0;
		unsigned short		nFunctionID = 0;
		static char*		pszData = new char[1024*1024*20];

		nLength = oDataRecover.Read( nMessageNo, nFunctionID, pszData, nLength );

		if( nLength <= 0 )
		{
			break;
		}

		OnQuotation( nMessageNo, nFunctionID, pszData, nLength );
	}

	QuoCollector::GetCollector()->OnLog( TLV_INFO, "CTPQuotation::LoadDataFile() : End of Quotation File..." );

	return 0;
}

void MkQuotation::CloseConnection()
{
	if( NULL != m_pCommIOApi )
	{
		m_pCommIOApi->Disconnect();
	}
}

MBPClientCommIO& MkQuotation::GetCommIO()
{
	return m_oCommIO;
}

unsigned int MkQuotation::GetMarketID() const
{
	return m_nMarketID;
}

int MkQuotation::PrepareDumpFile()
{
	if( false == Configuration::GetConfig().IsDumpModel() )
	{
		return 0;
	}

	QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::PrepareDumpFile() : Creating Quotation\'s dump file ......" );

	char			pszTmpFile[128*2] = { 0 };	///< 准备行情数据落盘
	::sprintf( pszTmpFile, "Quotation_%u_%u.dmp", DateTime::Now().DateToLong(), DateTime::Now().TimeToLong() );
	std::string		sFilePath = JoinPath( Configuration::GetConfig().GetDumpFolder(), pszTmpFile );

	m_oQuotDumper.CloseFile();
	if( false == m_oQuotDumper.OpenFile( sFilePath.c_str() ) )
	{
		QuoCollector::GetCollector()->OnLog( TLV_WARN, "MkQuotation::PrepareDumpFile() : failed 2 create quotation dump file" );
		return -1;
	}

	QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::PrepareDumpFile() : dump file created, result = %s", sFilePath.c_str() );

	return 0;
}

bool MkQuotation::SendLoginPkg()
{
	int							nErrCode = 0;
	std::string					sUserName, sPassword;
	char						pszSendData[1024] = { 0 };
	tagPackageHead*				pPkgHead = (tagPackageHead*)pszSendData;
	tagCommonLoginData_LF299*	pMsgBody = (tagCommonLoginData_LF299*)( pszSendData + sizeof(tagPackageHead) );

	if( false == Configuration::GetConfig().GetHQConfList().GetLoginInfo( sUserName, sPassword ) )
	{
		QuoCollector::GetCollector()->OnLog( TLV_WARN, "MkQuotation::SendLoginPkg() : invalid data source login data." );
		return false;
	}

	::strcpy( pMsgBody->pszActionKey, "login" );
	::strcpy( pMsgBody->pszUserName, sUserName.c_str() );
	::strcpy( pMsgBody->pszPassword, sPassword.c_str() );
	pMsgBody->nReqDBSerialNo = 0;

	pPkgHead->nSeqNo = 0;
	pPkgHead->nMarketID = 0;
	pPkgHead->nMsgLength = sizeof(tagCommonLoginData_LF299);

	if( (nErrCode=m_pCommIOApi->SendData( 299, 100, pszSendData, sizeof(tagCommonLoginData_LF299) + sizeof(tagPackageHead), true )) < 0 )
	{
		QuoCollector::GetCollector()->OnLog( TLV_WARN, "MkQuotation::SendLoginPkg() : failed 2 send login package." );
		return false;
	}

	m_oWorkStatus = ET_SS_INITIALIZING;			///< 更新MkQuotation会话的状态

	return true;
}

void MkQuotation::OnConnectSuc()
{
	QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::OnConnectSuc() : connection established." );

	m_oWorkStatus = ET_SS_CONNECTED;			///< 更新MkQuotation会话的状态

	if( false == SendLoginPkg() ) {
		return;
	}

	QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::OnConnectSuc() : login message is sended." );
	PrepareDumpFile();							///< 预备落盘文件的句柄
}

void MkQuotation::OnConnectFal()
{
	QuoCollector::GetCollector()->OnLog( TLV_WARN, "MkQuotation::OnConnectFal() : connection isn\'t established" );

	m_oWorkStatus = ET_SS_DISCONNECTED;			///< 更新MkQuotation会话的状态
}

void MkQuotation::OnDisconnect()
{
	QuoCollector::GetCollector()->OnLog( TLV_WARN, "MkQuotation::OnDisconnect() : connection disconnected" );

	m_oWorkStatus = ET_SS_DISCONNECTED;			///< 更新MkQuotation会话的状态
}

void MkQuotation::OnDestory()
{
}

bool MkQuotation::OnRecvData( unsigned short usMessageNo, unsigned short usFunctionID, bool bErrorFlag, const char* lpData, unsigned int uiSize )
{
	if( NULL == lpData )
	{
		QuoCollector::GetCollector()->OnLog( TLV_ERROR, "MkQuotation::OnRecvData() : invalid quotation data pointer (NULL)" );
		return false;
	}

	if( true == bErrorFlag )
	{
		QuoCollector::GetCollector()->OnLog( TLV_ERROR, "MkQuotation::OnRecvData() : an unknow error occur while receiving quotation." );
		return false;
	}

	if( true == Configuration::GetConfig().IsDumpModel() )
	{
		m_oQuotDumper.Record( usMessageNo, usFunctionID, lpData, uiSize );
	}

	if( 0 != m_oDecoder.Prepare4AUncompression( lpData ) )
	{
		QuoCollector::GetCollector()->OnLog( TLV_ERROR, "MkQuotation::OnRecvData() : failed 2 prepare a uncompression." );
		return false;
	}

	///< m_oDecoder

	return OnQuotation( usMessageNo, usFunctionID, lpData, uiSize );
}

bool MkQuotation::OnQuotation( unsigned short usMessageNo, unsigned short usFunctionID, const char* lpData, unsigned int uiSize )
{
	tagPackageHead*				pFrameHead = (tagPackageHead*)lpData;
	unsigned int				nFrameSeq = pFrameHead->nSeqNo;

	if( 0 == m_nMarketID )
	{
		m_nMarketID = pFrameHead->nMarketID;
	}

	if( m_oWorkStatus == ET_SS_WORKING )
	{
		QuoCollector::GetCollector()->OnStream( usMessageNo, lpData, uiSize );
	}

	for( unsigned int nOffset = sizeof(tagPackageHead); nOffset < uiSize; nOffset += pFrameHead->nMsgLength )
	{
		char*					pMsgBody = (char*)(lpData+nOffset);

		if( 299 == usMessageNo && 100 == usFunctionID )			///< MsgID == 299 是登录返回包
		{
			tagCommonLoginData_LF299*	pLoginResp = (tagCommonLoginData_LF299*)pMsgBody;

			if( 0 == ::strncmp( pLoginResp->pszActionKey, "success", ::strlen( "success" ) ) )
			{
				m_oWorkStatus = ET_SS_LOGIN;				///< 登录成功，更新MkQuotation会话的状态
				QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::OnQuotation() : log 2 server successfully." );
			}
			else
			{
				QuoCollector::GetCollector()->OnLog( TLV_WARN, "MkQuotation::OnQuotation() : failed 2 login!" );
			}
		}
		else if( 0 != usMessageNo )							///< MsgID == 0 是心跳包
		{
			if( m_oWorkStatus == ET_SS_WORKING )
			{
				QuoCollector::GetCollector()->OnData( usMessageNo, pMsgBody, pFrameHead->nMsgLength, false, false );
			}
			else
			{
				bool	bLastImageFlag = ( (nOffset+pFrameHead->nMsgLength >=uiSize) && (100==usFunctionID) ) ? true : false;

				QuoCollector::GetCollector()->OnImage( usMessageNo, pMsgBody, pFrameHead->nMsgLength, bLastImageFlag );

				if( true == bLastImageFlag )
				{
					m_oWorkStatus = ET_SS_WORKING;	///< 收到重复代码，全幅快照已收完整ET_SS_INITIALIZED
				}
			}
		}
	}

	return true;
}






