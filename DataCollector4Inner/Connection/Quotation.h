#ifndef __CTP_SESSION_H__
#define __CTP_SESSION_H__


#pragma warning(disable:4786)
#include <set>
#include <string>
#include <stdexcept>
#include "DataDump.h"
#include "../Configuration.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/DateTime.h"


#pragma pack(1)


/**
 * @class							tagPackageHead
 * @brief							���ݰ��İ�ͷ�ṹ����
 * @author							barry
 */
typedef struct
{
	unsigned __int64				nSeqNo;				///< �������
	unsigned char					nMarketID;			///< �г����
	unsigned short					nMsgLength;			///< ���ݲ��ֳ���
	unsigned short					nMsgCount;			///< Message����
} tagPackageHead;


/**
 * @class							tagLoginData
 * @brief							��¼���ݿ�
 * @author							barry
 */
typedef struct
{
	char							pszActionKey[20];	///< ָ���ַ���: login:�����¼ success:��¼�ɹ� failure:��¼ʧ��
	char							pszUserName[32];	///< �û���
	char							pszPassword[64];	///< ����
	unsigned int					nReqDBSerialNo;		///< ���������ˮ��֮�����������
	char							Reserve[1024];		///< ����
} tagCommonLoginData_LF299;


#pragma pack()


/**
 * @class			WorkStatus
 * @brief			����״̬����
 * @author			barry
 */
class WorkStatus
{
public:
	/**
	 * @brief				Ӧ״ֵ̬ӳ���״̬�ַ���
	 */
	static	std::string&	CastStatusStr( enum E_SS_Status eStatus );

public:
	/**
	 * @brief			����
	 * @param			eMkID			�г����
	 */
	WorkStatus();
	WorkStatus( const WorkStatus& refStatus );

	/**
	 * @brief			��ֵ����
						ÿ��ֵ�仯������¼��־
	 */
	WorkStatus&			operator= ( enum E_SS_Status eWorkStatus );

	/**
	 * @brief			����ת����
	 */
	operator			enum E_SS_Status();

private:
	CriticalObject		m_oLock;				///< �ٽ�������
	enum E_SS_Status	m_eWorkStatus;			///< ���鹤��״̬
};


/**
 * @class			MkQuotation
 * @brief			�Ự�������
 * @detail			��װ�������Ʒ�ڻ���Ȩ���г��ĳ�ʼ����������Ƶȷ���ķ���
 * @author			barry
 */
class MkQuotation : public MBPClientCommIO_Spi
{
public:
	MkQuotation();
	~MkQuotation();

public:///< �����ӿ�
	/**
	 * @brief				��ʼ��ctp����ӿ�
	 * @return				>=0			�ɹ�
							<0			����
	 * @note				������������������У�ֻ������ʱ��ʵ�ĵ���һ��
	 */
	int						Activate() throw(std::runtime_error);

	/**
	 * @brief				�ͷ�ctp����ӿ�
	 */
	int						Destroy();

public:///< �����ȡ�ӿ�
	/**
	 * @brief				������������������պ�����
	 * @return				==0							�ɹ�
							!=0							����
	 */
	int						Connect2Server();

	/**
	 * @brief				�Ͽ�����
	 */
	void					CloseConnection();

public:///< ���ܺ���
	/**
	 * @brief				��ȡ�Ự״̬��Ϣ
	 */
	WorkStatus&				GetWorkStatus();

	/**
	 * @brief				��ȡ�г����
	 */
	unsigned int			GetMarketID() const;

	/**
	 * @brief				��ȡ�˿ڷ�װ����
	 */
	MBPClientCommIO&		GetCommIO();

protected:///< CThostFtdcMdSpi�Ļص��ӿ�
	/**
	 * @brief				���ӳɹ���Ϣ�ص�
	 */
	virtual void			OnConnectSuc();

	/**
	 * @brief				����ʧ����Ϣ�ص�
	 */
	virtual void			OnConnectFal();

	/**
	 * @brief				���ӶϿ���Ϣ�ص�
	 */
	virtual void			OnDisconnect();

	/**
	 * @brief				����API��������Ϣ�ص�
	 */
	virtual void			OnDestory();

	/**
	 * @brief				�յ�������Ϣ��Ӧ����
	 * @param[in]			usMessageNo				��ϢID
	 * @param[in]			usFunctionID			���ܺ�
	 * @param[in]			bErrorFlag				�����ʶ
	 * @param[in]			lpData					�յ������ݻ����ַ
	 * @param[in]			uiSize					�յ������ݴ�С
	 * @return				����false��ʾ�������ݴ��󣬶Ͽ�����
	 */
	virtual bool			OnRecvData( unsigned short usMessageNo, unsigned short usFunctionID, bool bErrorFlag, const char* lpData, unsigned int uiSize );

protected:///< �յ����������ݴ�����
	/** 
	 * @brief				ˢ���ݵ����ͻ���
 	 * @param[in]			usMessageNo				��ϢID
	 * @param[in]			usFunctionID			���ܺ�
	 * @param[in]			lpData					�յ������ݻ����ַ
	 * @param[in]			uiSize					�յ������ݴ�С
	 * @return				����false��ʾ�������ݴ��󣬶Ͽ�����
	 */
	bool					OnQuotation( unsigned short usMessageNo, unsigned short usFunctionID, const char* lpData, unsigned int uiSize );

	/**
	 * @brief				�����µ�������¶�������
	 * @return				>=0						�ɹ�
	 */
	int						PrepareDumpFile();

	/**
	 * @brief				���͵�¼����������ݰ�
	 * @return				true					���͵�¼�ɹ�
	 */
	bool					SendLoginPkg();

private:
	MBPClientCommIO_Api*	m_pCommIOApi;			///< ͨѶ������ָ��
	MBPClientCommIO			m_oCommIO;				///< ͨѶ������

private:
	unsigned int			m_nMarketID;			///< �г����
	WorkStatus				m_oWorkStatus;			///< ����״̬
	MemoDumper<char>		m_oQuotDumper;			///< �������̶���
};




#endif





