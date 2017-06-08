//-----------------------------------------------------------------------------------------------------------------------------
//                `-.   \    .-'
//        ,-"`````""-\__ |  /							�ļ����ƣ�MBPClientCommIO
//         '-.._    _.-'` '-o,							�ļ�������MBP�ͻ��˽ӿ�
//             _>--:{{<   ) |)							�ļ���д��Lumy
//         .-''      '-.__.-o`							�������ڣ�2016.10.19
//        '-._____..-/`  |  \							�������ڣ�2016.10.19
//                ,-'   /    `-.
//-----------------------------------------------------------------------------------------------------------------------------
#ifndef __MPBClientCommIO_H__
#define __MPBClientCommIO_H__
//-----------------------------------------------------------------------------------------------------------------------------
#include "MBPClientCommIO.hpp"
#include <windows.h>
//-----------------------------------------------------------------------------------------------------------------------------
typedef int  tagFun_StartWork(const tagMBPClientIO_RunParam * lpParam);
typedef void tagFun_EndWork(void);
typedef MBPClientCommIO_Api * tagFun_CreateMBPClientCommIO_Api(void);
typedef MBPSyncClientIO * tagFun_CreateMBPSyncClientIO(void);
typedef void tagFun_GetStatus(tagMBPClientIO_Status * lpOut);
typedef unsigned int tagFun_GetDllVersion(void);
//-----------------------------------------------------------------------------------------------------------------------------
class MBPClientCommIO
{
protected:
	HINSTANCE								m_hDllHandle;
	tagFun_StartWork					*	m_lpStartWork;
	tagFun_EndWork						*	m_lpEndWork;
	tagFun_CreateMBPClientCommIO_Api	*	m_lpCreateASyncApi;
	tagFun_CreateMBPSyncClientIO		*	m_lpCreateSyncClient;
	tagFun_GetStatus					*	m_lpGetStatus;
	tagFun_GetDllVersion				*	m_lpGetVersion;
public:
	MBPClientCommIO(void);
	virtual ~MBPClientCommIO();
public:
	int  Instance(const tagMBPClientIO_RunParam * lpParam);
	void Release(void);
public:
	MBPClientCommIO_Api * CreateMBPClientCommIO_Api(void);
	MBPSyncClientIO		* CreateMBPSyncClientIO(void);
	void GetStatus(tagMBPClientIO_Status * lpOut);
	unsigned int GetVersion(void);
};
//-----------------------------------------------------------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------------------------------------------------------