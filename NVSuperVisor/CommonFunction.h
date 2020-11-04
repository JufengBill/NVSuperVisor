#pragma once

#include <vector> 
#include  "stdafx.h"


using namespace std;

class CCommonFunction
{
public:
	CCommonFunction();
	virtual ~CCommonFunction();

	//获取指定扩展名的文件名
	//vector<CString> FindFiles(const CString dir, const CString ext);   // = _T("yuv")
	vector<std::string> FindFiles(const std::string & path);
};

