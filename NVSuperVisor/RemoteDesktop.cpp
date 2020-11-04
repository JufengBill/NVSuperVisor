#include "stdafx.h"
#include "RemoteDesktop.h"


CRemoteDesktop::CRemoteDesktop()
{
	//��ȡ��ǰ��Ӧ�ó���·������ȷ��INI�ļ��������ļ���
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



//MFCִ��CMD�������䷵����ϢԴ����
//MFCִ��CMD�������䷵��ֵ��
//ԭ�������ùܵ�����������һ������ִ��cmd��������䷵����Ϣ����ܵ��У��ٶ����ܵ��е�����ɡ�
//�������ҷ�װ��һ��������������Ҫִ�е�CMD����������֮���á�&���Ÿ����������ķ���ֵ����ִ������ķ�����Ϣ����ʽ��ΪCString��ʽ��

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

//�����õ�һ���ⲿ������void StringToChar(&str);
//��������������ǽ�CString��ʽ������ת��ΪTCHAR��ʽ��������Դ���룺

TCHAR* CRemoteDesktop::StringToChar(CString& str)
{
	int len = str.GetLength();
	TCHAR* tr = str.GetBuffer(len);
	str.ReleaseBuffer();
	return tr;
}

BOOL CRemoteDesktop::ExecDosCmd()
{
	#define EXECDOSCMD "synctoycmd -r Data2" //���Ի����������

	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		return FALSE;
	}
	char command[1024]; //����1K�������У������˰�
	strcpy(command, "Cmd.exe /C ");
	strcat(command, EXECDOSCMD);
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWrite; //�Ѵ������̵ı�׼��������ض��򵽹ܵ�����
	si.hStdOutput = hWrite; //�Ѵ������̵ı�׼����ض��򵽹ܵ�����
	si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	//�ؼ����裬CreateProcess�����������������MSDN
	if (!CreateProcess(NULL, command, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		CloseHandle(hWrite);
		CloseHandle(hRead);
		return FALSE;
	}

	CloseHandle(hWrite);
	char buffer[4096] = { 0 }; //��4K�Ŀռ����洢��������ݣ�ֻҪ������ʾ�ļ����ݣ�һ��������ǹ����ˡ�
	DWORD bytesRead;
	while (true)
	{
		if (ReadFile(hRead, buffer, 4095, &bytesRead, NULL) == NULL)
			break;
		//buffer�о���ִ�еĽ�������Ա��浽�ı���Ҳ����ֱ�����
		AfxMessageBox(buffer); //�����ǵ����Ի�����ʾ
	}
	CloseHandle(hRead);
	return TRUE;
}