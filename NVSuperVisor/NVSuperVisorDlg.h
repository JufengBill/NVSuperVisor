
// NVSuperVisorDlg.h : 头文件
//
#if !defined(AFX_SENDDLG_H__B1FA7D63_8F13_43C1_896B_67F13D5733FC__INCLUDED_)
#define AFX_SENDDLG_H__B1FA7D63_8F13_43C1_896B_67F13D5733FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TcpClient.h"
#include "afxcmn.h"
#include "RemoteDesktop.h"
#include "afxwin.h"
#include "InputBox.h"
#include "WrkList.h"
#include "IniFileReadWrite.h"
#include "CommonFunction.h"

#define IDC_SYSTEMBTN 10000
#define NUMOFBTN      8


// CNVSuperVisorDlg 对话框
class CNVSuperVisorDlg : public CDialogEx
{
// 构造
public:
	CNVSuperVisorDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CNVSuperVisorDlg(void);

	CStatusBar m_bar;
	CFont *fa_Bar;

	void DispInfo(CString strInfo);
	void DispError(CString errMsg);

// 对话框数据
	enum { IDD = IDD_NVSUPERVISOR_DIALOG };

	CStatic	m_ctlCnnStatus;
	//CStatic	m_ctlInfo;
	DWORD	m_dwFilePackageSize;
	CString	m_strServerIp;
	int		m_nPort;
	CString	m_strFileName;
	CString	m_strMsg;
	CString m_strProjectName;   //当前的项目名称

	int     m_cursorType;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	int m_iFinishCount_OpenNVGate;
	int m_iFinishCount_SendFiles;
	int m_iFinishCount_LoadWorkbook;
	int m_iSuccessCount_LoadWorkbook;
	int m_iFinishCount_Stopped;
	int m_iFinishCount_Save;
	//int m_iFinishCount_Armed;
	//int m_iFinishCount_StartMonitor;

	int m_iFinishCount_AnalyzerStatus;
	int m_iFinishCount_FanStatus;
	CString nSelText;
	int nListCount;

	void SetCurrentProjectName(CString projectName);

private:
	CTcpClient *m_pTcpClient;
	CTcpClient *m_pTcpClient1;

	CTcpClient *m_pTcpClient_Generate;
	CTcpClient **m_pTcpClient_SendFiles;
	CTcpClient **m_pTcpClient_OpenNVGate;
	CTcpClient **m_pTcpClient_LoadWorkbook;
	CTcpClient **m_pTcpClient_States;
	CTcpClient **m_pTcpClient_Stop;
	CTcpClient **m_pTcpClient_CloseNVGate;
	CTcpClient **m_pTcpClient_Save;
	CTcpClient **m_pTcpClient_FanStatus;

	CRemoteDesktop *m_pRemoteDesktop;
	CIniFileReadWrite *m_pIniFileReadWrite;
	CCommonFunction *m_pCommonFuction;

	CString *m_pAnalyzerStatus;

	BOOL b_FinishSaving;

	CString str_ConfigurationName;
	CString str_CurrentCFGName;


	//NVMI *m_pNVGateMaster;
	int  numofSystem;
	int  totalChannel;
	CString *ipAddress;
	int *numofChannel;
	CString *m_pType;    //0为单台设备，1为Teamwork设备
	CString *m_pHardIpAddr;    //硬件的IP地址
	CString iniFileName;
	CString tempIniFileName;
	CString strAppPath;

	int m_nNumberOfMonitor;    //Monitor的数量
	CString *m_pMonitorIP;

	

	//static bool f_recvFinish;
	static void OnSocketConnection(void *pNotifyObj, SOCKET hSocket);
	static void OnSocketClose(void *pNotifyObj, SOCKET hSocket, EMSocketCloseReason Socket);
	static void OnOneNetMsg(void *pNotifyObj, char *Msg, int nMsgLen);
	static void OnSendFileSucc(void *pNotifyObj, char *szPathName);
	static void OnGetLocalInformationSucc(void *pNotifyObj);
	static void OnGetLocalInformationFail(void *pNotifyObj);
	static void OnOneMessage(void *pNotifyObj, CString Msg);
	static void OnErrorMessage(void *pNotifyObj, CString Msg);
	static void OnSendFileFail(void *pNotifyObj, char *szPathName, EMSendFileFailReason SendFileFailReason);
	static void OnSendFileProgress(void *pNotifyObj, UWORD nSentBytes, UWORD nTotalBytes);
	static void OnSetCursorType(void *pNotifyObj, int cursorType);
	static void OnSetAnalyzerStatus(void *pNotifyObj, int indexSystem, CString analyzerStatus, int indexStates);
	static void OnSetFinishCount(void *pNotifyObj, int iSource, int successFlag);
	static void OnExecuteGettingStatus(void *pNotifyObj, int indexSystem, CString analyzerStatus); 
	static void OnSetButtonStatus(void *pNotifyObj, int m_iID, BOOL m_bStatus);
	static void OnSetCurrentProjectName(void *pNotifyObj, CString projectName);

	CButton* NewMyButton(int nID, CRect rect, int nStyle);

	void SetButtonEnableDisable(void);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedReadgenerate();
	//afx_msg void OnBnClickedSendMsg();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);	
	afx_msg void OnBnClickedSendworkbook();
	afx_msg void OnBnClickedOperateremotenvgate();
	afx_msg void OnBnClickedOpennvgate();
	afx_msg void OnItemdblclickSystemList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOpenremotedesktop();
	afx_msg void OnBnClickedAlarmrecorder();
	afx_msg void OnBnClickedStartmonitor();
	afx_msg void OnBnClickedStartrecord();
	afx_msg void OnBnClickedStoprecord();
	CButton m_btnReadGenerate;
	CButton m_btnSendWKFiles;
	CButton m_btnOpenRemoteNVGate;
	CButton m_btnLoadWK;
	CButton m_btnArm;
	CButton m_btnStartMonitor;
	CButton m_btnStartRecord;
	CButton m_btnStopandSave;
	CListCtrl m_ctlSystemStatesList;
	CListCtrl m_ctlSystemList;
	CButton m_btnRunAutoZero;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnautozero();

	CButton* m_pSystemButton;
	CFont *fa;

	afx_msg void OnSystemButton(UINT uID);

	//afx_msg void OnBnClickedBtnsystem();
	CButton m_BtnExit;
	afx_msg void OnClose();
	afx_msg void OnBnClickedSynctoy();
	CString m_CfgName;
	CStatic m_cCfgName;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDDLG_H__B1FA7D63_8F13_43C1_896B_67F13D5733FC__INCLUDED_)