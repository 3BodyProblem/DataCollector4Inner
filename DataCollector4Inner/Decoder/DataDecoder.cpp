#include "DataDecoder.h"
#include "../DataCollector4Inner.h"


DataDecoder::DataDecoder()
 : m_pDecoderApi( NULL )
{
}

int DataDecoder::Initialize( std::string sPluginPath, std::string sCnfXml, unsigned int nXCodeBuffSize )
{
	Release();
	QuoCollector::GetCollector()->OnLog( TLV_INFO, "DataDecoder::Initialize() : initializing data decoder plugin ......" );

	T_Func_FetchModuleVersion	pFunGetVersion = NULL;
	T_Func_GetDecodeApi			pFuncDecodeApi = NULL;
	int							nErrorCode = m_oDllPlugin.LoadDll( sPluginPath );

	if( 0 != nErrorCode )
	{
		QuoCollector::GetCollector()->OnLog( TLV_ERROR, "DataDecoder::Initialize() : failed 2 load data decoder plugin [%s], errorcode=%d", sPluginPath.c_str(), nErrorCode );
		return nErrorCode;
	}

	pFunGetVersion = (T_Func_FetchModuleVersion)m_oDllPlugin.GetDllFunction( "FetchModuleVersion" );
	pFuncDecodeApi = (T_Func_GetDecodeApi)m_oDllPlugin.GetDllFunction( "GetDecodeApi" );

	if( NULL == pFunGetVersion || NULL == pFuncDecodeApi )
	{
		QuoCollector::GetCollector()->OnLog( TLV_ERROR, "DataDecoder::Initialize() : invalid fuction pointer(NULL)" );
		return -100;
	}

	m_pDecoderApi = pFuncDecodeApi();
	QuoCollector::GetCollector()->OnLog( TLV_INFO, "DataDecoder::Initialize() : DataDecoder Version => [%s] ......", pFunGetVersion() );
	if( NULL == m_pDecoderApi )
	{
		QuoCollector::GetCollector()->OnLog( TLV_ERROR, "DataDecoder::Initialize() : invalid decoder obj. pointer(NULL)" );
		return -200;
	}

	if( 0 != (nErrorCode = m_pDecoderApi->Initialize( sCnfXml.c_str() )) )
	{
		QuoCollector::GetCollector()->OnLog( TLV_ERROR, "DataDecoder::Initialize() : failed 2 initialize data decoder, configuration file: %s", sCnfXml.c_str() );
		return nErrorCode;
	}

	QuoCollector::GetCollector()->OnLog( TLV_INFO, "DataDecoder::Initialize() : data decoder plugin is initialized ......" );

	return 0;
}

void DataDecoder::Release()
{
	if( NULL != m_pDecoderApi )
	{
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "DataDecoder::Release() : releasing data decoder plugin ......" );
		m_pDecoderApi->Release();
		delete m_pDecoderApi;
		m_pDecoderApi = NULL;
		QuoCollector::GetCollector()->OnLog( TLV_INFO, "DataDecoder::Release() : data decoder plugin is released ......" );
	}

	m_oDllPlugin.CloseDll();
}

int DataDecoder::Prepare4AUncompression( const char* pData, unsigned int nLen )
{
	if( NULL == m_pDecoderApi || NULL == pData )
	{
		return -1;
	}

	return m_pDecoderApi->Attach2Buffer( (char*)pData, nLen );
}

int DataDecoder::UncompressData( unsigned short nMsgID, char *pData, unsigned int nLen )
{
	if( NULL == m_pDecoderApi || NULL == pData )
	{
		return -1;
	}

	return m_pDecoderApi->DecodeMessage( nMsgID, pData, nLen );
}















