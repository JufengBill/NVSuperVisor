#include "stdafx.h"
#include "CommonFunction.h" 
#include <io.h>  
#include <string>  
#include <vector>

CCommonFunction::CCommonFunction()
{
}


CCommonFunction::~CCommonFunction()
{
}


//获取子文件的名称
vector<std::string> CCommonFunction::FindFiles(const std::string & path)
{
	
	vector<std::string> files;

	//文件句柄  
	long hFile = 0;
	//文件信息，_finddata_t需要io.h头文件  
	struct _finddata_t fileinfo;
	std::string p;
	std::string pValue;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					if (fileinfo.name[0] != '#')
						files.push_back(fileinfo.name);
				}
				
			}

		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);

		return files;
	}
}


