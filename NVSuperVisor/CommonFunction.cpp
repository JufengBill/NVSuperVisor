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


//��ȡ���ļ�������
vector<std::string> CCommonFunction::FindFiles(const std::string & path)
{
	
	vector<std::string> files;

	//�ļ����  
	long hFile = 0;
	//�ļ���Ϣ��_finddata_t��Ҫio.hͷ�ļ�  
	struct _finddata_t fileinfo;
	std::string p;
	std::string pValue;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//�����Ŀ¼,����֮  
			//�������,�����б�  
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


