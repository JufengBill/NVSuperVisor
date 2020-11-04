#pragma once

class CRemoteDesktop
{
public:
	CRemoteDesktop();
	virtual ~CRemoteDesktop();

	int OpenRemoteDesktop(CString IpAddr);
	int CreateSyncPair(CString remoteHostName, CString projectName);
	int RunSync(CString remoteHostName, CString projectName);
	CString ExecuteCmd(CString str);
	TCHAR*  StringToChar(CString& str);
	BOOL ExecDosCmd(void);

private:
	CString iniFileName; //iniÎÄ¼þÂ·¾¶
	CString computerName;
};

