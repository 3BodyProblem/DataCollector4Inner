#ifndef __DATA_COLLECTOR_H__
#define __DATA_COLLECTOR_H__


#pragma warning(disable: 4786)
#include <vector>
#include <string>
#include "Infrastructure/Lock.h"
#include "Connection/MBPClientCommIO.h"


extern	HMODULE						g_oModule;				///< ��ǰdllģ�����


/**
 * @class							LinkConfig
 * @brief							һ��������Ϣ
 * @date							2017/5/15
 * @author							barry
 */
class LinkConfig
{
public:
	LinkConfig();

	/**
	 * @brief						��ȡ����Դ��������
	 */
	bool							GetConfig( std::string& sIP, unsigned int& nPort );

public:
	std::vector<std::string>		m_vctSvrIP;				///< ��������ַ
	std::vector<unsigned int>		m_vctSvrPort;			///< �������˿�

protected:
	unsigned int					m_nDataPos;				///< ����λ��
};


/**
 * @class							Configuration
 * @brief							������Ϣ
 * @date							2017/5/15
 * @author							barry
 */
class Configuration
{
protected:
	Configuration();

public:
	/**
	 * @brief						��ȡ���ö���ĵ������ö���
	 */
	static Configuration&			GetConfig();

	/**
	 * @brief						����������
	 * @return						==0						�ɹ�
									!=						����
	 */
    int								Initialize();

	/**
	 * @brief						�ͷ���Դ
	 */
	void							Release();

public:
	/**
	 * @brief						���в���
	 */
	const tagMBPClientIO_RunParam&	GetRunParam() const;

	/**
	 * @brief						ȡ�ÿ�������Ŀ¼(���ļ���)
	 */
	const std::string&				GetDumpFolder() const;

	/**
	 * @brief						��ȡ���г�������·���ñ�
	 */
	LinkConfig&						GetHQConfList();

private:
	tagMBPClientIO_RunParam			m_tagClientRunParam;	///< API���в���
	std::string						m_sDumpFileFolder;		///< ��������·��(��Ҫ���ļ���)
	LinkConfig						m_oHQConfigList;		///< CTP������������������б�
};





#endif








