//-----------------------------------------------------------------------------------------------------------------------------
#include "MBPClientCommIO.h"
#include <stdio.h>
#include "../Configuration.h"
//-----------------------------------------------------------------------------------------------------------------------------
MBPClientCommIO::MBPClientCommIO(void)
{
	m_hDllHandle = NULL;
	m_lpStartWork = NULL;
	m_lpEndWork = NULL;
	m_lpCreateASyncApi = NULL;
	m_lpCreateSyncClient = NULL;
	m_lpGetStatus = NULL;
	m_lpGetVersion = NULL;
}
//.............................................................................................................................
MBPClientCommIO::~MBPClientCommIO()
{
	Release();
}
//.............................................................................................................................
int  MBPClientCommIO::Instance(const tagMBPClientIO_RunParam * lpParam)
{
	char						tempbuf[256];
	char						dllpath[256];
	int							i;
	int							errorcode;

	Release();

	::GetModuleFileNameA(g_oModule,tempbuf,sizeof(tempbuf));
	for ( i=strlen(tempbuf)-1;i>=0;i-- )
	{
		if ( tempbuf[i] == '\\' || tempbuf[i] == '/' )
		{
			tempbuf[i+1] = 0;
			break;
		}
	}
	_snprintf(dllpath,sizeof(dllpath),"%sMBPClientCommIO.dll",tempbuf);

	if ( (m_hDllHandle = ::LoadLibraryA(dllpath)) == NULL )
	{
		Release();
		return(-1);
	}

	m_lpStartWork = (tagFun_StartWork *)::GetProcAddress(m_hDllHandle,"StartWork");
	m_lpEndWork = (tagFun_EndWork *)::GetProcAddress(m_hDllHandle,"EndWork");
	m_lpCreateASyncApi = (tagFun_CreateMBPClientCommIO_Api *)::GetProcAddress(m_hDllHandle,"CreateMBPClientCommIO_Api");
	m_lpCreateSyncClient = (tagFun_CreateMBPSyncClientIO *)::GetProcAddress(m_hDllHandle,"CreateMBPSyncClientIO");
	m_lpGetStatus = (tagFun_GetStatus *)::GetProcAddress(m_hDllHandle,"GetStatus");
	m_lpGetVersion = (tagFun_GetDllVersion *)::GetProcAddress(m_hDllHandle,"GetDllVersion");

	if ( m_lpStartWork == NULL || m_lpEndWork == NULL || m_lpCreateASyncApi == NULL || m_lpCreateSyncClient == NULL || m_lpGetStatus == NULL || m_lpGetVersion == NULL )
	{
		Release();
		return(-2);
	}

	if ( (errorcode = m_lpStartWork(lpParam)) < 0 )
	{
		Release();
		return(-3);
	}

	return(1);
}
//.............................................................................................................................
void MBPClientCommIO::Release(void)
{
	if ( m_lpEndWork != NULL )
	{
		m_lpEndWork();
		m_lpEndWork = NULL;
	}

	if ( m_hDllHandle != NULL )
	{
		::FreeLibrary(m_hDllHandle);
		m_hDllHandle = NULL;
	}

	m_lpStartWork = NULL;
	m_lpCreateASyncApi = NULL;
	m_lpCreateSyncClient = NULL;
	m_lpGetStatus = NULL;
}
//.............................................................................................................................
MBPClientCommIO_Api * MBPClientCommIO::CreateMBPClientCommIO_Api(void)
{
	if ( m_lpCreateASyncApi == NULL )
	{
		return(NULL);
	}

	return(m_lpCreateASyncApi());
}
//.............................................................................................................................
MBPSyncClientIO		* MBPClientCommIO::CreateMBPSyncClientIO(void)
{
	if ( m_lpCreateSyncClient == NULL )
	{
		return(NULL);
	}

	return(m_lpCreateSyncClient());
}
//.............................................................................................................................
void MBPClientCommIO::GetStatus(tagMBPClientIO_Status * lpOut)
{
	if ( m_lpGetStatus != NULL )
	{
		m_lpGetStatus(lpOut);
	}
}
//.............................................................................................................................
unsigned int MBPClientCommIO::GetVersion(void)
{
	if ( m_lpGetVersion == NULL )
	{
		return(0);
	}

	return(m_lpGetVersion());
}
//-----------------------------------------------------------------------------------------------------------------------------