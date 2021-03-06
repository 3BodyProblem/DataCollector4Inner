#include "targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "Configuration.h"
#include "UnitTest/UnitTest.h"
#include "DataCollector4Inner.h"


const std::string		g_sVersion = "1.3.5";


QuoCollector::QuoCollector()
 : m_pCbDataHandle( NULL )
{
}

QuoCollector& QuoCollector::GetCollector()
{
	static	QuoCollector	obj;

	return obj;
}

int QuoCollector::Initialize( I_DataHandle* pIDataHandle )
{
	int		nErrorCode = 0;

	m_pCbDataHandle = pIDataHandle;
	if( NULL == m_pCbDataHandle )
	{
		::printf( "QuoCollector::Initialize() : invalid arguments (NULL), version = %s\n", g_sVersion.c_str() );
		return -1;
	}

	QuoCollector::GetCollector()->OnLog( TLV_ERROR, "QuoCollector::Initialize() : [Version] %s", g_sVersion.c_str() );

	if( 0 != (nErrorCode = Configuration::GetConfig().Initialize()) )
	{
		QuoCollector::GetCollector()->OnLog( TLV_ERROR, "QuoCollector::Initialize() : failed 2 initialize configuration obj, errorcode=%d", nErrorCode );
		return -2;
	}

	if( 0 != (nErrorCode=m_oQuotationData.Activate()) )
	{
		QuoCollector::GetCollector()->OnLog( TLV_WARN, "QuoCollector::Initialize() : failed 2 activate quotation obj, errorcode=%d", nErrorCode );
		return -3;
	}

	return 0;
}

void QuoCollector::Release()
{
	m_oQuotationData.Destroy();
}

I_DataHandle* QuoCollector::operator->()
{
	if( NULL == m_pCbDataHandle )
	{
		::printf( "QuoCollector::operator->() : invalid data callback interface ptr.(NULL)\n" );
	}

	return m_pCbDataHandle;
}

unsigned int QuoCollector::GetMarketID() const
{
	return m_oQuotationData.GetMarketID();
}

void QuoCollector::Halt()
{
	m_oQuotationData.CloseConnection();
}

int QuoCollector::RecoverQuotation()
{
	unsigned int	nSec = 0;
	int				nErrorCode = 0;

	if( 0 != (nErrorCode=m_oQuotationData.Connect2Server()) )
	{
		QuoCollector::GetCollector()->OnLog( TLV_WARN, "QuoCollector::RecoverQuotation() : failed 2 subscript quotation, errorcode=%d", nErrorCode );
		return -1;
	}

	for( nSec = 0; nSec < 12 && ET_SS_WORKING != m_oQuotationData.GetWorkStatus(); nSec++ )
	{
		SimpleTask::Sleep( 1000 * 1 );
	}

	if( ET_SS_WORKING == m_oQuotationData.GetWorkStatus() )
	{
		return 0;
	}
	else
	{
		QuoCollector::GetCollector()->OnLog( TLV_WARN, "QuoCollector::RecoverQuotation() : overtime [> %d sec.], errorcode=%d", nSec, nErrorCode );
		return -2;
	}
}

enum E_SS_Status QuoCollector::GetCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen )
{
	tagMBPClientIO_Status	tagStatus = { 0 };
	Configuration&			refCnf = Configuration::GetConfig();
	WorkStatus&				refStatus = m_oQuotationData.GetWorkStatus();
	std::string&			sStatusDesc = WorkStatus::CastStatusStr( (enum E_SS_Status)refStatus );

	if( refStatus == ET_SS_UNACTIVE )
	{
		::memset( &tagStatus, 0, sizeof(tagStatus) );
	}
	else
	{
		m_oQuotationData.GetCommIO().GetStatus( &tagStatus );
	}

	nStrLen = ::sprintf( pszStatusDesc, "模块名=自适应行情代理,Version=%s,市场编号=%u,快照目录=%s,连接状态=%s,发送缓冲占比=%u(％),内存池占比=%u(％),接收频率=%u(次/秒),发送频率=%u(次/秒),接收带宽=%u(bps),接收总量=%I64d(字节)"
		, g_sVersion.c_str(), m_oQuotationData.GetMarketID(), refCnf.GetDumpFolder().c_str(), sStatusDesc.c_str()
		, tagStatus.uiSendBufPercent, tagStatus.uiMemoryPoolPercent, tagStatus.uiRecvFreq, tagStatus.uiSendFreq, tagStatus.uiRecvBandWidth, tagStatus.ui64RecvAmount );

	return (enum E_SS_Status)(m_oQuotationData.GetWorkStatus());
}


extern "C"
{
	__declspec(dllexport) int __stdcall	Initialize( I_DataHandle* pIDataHandle )
	{
		return QuoCollector::GetCollector().Initialize( pIDataHandle );
	}

	__declspec(dllexport) void __stdcall Release()
	{
		QuoCollector::GetCollector().Release();
	}

	__declspec(dllexport) int __stdcall	RecoverQuotation()
	{
		return QuoCollector::GetCollector().RecoverQuotation();
	}

	__declspec(dllexport) void __stdcall HaltQuotation()
	{
		QuoCollector::GetCollector().Halt();
	}

	__declspec(dllexport) bool __stdcall IsProxy()
	{
		return true;
	}

	__declspec(dllexport) int __stdcall	GetStatus( char* pszStatusDesc, unsigned int& nStrLen )
	{
		return QuoCollector::GetCollector().GetCollectorStatus( pszStatusDesc, nStrLen );
	}

	__declspec(dllexport) int __stdcall	GetMarketID()
	{
		return QuoCollector::GetCollector().GetMarketID();
	}

	__declspec(dllexport) void __stdcall	ExecuteUnitTest()
	{
		::printf( "\n\n---------------------- [Begin] -------------------------\n" );
		ExecuteTestCase();
		::printf( "----------------------  [End]  -------------------------\n\n\n" );
	}

	__declspec(dllexport) void __stdcall Echo()
	{
		MkQuotation				objQuotation;

		objQuotation.LoadDataFile( Configuration::GetConfig().GetQuotationFilePath().c_str() );		///< 解析实时文件
	}

}




