#pragma once

#include <vector> 
#include  "stdafx.h"


using namespace std;

class CCommonFunction
{
public:
	CCommonFunction();
	virtual ~CCommonFunction();

	//��ȡָ����չ�����ļ���
	//vector<CString> FindFiles(const CString dir, const CString ext);   // = _T("yuv")
	vector<std::string> FindFiles(const std::string & path);
};

