#include "stdafx.h"
#include "RemoteDesktop.h"


CRemoteDesktop::CRemoteDesktop()
{
	//获取当前的应用程序路径名，确定INI文件的完整文件名
	CString strAppPath = _T("");
	GetModuleFileName(NULL, strAppPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	strAppPath.ReleaseBuffer();
	int nPos = strAppPath.ReverseFind('\\');
	strAppPath = strAppPath.Left(nPos);
	iniFileName.Format(_T("%s\\Custom.ini"), strAppPath);
	char strINIInfo[64] = { "" };
	GetPrivateProfileString(_T("Parameters"), _T("LocalComputeName"), _T(""), strINIInfo, 64, iniFileName);
	computerName = _T(strINIInfo);
}


CRemoteDesktop::~CRemoteDesktop()
{
}

int CRemoteDesktop::OpenRemoteDesktop(CString IpAddress)
{
	CString str_Temp = _T("");
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));

	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.lpVerb = _T("open");
	sei.lpFile = _T("c://program files//tightvnc//tvnviewer.exe");
	str_Temp.Format(_T("%s::5900"), IpAddress);
	sei.lpParameters = str_Temp;
	sei.nShow = SW_SHOWNORMAL;
	ShellExecuteEx(&sei);

	if ((int)sei.hInstApp > 32)
	{
		return 0;
	}
	else
	{
		return -1;
	}

}

int CRemoteDesktop::CreateSyncPair(CString remoteHostName, CString projectName)
{
		
	//char str_Content[64] = { "0" };
	//GetPrivateProfileString(_T("Parameters"), _T("ProjectFolder"), _T(""), str_Content, 64, iniFileName);
	

	CString str_Temp = _T("");
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));

	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.lpVerb = _T("open");
	sei.lpFile = _T("c://program files//synctoy 2.1//synctoy.exe");	

	str_Temp.Format(_T("-d(left=\\\\%s\\Projects\\%s,right=\\\\%s\\Projects\\%s,name=%s-%s,operation=echo,-excluded=*.prj)"), remoteHostName, projectName, computerName, projectName, remoteHostName, projectName);
	//str_Temp.Format(_T("-d(left=\\\\%s\\Projects,right=\\\\zhengyunchen\\Projects,name=%s,operation=echo,-excluded=*.prj)"), remoteHostName, remoteHostName);
	sei.lpParameters = str_Temp;
	sei.nShow = SW_SHOWNORMAL;
	ShellExecuteEx(&sei);

	if ((int)sei.hInstApp > 32)
	{
		//str_Temp.Format(_T("System%d"), nSys + 1);
		//::WritePrivateProfileStringA(str_Temp, _T("SyncPair"), _T("YES"), iniFileName);
		return 0;
	}
	else
	{
		//str_Temp.Format(_T("System%d"), nSys + 1);
		//::WritePrivateProfileStringA(str_Temp, _T("SyncPair"), _T("NO"), iniFileName);
		return -1;
	}
}

int CRemoteDesktop::RunSync(CString remoteHostName, CString projectName)
{

	CString str_Temp = _T("");
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));

	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.lpVerb = _T("open");
	sei.lpFile = _T("c://program files//synctoy 2.1//synctoycmd.exe");

	str_Temp.Format(_T("-R %s-%s"), remoteHostName, projectName);

	sei.lpParameters = str_Temp;
	sei.nShow = SW_SHOWNORMAL;
	ShellExecuteEx(&sei);

	if ((int)sei.hInstApp > 32)
	{
		return 0;
	}
	else
	{
		return -1;
	}

	//return 0;
}



//MFC执行CMD命令并获得其返回信息源代码
//MFC执行CMD命令并获得其返回值。
//原理是利用管道技术，创建一个进程执行cmd命令，并将其返回信息存入管道中，再读出管道中的命令即可。
//以下是我封装的一个函数，参数是要执行的CMD命令，多个命令之间用“&”号隔开，函数的返回值就是执行命令的返回信息。格式均为CString格式。

CString CRemoteDesktop::ExecuteCmd(CString str)
{
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		//MessageBox("Error on CreatePipe()!");
		return NULL;
	}
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	TCHAR* cmdline = StringToChar(str);
	if (!CreateProcess(NULL, cmdline, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		//MessageBox("Error on CreateProcess()!");
		return NULL;
	}
	CloseHandle(hWrite);

	char buffer[4096];
	memset(buffer, 0, 4096);
	CString output;
	DWORD byteRead;
	while (true)
	{
		if (ReadFile(hRead, buffer, 4095, &byteRead, NULL) == NULL)
		{
			break;
		}
		output += buffer;
	}
	return output;
}

//其中用到一个外部函数：void StringToChar(&str);
//这个函数的作用是讲CString格式的命令转化为TCHAR格式，以下是源代码：

TCHAR* CRemoteDesktop::StringToChar(CString& str)
{
	int len = str.GetLength();
	TCHAR* tr = str.GetBuffer(len);
	str.ReleaseBuffer();
	return tr;
}

BOOL CRemoteDesktop::ExecDosCmd()
{
	#define EXECDOSCMD "synctoycmd -r Data2" //可以换成你的命令

	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		return FALSE;
	}
	char command[1024]; //长达1K的命令行，够用了吧
	strcpy(command, "Cmd.exe /C ");
	strcat(command, EXECDOSCMD);
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWrite; //把创建进程的标准错误输出重定向到管道输入
	si.hStdOutput = hWrite; //把创建进程的标准输出重定向到管道输入
	si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	//关键步骤，CreateProcess函数参数意义请查阅MSDN
	if (!CreateProcess(NULL, command, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		CloseHandle(hWrite);
		CloseHandle(hRead);
		return FALSE;
	}

	CloseHandle(hWrite);
	char buffer[4096] = { 0 }; //用4K的空间来存储输出的内容，只要不是显示文件内容，一般情况下是够用了。
	DWORD bytesRead;
	while (true)
	{
		if (ReadFile(hRead, buffer, 4095, &bytesRead, NULL) == NULL)
			break;
		//buffer中就是执行的结果，可以保存到文本，也可以直接输出
		AfxMessageBox(buffer); //这里是弹出对话框显示
	}
	CloseHandle(hRead);
	return TRUE;
}