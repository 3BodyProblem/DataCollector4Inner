//-----------------------------------------------------------------------------------------------------------------------------
//                `-.   \    .-'
//        ,-"`````""-\__ |  /							�ļ����ƣ�MBPClientCommIO
//         '-.._    _.-'` '-o,							�ļ�������MBP�ͻ���ͨѶ�ӿ�
//             _>--:{{<   ) |)							�ļ���д��Lumy
//         .-''      '-.__.-o`							�������ڣ�2016.10.18
//        '-._____..-/`  |  \							�������ڣ�2016.10.18
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
	unsigned int						uiMaxLinkCount;			//֧�ֵ�����������������첽��ʽʹ�á�
	unsigned int						uiSendBufCount;			//������Դ���������첽��ʽʹ�á�
	unsigned int						uiThreadCount;			//�߳����������첽��ʽʹ�á�
	unsigned int						uiPageSize;				//�ڴ��ҳ���С�����첽��ʽʹ�á�
	unsigned int						uiPageCount;			//�ڴ��ҳ�����������첽��ʽʹ�á�
	bool								bSSL;					//�Ƿ�����SSL����
	char								szCrtFileName[256];		//OpenSSL .crt֤���ļ�·������SSLʹ��
	char								szPfxFileName[256];		//OpenSSL .pfx֤���ļ�·������SSLʹ��
	char								szPfxFilePasswrod[32];	//OpenSSL .pfx֤�����룬��SSLʹ��
	bool								bDetailLog;				//�Ƿ��ӡ��ϸ��־
	tagFun_OnError					*	lpOnError;				//������Ӧָ��
	unsigned char						ucSSLType;				//SSLЭ��汾��0:��ʹ��SSL 1:SSL3.0/2.0����Э�� 2:TLS1.0 3:TLS1.1 ����:TLS1.2��
	char								szCiperList[2048];		//SSLʹ�õ������׼�����"ECDHE-RSA-AES128-GCM-SHA256"��"AES256-SHA"��
	char								szReserved[2047];		//����
} tagMBPClientIO_RunParam;
//.............................................................................................................................
typedef struct
{
	unsigned int						uiMaxLinkCount;			//�����������
	unsigned int						uiConnectCount;			//������������
	unsigned int						uiCurLinkCount;			//��ǰ��������
	unsigned int						uiSendBufPercent;		//���ͻ���ռ�ðٷֱȡ��ٷֱȡ�
	unsigned int						uiMemoryPoolPercent;	//�ڴ��ռ�ðٷֱȡ��ٷֱȡ�
	unsigned int						uiRecvFreq;				//����Ƶ�ʡ���/�롿
	unsigned int						uiSendFreq;				//����Ƶ�ʡ���/�롿
	unsigned int						uiSendLostFreq;			//���Ͷ�ʧƵ�ʡ���/�롿
	unsigned int						uiRecvBandWidth;		//���մ���bps��
	unsigned int						uiSendBandWidth;		//���ʹ���bps��
	unsigned __int64					ui64RecvAmount;			//�����������ֽڡ�
	unsigned __int64					ui64SendAmount;			//�����������ֽڡ�
	double								dCompressRate;			//ѹ���ʡ��ٷֱȡ�
	unsigned int						uiWorkThreadCount;		//�����߳�����
} tagMBPClientIO_Status;
//-----------------------------------------------------------------------------------------------------------------------------
//�첽ͨѶ��Ӧ����SPI
class MBPClientCommIO_Spi
{
public:
	//���ӳɹ���Ϣ��Ӧ����
	virtual void OnConnectSuc(void) = 0;
	//����ʧ����Ϣ��Ӧ����
	virtual void OnConnectFal(void) = 0;
	//���ӶϿ���Ϣ��Ӧ����
	virtual void OnDisconnect(void) = 0;
	//�յ�������Ϣ��Ӧ����������false��ʾ�������ݴ��󣬶Ͽ�����
	virtual bool OnRecvData(unsigned short usMessageNo,unsigned short usFunctionID,bool bErrorFlag,const char * lpData,unsigned int uiSize) = 0;
	//����API������
	virtual void OnDestory(void) = 0;
};
//-----------------------------------------------------------------------------------------------------------------------------
//�첽ͨѶAPI
class MBPClientCommIO_Api
{
public:
	//ע����Ϣ��Ӧ����
	virtual void RegisterSpi(MBPClientCommIO_Spi * lpSpi) = 0;
	//���ӷ�����
	virtual int  Connect(const char * szIPAddr,unsigned int uiPort) = 0;
	//�Ͽ�����
	virtual void Disconnect(void) = 0;
	//��������
	virtual int  SendData(unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize,bool bCompress) = 0;
	//���ʹ���
	virtual int  SendError(unsigned short usMessageNo,unsigned short usFunctionID,const char * lpErrorInfo) = 0;
	//ɾ���ӿڶ�����
	virtual void Release(void) = 0;
	//��ȡSOCKET���
	virtual SOCKET GetSocket(void) = 0;
};
//-----------------------------------------------------------------------------------------------------------------------------
//ͬ��ͨѶͨѶ����
class MBPSyncClientIO
{
public:
	//������IP��ַ������������˿ڣ����ӳ�ʱʱ�䣨���룩
	virtual int  Connect(const char * szIPAddr,unsigned int uiPort,unsigned int uiTimeOut) = 0;
	//�Ͽ������������
	virtual void Disconnect(void) = 0;
	//��������
	//���������ܺţ��������ݻ��壬�������ݳ��ȣ��Ƿ���Ҫѹ��
	virtual int  SendData(unsigned short usMessageNo,unsigned short usFunctionID,const char * lpData,unsigned int uiSize,bool bCompress) = 0;
	//���ʹ���
	virtual int  SendError(unsigned short usMessageNo,unsigned short usFunctionID,const char * lpErrorInfo) = 0;
	//��������
	//���������յ����ݵĹ��ܺţ��������ݻ��壬�������ݻ����С
	virtual int  RecvData(unsigned short * lpMessageNo,unsigned short * lpFunctionID,bool * lpErrorFlag,char * lpOutBuf,unsigned int uiOutSize) = 0;
	//�ͷ�
	virtual void Release(void) = 0;
	//��ȡSOCKET���
	virtual SOCKET GetSocket(void) = 0;
};
//-----------------------------------------------------------------------------------------------------------------------------
//��̬���ӿ�ӿ��������
//int  StartWork(const tagMBPClientIO_RunParam * lpParam);
//void EndWork(void);
//MBPClientCommIO_Api * CreateMBPClientCommIO_Api(void);
//MBPSyncClientIO * CreateMBPSyncClientIO(void);
//void GetStatus(tagMBPClientIO_Status * lpOut)
//.............................................................................................................................
//���÷���
//���ȵ���StartWork��������ͨѶģ�飬ע�⣺���Ҫʹ��SSL���ܣ�������֤��·��������д�������շǼ��ܷ�ʽ����ͨѶ
//Ȼ�����ʵ��Ӧ�ú���������������CreateMBPClientCommIO_Api�����첽ͨѶ���ӻ����CreateMBPSyncClientIO����ͬ��ͨѶ����
//����ͨѶ�����У����Ե���GetStatus��ȡ��ǰͨѶ��Ϣ����������������������������ȵ�
//��ҪֹͣͨѶʱ�����밴�����¹���
//	1�������д��������Ķ���ȫ������Release�����ͷ�
//	2������ɵ�1���Ļ����ϣ��ŵ���EndWork�ر�����ͨѶ����
//	ע�⣺��1���͵�2��˳���ܵߵ�
//-----------------------------------------------------------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------------------------------------------------------