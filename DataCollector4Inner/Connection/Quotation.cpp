#include <algorithm>
#include "Quotation.h"
#include "../DataCollector4Inner.h"


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
		m_pCommIOApi->RegisterSpi( NULL );
		m_pCommIOApi->Disconnect();
		m_pCommIOApi->Release();
		m_pCommIOApi = NULL;
		m_oCommIO.Release();

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

		m_pCommIOApi->RegisterSpi( this );
		m_oWorkStatus = ET_SS_DISCONNECTED;				///< 更新MkQuotation会话的状态::GetStatus()

		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::Activate() : ............ MkQuotation Activated!.............." );
	}

	return 0;
}

int MkQuotation::RecoverQuotation()
{
	if( m_oWorkStatus == ET_SS_DISCONNECTED )
	{
		int									nErrorCode = 0;
		std::string							sIP = "";
		unsigned int						nPort = 0;

		if( false == Configuration::GetConfig().GetHQConfList().GetConfig( sIP, nPort ) )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "MkQuotation::RecoverQuotation() : invalid data source connection configuration." );
			return -1;
		}

		if( (nErrorCode = m_pCommIOApi->Connect( sIP.c_str(), nPort )) < 0 )
		{
			QuoCollector::GetCollector()->OnLog( TLV_WARN, "MkQuotation::RecoverQuotation() : failed 2 establish connection, errorcode = %d", nErrorCode );
			return -2;
		}

		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::Activate() : connection --> %s : %u", sIP.c_str(), nPort );
	}

	return 0;
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
	if( GetWorkStatus() == ET_SS_LOGIN )			///< 登录成功后，执行订阅操作
	{
//		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::PrepareDumpFile() : Creating ctp session\'s dump file ......" );

		DateTime		oTodayDate;
		char			pszTmpFile[128] = { 0 };			///< 准备行情数据落盘
		unsigned int	nNowTime = DateTime::Now().TimeToLong();

		oTodayDate.SetCurDateTime();
		if( nNowTime > 40000 && nNowTime < 180000 ) {
			::strcpy( pszTmpFile, "Quotation_day.dmp" );
		} else if( nNowTime > 0 && nNowTime < 40000 ) {
			oTodayDate -= (60*60*8);
			::strcpy( pszTmpFile, "Quotation_nite.dmp" );
		} else {
			::strcpy( pszTmpFile, "Quotation_nite.dmp" );
		}
//		std::string		sDumpFile = GenFilePathByWeek( Configuration::GetConfig().GetDumpFolder().c_str(), pszTmpFile, oTodayDate.Now().DateToLong() );
//		bool			bRet = QuotationSync::CTPSyncSaver::GetHandle().Init( sDumpFile.c_str(), DateTime::Now().DateToLong(), false );

		m_oWorkStatus = ET_SS_INITIALIZING;		///< 更新MkQuotation会话的状态
//		QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::PrepareDumpFile() : dump file created, result = %d", bRet );

		return 0;
	}

	return -2;
}

void MkQuotation::OnConnectSuc()
{
	QuoCollector::GetCollector()->OnLog( TLV_INFO, "MkQuotation::OnConnectSuc() : connection established." );

	m_oWorkStatus = ET_SS_CONNECTED;			///< 更新MkQuotation会话的状态
	m_oWorkStatus = ET_SS_LOGIN;				///< 更新MkQuotation会话的状态
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

	if( 100 != usMessageNo )
	{
		QuoCollector::GetCollector()->OnLog( TLV_ERROR, "MkQuotation::OnRecvData() : an unknow message no (%u)", usMessageNo );
		return false;
	}

//	QuotationSync::CTPSyncSaver::GetHandle().SaveSnapData( *pMarketData );

	return OnQuotation( usMessageNo, usFunctionID, lpData, uiSize );
}

bool MkQuotation::OnQuotation( unsigned short usMessageNo, unsigned short usFunctionID, const char* lpData, unsigned int uiSize )
{
	const char*					pBody = lpData + sizeof(tagPackageHead);
	tagPackageHead*				pFrameHead = (tagPackageHead*)lpData;
	unsigned int				nBodyLen = pFrameHead->nBodyLen;
	unsigned int				nMsgCount = pFrameHead->nMsgCount;
	unsigned int				nFrameSeq = pFrameHead->nSeqNo;

	if( 0 == m_nMarketID )
	{
		m_nMarketID = pFrameHead->nMarketID;
	}

	for( unsigned int nOffset = 0; nOffset < nBodyLen; )
	{
		const tagBlockHead*		pMsg = (tagBlockHead*)(pBody+nOffset);
		char*					pMsgBody = (char*)(pBody+nOffset+sizeof(tagBlockHead));

		if( 0 != pMsg->nDataType )		///< MsgID == 0 是心跳包
		{
			if( m_oWorkStatus == ET_SS_WORKING )
			{
				QuoCollector::GetCollector()->OnData( pMsg->nDataType, pMsgBody, pMsg->nDataLen, false );
			}
			else
			{
				bool	bLastImageFlag = ( (nOffset+((tagBlockHead*)(pBody+nOffset))->nDataLen+sizeof(tagBlockHead) >=nBodyLen) && (100==usFunctionID) ) ? true : false;

				QuoCollector::GetCollector()->OnImage( pMsg->nDataType, pMsgBody, pMsg->nDataLen, bLastImageFlag );

				if( true == bLastImageFlag )
				{
					m_oWorkStatus = ET_SS_WORKING;	///< 收到重复代码，全幅快照已收完整ET_SS_INITIALIZED
				}
			}
		}

		nOffset += (pMsg->nDataLen+sizeof(tagBlockHead));
	}

	return true;
}






