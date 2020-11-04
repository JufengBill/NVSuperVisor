#pragma once

class CIniFileReadWrite
{
public:
	// 写ini文件, 默认文件在当前程序运行目录下, 文件名为set.ini
	BOOL WriteValue(LPCTSTR lpszSection, LPCTSTR lpszSectionKey, LPCTSTR lpszValue);
	BOOL WriteValue(LPCTSTR lpszSection, LPCTSTR lpszSectionKey, int iValue);
	BOOL WriteValue(LPCTSTR lpszSection, LPCTSTR lpszSectionKey, double dValue);
	//读ini文件,默认文件在当前程序运行目录下,文件名为set.ini
	BOOL ReadValue(LPCTSTR lpszSection, LPCTSTR lpszSectionKey, CString &szValue);
	BOOL ReadValue(CString lpszSection, LPCTSTR lpszSectionKey, int &iValue);
	BOOL ReadValue(CString lpszSection, LPCTSTR lpszSectionKey, double &dValue);
	//更改文件路径
	void SetPathFileName(LPCTSTR lpszPathFileName);

	//获取当前应用程序的路径目录
	CString GetCurrentPath();


	//构造函数
	CIniFileReadWrite();
	virtual ~CIniFileReadWrite();

private:
	//文件路径及文件
	CString m_sPathFileName;

};

