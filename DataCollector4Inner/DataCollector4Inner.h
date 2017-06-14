#ifndef __DATA_TABLE_PAINTER_H__
#define	__DATA_TABLE_PAINTER_H__


#include "Interface.h"
#include "Configuration.h"
#include "Infrastructure/Lock.h"
#include "Connection/Quotation.h"
#include "Infrastructure/Thread.h"


/**
 * @class						T_LOG_LEVEL
 * @brief						��־����
 * @author						barry
 */
enum T_LOG_LEVEL
{
	TLV_INFO = 0,
	TLV_WARN = 1,
	TLV_ERROR = 2,
	TLV_DETAIL = 3,
};


/**
 * @class						QuoCollector
 * @brief						���ʺ��������ݲɼ�ģ��������
 * @detail						��ģ���ʺ����κ��г����������ݽṹ;
								&
								��Ϊ����ģ�齫�����κ�ʱ�������״̬�����˳�ʼ������ʱ�Σ�
 * @date						2017/5/15
 * @author						barry
 */
class QuoCollector : public SimpleTask
{
protected:
	QuoCollector();

public:
	/**
	 * @brief					��ʼ�����ݲɼ���
	 * @detail					�������� + ������Ϣ�ص� + ������ϵ�ͨѶģ�� + �����Ͽ������߳�
	 * @param[in]				pIDataHandle				����ص��ӿ�
	 * @return					==0							��ʼ���ɹ�
								!=0							����
	 */
	int							Initialize( I_DataHandle* pIDataHandle );

	/**
	 * @brief					�ͷ���Դ
	 */
	void						Release();

public:
	/**
	 * @brief					������������������պ�����
	 * @return					==0							�ɹ�
								!=0							����
	 */
	int							RecoverQuotation();

	/**
	 * @brief					��ʱֹͣ�Զ��Զ���������
	 */
	void						Halt();

public:
	/**
	 * @brief					ȡ�òɼ�ģ��ĵ�ǰ״̬
 	 * @param[out]				pszStatusDesc				���س�״̬������
	 * @param[in,out]			nStrLen						�������������泤�ȣ������������Ч���ݳ���
	 * @return					����ģ�鵱ǰ״ֵ̬
	 */
	enum E_SS_Status			GetCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief					��ȡ�г����
	 */
	unsigned int				GetMarketID() const;

	/**
	 * @brief					��ȡ���ݲɼ�����ĵ�������
	 */
	static QuoCollector&		GetCollector();

	/**
	 * @brief					���ط�������ص��ӿڵ�ַ
	 * @return					����ص��ӿ�ָ���ַ
	 */
	I_DataHandle*				operator->();

protected:
	/**
	 * @brief					������(��ѭ�����Զ��Ͽ������ϼ�����Դ)
	 * @return					==0					�ɹ�
								!=0					ʧ��
	 */
	virtual int					Execute();

protected:
	I_DataHandle*				m_pCbDataHandle;			///< ����(����/��־�ص��ӿ�)
	MkQuotation					m_oQuotationData;			///< ʵʱ�������ݻỰ����
};





/**
 * @brief						DLL�����ӿ�
 * @author						barry
 * @date						2017/4/1
 */
extern "C"
{
	/**
	 * @brief								��ʼ�����ݲɼ�ģ��
	 */
	__declspec(dllexport) int __stdcall		Initialize( I_DataHandle* pIDataHandle );

	/**
	 * @brief								�ͷ����ݲɼ�ģ��
	 */
	__declspec(dllexport) void __stdcall	Release();

	/**
	 * @brief								���³�ʼ����������������
	 */
	__declspec(dllexport) int __stdcall		RecoverQuotation();

	/**
	 * @brief								��ʱ���ݲɼ�
	 */
	__declspec(dllexport) void __stdcall	HaltQuotation();

	/**
	 * @brief								��ȡģ��ĵ�ǰ״̬
	 * @param[out]							pszStatusDesc				���س�״̬������
	 * @param[in,out]						nStrLen						�������������泤�ȣ������������Ч���ݳ���
	 * @return								����ģ�鵱ǰ״ֵ̬
	 */
	__declspec(dllexport) int __stdcall		GetStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief								��ȡ�г����
	 */
	__declspec(dllexport) int __stdcall		GetMarketID();

	/**
	 * @brief								�Ƿ�Ϊ���鴫��Ĳɼ���
	 */
	__declspec(dllexport) bool __stdcall	IsProxy();

	/**
	 * @brief								��Ԫ���Ե�������
	 */
	__declspec(dllexport) void __stdcall	ExecuteUnitTest();
}




#endif





