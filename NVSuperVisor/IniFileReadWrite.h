#pragma once

class CIniFileReadWrite
{
public:
	// дini�ļ�, Ĭ���ļ��ڵ�ǰ��������Ŀ¼��, �ļ���Ϊset.ini
	BOOL WriteValue(LPCTSTR lpszSection, LPCTSTR lpszSectionKey, LPCTSTR lpszValue);
	BOOL WriteValue(LPCTSTR lpszSection, LPCTSTR lpszSectionKey, int iValue);
	BOOL WriteValue(LPCTSTR lpszSection, LPCTSTR lpszSectionKey, double dValue);
	//��ini�ļ�,Ĭ���ļ��ڵ�ǰ��������Ŀ¼��,�ļ���Ϊset.ini
	BOOL ReadValue(LPCTSTR lpszSection, LPCTSTR lpszSectionKey, CString &szValue);
	BOOL ReadValue(CString lpszSection, LPCTSTR lpszSectionKey, int &iValue);
	BOOL ReadValue(CString lpszSection, LPCTSTR lpszSectionKey, double &dValue);
	//�����ļ�·��
	void SetPathFileName(LPCTSTR lpszPathFileName);

	//��ȡ��ǰӦ�ó����·��Ŀ¼
	CString GetCurrentPath();


	//���캯��
	CIniFileReadWrite();
	virtual ~CIniFileReadWrite();

private:
	//�ļ�·�����ļ�
	CString m_sPathFileName;

};

