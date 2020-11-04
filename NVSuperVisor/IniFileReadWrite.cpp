#include "stdafx.h"
#include "IniFileReadWrite.h"


BOOL CIniFileReadWrite::WriteValue(LPCTSTR lpszSection, LPCTSTR lpszSectionKey, LPCTSTR lpszValue)
{
	return WritePrivateProfileString(lpszSection, lpszSectionKey, lpszValue, m_sPathFileName);
}

BOOL CIniFileReadWrite::WriteValue(LPCTSTR lpszSection, LPCTSTR lpszSectionKey, int iValue)
{
	CString sValue;
	sValue.Format(_T("%d"), iValue);
	return WriteValue(lpszSection, lpszSectionKey, sValue);
}

BOOL CIniFileReadWrite::WriteValue(LPCTSTR lpszSection, LPCTSTR lpszSectionKey, double  dValue)
{
	CString sValue;
	sValue.Format(_T("%0.2f"), dValue);
	return WriteValue(lpszSection, lpszSectionKey, sValue);
}

BOOL CIniFileReadWrite::ReadValue(LPCTSTR lpszSection, LPCTSTR lpszSectionKey, CString  & szValue)
{
	char buff[1024];
	DWORD dwRead = GetPrivateProfileString(lpszSection, lpszSectionKey, NULL, buff, 1024, m_sPathFileName);
	if (dwRead == 0)
		return FALSE;
	szValue = _T(buff);
	return TRUE;
}

BOOL CIniFileReadWrite::ReadValue(CString lpszSection, LPCTSTR lpszSectionKey, int & iValue)
{
	char buff[1024];
	DWORD dwRead = GetPrivateProfileString(lpszSection, lpszSectionKey, NULL, buff, 1024, m_sPathFileName);
	if (dwRead == 0)
		return FALSE;
	int value = atoi(buff);
	iValue = value;
	return TRUE;
}

BOOL CIniFileReadWrite::ReadValue(CString lpszSection, LPCTSTR lpszSectionKey, double   & dValue)
{
	char buff[1024];
	DWORD dwRead = GetPrivateProfileString(lpszSection, lpszSectionKey, NULL, buff, 1024, m_sPathFileName);
	if (dwRead == 0)
		return FALSE;
	double value = atof((char*)buff);
	dValue = value;
	return TRUE;
}

// �����ļ�·�� C:a . 
void  CIniFileReadWrite::SetPathFileName(LPCTSTR lpszPathFileName)
{
	m_sPathFileName = lpszPathFileName;
}

CString CIniFileReadWrite::GetCurrentPath()
{
	TCHAR exeFullPath[MAX_PATH]; // MAX_PATH��API�ж����˰ɣ�������
	GetModuleFileName(NULL, exeFullPath, MAX_PATH);
	CString sPath;
	sPath = exeFullPath;
	int nPos;
	nPos = sPath.ReverseFind('\\');
	sPath = sPath.Left(nPos + 1);
	return sPath;
}

//��ʼ�ļ�ΪCUSTOM.INI�ļ�
CIniFileReadWrite::CIniFileReadWrite(void)
{
	//��ȡ��ǰӦ�ó����·��
	CString strPath = GetCurrentPath();
	//�ļ�·��
	strPath = strPath + _T("custom.ini");
	m_sPathFileName = strPath;
}

CIniFileReadWrite:: ~CIniFileReadWrite(void)
{

}
