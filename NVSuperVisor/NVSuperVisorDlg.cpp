
// NVSuperVisorDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "NVSuperVisor.h"
#include "NVSuperVisorDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT BASED_CODE indicators[] =
{
	ID_INDICATOR_NISH,
	ID_INDICATOR_TIME
};


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CNVSuperVisorDlg 对话框



CNVSuperVisorDlg::CNVSuperVisorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNVSuperVisorDlg::IDD, pParent)
	, m_CfgName(_T(""))
{
	m_dwFilePackageSize = 0;
	m_strServerIp = _T("");
	m_nPort = 0;
	m_strFileName = _T("");
	m_strMsg = _T("");
	m_cursorType = 0;
	
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pTcpClient = new CTcpClient(this);
	m_pTcpClient1 = new CTcpClient(this);

	m_pTcpClient_Generate = new CTcpClient(this);

	//m_pTcpClient_States = new CTcpClient(this);
	m_pRemoteDesktop = new CRemoteDesktop();

	m_iFinishCount_OpenNVGate = 0;
	m_iFinishCount_SendFiles = 0;
	m_iFinishCount_LoadWorkbook = 0;
	m_iSuccessCount_LoadWorkbook = 0;
	m_iFinishCount_AnalyzerStatus = 0;
	m_iFinishCount_Stopped = 0;
	m_iFinishCount_Save = 0;
	m_iFinishCount_FanStatus = 0;

	m_pIniFileReadWrite = new CIniFileReadWrite();
	m_pCommonFuction = new CCommonFunction();

	fa = new CFont;
	fa_Bar = new CFont;

	b_FinishSaving = FALSE;

	//m_iFinishCount_Armed = 0;
	//m_iFinishCount_StartMonitor = 0;

	strAppPath = m_pIniFileReadWrite->GetCurrentPath();

	iniFileName = strAppPath + _T("custom.ini");
	tempIniFileName = strAppPath + _T("Tempsetting.ini");
}

CNVSuperVisorDlg::~CNVSuperVisorDlg(void)
{
	delete m_pTcpClient;
	delete m_pTcpClient1;

	delete m_pTcpClient_Generate;

	//delete m_pTcpClient_States;
	delete[] numofChannel;
	delete[] ipAddress;
	delete m_pRemoteDesktop;
	delete[] m_pType;
	delete[] m_pHardIpAddr;
	delete[] m_pMonitorIP;

	delete[] m_pAnalyzerStatus;
	//delete[] (*m_pTcpClient_Stop);
	for (int i = 0; i < numofSystem; i++)
	{
		if (m_pTcpClient_Stop[i] != NULL)
		{
			delete m_pTcpClient_Stop[i];
			m_pTcpClient_Stop[i] = NULL;
		}	

		if (m_pTcpClient_States[i] != NULL)
		{
			delete m_pTcpClient_States[i];
			m_pTcpClient_States[i] = NULL;
		}

		

		if (m_pTcpClient_OpenNVGate[i] != NULL)
		{
			delete m_pTcpClient_OpenNVGate[i];
			m_pTcpClient_OpenNVGate[i] = NULL;
		}

		if (m_pTcpClient_LoadWorkbook[i] != NULL)
		{
			delete m_pTcpClient_LoadWorkbook[i];
			m_pTcpClient_LoadWorkbook[i] = NULL;
		}

		if (m_pTcpClient_CloseNVGate[i] != NULL)
		{
			delete m_pTcpClient_CloseNVGate[i];
			m_pTcpClient_CloseNVGate[i] = NULL;
		}

		if (m_pTcpClient_Save[i] != NULL)
		{
			delete m_pTcpClient_Save[i];
			m_pTcpClient_Save[i] = NULL;
		}

		if (m_pTcpClient_FanStatus[i] != NULL)
		{
			delete m_pTcpClient_FanStatus[i];
			m_pTcpClient_FanStatus[i] = NULL;
		}
	}

	for (int i = 0; i < numofSystem + m_nNumberOfMonitor; i++)
	{
		if (m_pTcpClient_SendFiles[i] != NULL)
		{
			delete m_pTcpClient_SendFiles[i];
			m_pTcpClient_SendFiles[i] = NULL;
		}
	}
	
	delete[] m_pTcpClient_States;
	delete[] m_pTcpClient_Stop;
	delete[] m_pTcpClient_SendFiles;
	delete[] m_pTcpClient_OpenNVGate;
	delete[] m_pTcpClient_LoadWorkbook;
	delete[] m_pTcpClient_CloseNVGate;
	delete[] m_pTcpClient_Save;
	delete[] m_pTcpClient_FanStatus;

	delete m_pIniFileReadWrite;
	delete m_pCommonFuction;

	delete fa;
	delete fa_Bar;
	delete[] m_pSystemButton;

}

void CNVSuperVisorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_INFO, m_ctlInfo);
	DDX_Control(pDX, IDC_CONN_STATUS, m_ctlCnnStatus);
	//DDX_Text(pDX, IDC_MSG, m_strMsg);
	DDX_Control(pDX, IDC_SYSTEM_LIST, m_ctlSystemList);
	DDX_Control(pDX, IDC_READGENERATE, m_btnReadGenerate);
	DDX_Control(pDX, IDC_SENDWORKBOOK, m_btnSendWKFiles);
	DDX_Control(pDX, IDC_OPENNVGATE, m_btnOpenRemoteNVGate);
	DDX_Control(pDX, IDC_OPERATEREMOTENVGATE, m_btnLoadWK);
	DDX_Control(pDX, IDC_ALARMRECORDER, m_btnArm);
	DDX_Control(pDX, IDC_STARTMONITOR, m_btnStartMonitor);
	DDX_Control(pDX, IDC_STARTRECORD, m_btnStartRecord);
	DDX_Control(pDX, IDC_STOPRECORD, m_btnStopandSave);
	DDX_Control(pDX, IDC_SYSTEMSTATESLIST, m_ctlSystemStatesList);
	DDX_Control(pDX, IDC_BTNAUTOZERO, m_btnRunAutoZero);

	DDX_Control(pDX, IDOK, m_BtnExit);
	DDX_Text(pDX, IDC_CFGNAME, m_CfgName);
	DDX_Control(pDX, IDC_CFGNAME, m_cCfgName);
}

BEGIN_MESSAGE_MAP(CNVSuperVisorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_READGENERATE, &CNVSuperVisorDlg::OnBnClickedReadgenerate)
	//ON_BN_CLICKED(IDC_SEND_MSG, &CNVSuperVisorDlg::OnBnClickedSendMsg)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_SENDWORKBOOK, &CNVSuperVisorDlg::OnBnClickedSendworkbook)
	ON_BN_CLICKED(IDC_OPERATEREMOTENVGATE, &CNVSuperVisorDlg::OnBnClickedOperateremotenvgate)
	ON_BN_CLICKED(IDC_OPENNVGATE, &CNVSuperVisorDlg::OnBnClickedOpennvgate)
	//ON_NOTIFY(HDN_ITEMDBLCLICK, 0, &CNVSuperVisorDlg::OnItemdblclickSystemList)
	//ON_BN_CLICKED(IDC_OPENREMOTEDESKTOP, &CNVSuperVisorDlg::OnBnClickedOpenremotedesktop)
	ON_BN_CLICKED(IDC_ALARMRECORDER, &CNVSuperVisorDlg::OnBnClickedAlarmrecorder)
	ON_BN_CLICKED(IDC_STARTMONITOR, &CNVSuperVisorDlg::OnBnClickedStartmonitor)
	ON_BN_CLICKED(IDC_STARTRECORD, &CNVSuperVisorDlg::OnBnClickedStartrecord)
	ON_BN_CLICKED(IDC_STOPRECORD, &CNVSuperVisorDlg::OnBnClickedStoprecord)
	ON_BN_CLICKED(IDOK, &CNVSuperVisorDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTNAUTOZERO, &CNVSuperVisorDlg::OnBnClickedBtnautozero)

	//{{AFX_MSG_MAP(CTextEditorView)
	ON_COMMAND_RANGE(IDC_SYSTEMBTN, IDC_SYSTEMBTN + NUMOFBTN - 1, OnSystemButton)
	
	//}}AFX_MSG_MAP

	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_SYNCTOY, &CNVSuperVisorDlg::OnBnClickedSynctoy)
END_MESSAGE_MAP()


// CNVSuperVisorDlg 消息处理程序

BOOL CNVSuperVisorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	m_bar.Create(this);//创建状态栏
	m_bar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT)); //设置状态栏数目

	CRect rect;
	GetClientRect(&rect);

	fa_Bar->CreatePointFont(110, "Microsoft Sans Serif");

	//设置各栏长度
	m_bar.SetPaneInfo(0, ID_INDICATOR_NISH, SBPS_NORMAL, rect.Width()*0.8);
	m_bar.SetPaneInfo(1, ID_INDICATOR_TIME, SBPS_STRETCH, 0);
	m_bar.SetFont(fa_Bar);

	SetTimer(100, 1000, NULL);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, ID_INDICATOR_TIME);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	m_strServerIp = "127.0.0.1";
	m_nPort = 8000;
	m_dwFilePackageSize = 1024;
	m_strFileName = "c:\\win10.iso";
	UpdateData(FALSE);

	//::EnableMenuItem(::GetSystemMenu(this->m_hWnd, false), SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);//forbid close

	//m_pTcpClient->SetProgressTimeInterval(100);
	//m_pTcpClient->SetOnSocketConnect(OnSocketConnection);
	//m_pTcpClient->SetOnSocketClose(OnSocketClose);
	//m_pTcpClient->SetOnOneNetMsg(OnOneNetMsg);
	//m_pTcpClient->SetOnOneMsg(OnOneMessage);
	//m_pTcpClient->SetOnSendFileSucc(OnSendFileSucc);
	//m_pTcpClient->SetOnSendFileFail(OnSendFileFail);
	//m_pTcpClient->SetOnSendFileProgress(OnSendFileProgress);
	//m_pTcpClient->SetOnSetCursorType(OnSetCursorType);
	//m_pTcpClient->SetOnButtonStatus(OnSetButtonStatus);
	//m_pTcpClient->SetOnAnalyzerStatus(OnSetAnalyzerStatus);

	//m_pTcpClient1->SetProgressTimeInterval(100);
	//m_pTcpClient1->SetOnSocketConnect(OnSocketConnection);
	//m_pTcpClient1->SetOnSocketClose(OnSocketClose);
	//m_pTcpClient1->SetOnOneNetMsg(OnOneNetMsg);
	//m_pTcpClient1->SetOnOneMsg(OnOneMessage);
	//m_pTcpClient1->SetOnSendFileSucc(OnSendFileSucc);
	//m_pTcpClient1->SetOnSendFileFail(OnSendFileFail);
	//m_pTcpClient1->SetOnSendFileProgress(OnSendFileProgress);
	//m_pTcpClient1->SetOnSetCursorType(OnSetCursorType);
	//m_pTcpClient1->SetOnButtonStatus(OnSetButtonStatus);
	//m_pTcpClient1->SetOnAnalyzerStatus(OnSetAnalyzerStatus);

	m_pTcpClient_Generate->SetOnSocketConnect(OnSocketConnection);
	m_pTcpClient_Generate->SetOnSocketClose(OnSocketClose);
	m_pTcpClient_Generate->SetOnOneNetMsg(OnOneNetMsg);
	m_pTcpClient_Generate->SetOnOneMsg(OnOneMessage);
	m_pTcpClient_Generate->SetOnSetCursorType(OnSetCursorType);
	m_pTcpClient_Generate->SetOnButtonStatus(OnSetButtonStatus);
	m_pTcpClient_Generate->SetOnAnalyzerStatus(OnSetAnalyzerStatus);
	m_pTcpClient_Generate->setOnFinishCount(OnSetFinishCount);

	CRect wndRect1;
	//CRect wndRect2;

	/****首先看对话框的******/
	this->GetWindowRect(&wndRect1); //PrintRect("Dialog:GetWindowRect", wndRectScreen1);
	//this->ScreenToClient(&wndRect1); //PrintRect("Dialog:ScreenToClient", wndRectClient1);

	//this->GetClientRect(&wndRect2); //PrintRect("Dialog:GetClientRect", wndRectClient2);
	//this->ClientToScreen(&wndRect2); //PrintRect("Dialog:ClientToScreen", wndRectScreen2);


	LONG lStyle;
	lStyle = GetWindowLong(m_ctlSystemList.m_hWnd, GWL_STYLE);//获取当前窗口style
	lStyle &= ~LVS_TYPEMASK; //清除显示方式位
	lStyle |= LVS_REPORT; //设置style
	SetWindowLong(m_ctlSystemList.m_hWnd, GWL_STYLE, lStyle);//设置style

	DWORD dwStyle = m_ctlSystemList.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
	//dwStyle |= LVS_EX_CHECKBOXES;//item前生成checkbox控件
	m_ctlSystemList.SetExtendedStyle(dwStyle); //设置扩展风格

	m_ctlSystemList.InsertColumn(0, _T("Item"), LVCFMT_LEFT, wndRect1.right * 0.1);
	m_ctlSystemList.InsertColumn(1, _T("System Name"), LVCFMT_LEFT, wndRect1.right * 0.1);
	m_ctlSystemList.InsertColumn(2, _T("System Type"), LVCFMT_LEFT, wndRect1.right * 0.115);
	m_ctlSystemList.InsertColumn(3, _T("Contoller IP Address"), LVCFMT_LEFT, wndRect1.right * 0.15);
	m_ctlSystemList.InsertColumn(4, _T("Hardware IP Address"), LVCFMT_LEFT, wndRect1.right * 0.2);
	m_ctlSystemList.InsertColumn(5, _T("Number of Channels"), LVCFMT_LEFT, wndRect1.right * 0.125);
	m_ctlSystemList.InsertColumn(6, _T("Chain"), LVCFMT_LEFT, wndRect1.right * 0.1);
	//m_ctlSystemList.InsertColumn(7, _T("Recorder Status"), LVCFMT_LEFT, 200);


	lStyle = GetWindowLong(m_ctlSystemStatesList.m_hWnd, GWL_STYLE);//获取当前窗口style
	lStyle &= ~LVS_TYPEMASK; //清除显示方式位
	lStyle |= LVS_REPORT; //设置style
	SetWindowLong(m_ctlSystemStatesList.m_hWnd, GWL_STYLE, lStyle);//设置style
	
	dwStyle = m_ctlSystemStatesList.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
	//dwStyle |= LVS_EX_CHECKBOXES;//item前生成checkbox控件
	m_ctlSystemStatesList.SetExtendedStyle(dwStyle); //设置扩展风格

	m_ctlSystemStatesList.InsertColumn(0, _T("Item"), LVCFMT_LEFT, wndRect1.right * 0.1);
	m_ctlSystemStatesList.InsertColumn(1, _T("Recorder States"), LVCFMT_LEFT, wndRect1.right * 0.1);
	m_ctlSystemStatesList.InsertColumn(2, _T("Recorder Duration"), LVCFMT_LEFT, wndRect1.right * 0.115);
	m_ctlSystemStatesList.InsertColumn(3, _T("Temperature"), LVCFMT_LEFT, wndRect1.right * 0.1);
	
	CString str_IniInfo = _T("");
	CString str_SectionName = _T("");
	char str_Content[64] = { "0" };

	//获取Monitor的数量
	m_pIniFileReadWrite->SetPathFileName(strAppPath + _T("custom.ini"));
	m_pIniFileReadWrite->ReadValue(_T("Monitor"), _T("Number"), m_nNumberOfMonitor);
	//GetPrivateProfileString(_T("Monitor"), _T("Number"), _T("0"), str_Content, 64, iniFileName);
	//m_nNumberOfMonitor = atoi(str_Content);

	m_pMonitorIP = new CString[m_nNumberOfMonitor];

	//获取所有Monitor的IP地址
	for (int i = 0; i < m_nNumberOfMonitor; i++)
	{
		str_SectionName.Format(_T("Monitor%d"), i + 1);
		m_pIniFileReadWrite->ReadValue(_T("Monitor"), str_SectionName, m_pMonitorIP[i]);
	}

	//获取主机数量
	m_pIniFileReadWrite->ReadValue(_T("Parameters"), _T("NumberOfSystem"), numofSystem);
	numofChannel = (int*)malloc(sizeof(int)* numofSystem);

	ipAddress = new CString[numofSystem];
	m_pType = new CString[numofSystem];
	m_pHardIpAddr = new CString[numofSystem];
	m_pAnalyzerStatus = new CString[numofSystem];

	fa->CreatePointFont(85, "Microsoft Sans Serif");

	int a = 0.031 * wndRect1.right;
	int b = 0.81 * wndRect1.bottom;
	int l = 0.034 * wndRect1.right;
	int m = 0.087 * wndRect1.right;
	int n = 0.087 * wndRect1.bottom;

	m_pSystemButton = new CButton[numofSystem];
	DWORD dwStyleB = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
	for (int i = 0; i < numofSystem; i++)
	{
		CString str_Temp = _T("");
		str_Temp.Format(_T("System #%d"), i + 1);
		m_pSystemButton[i].Create(str_Temp, dwStyleB, CRect(a + m * i + l * i, b, a + m * i + l * i + m, b + n), this, IDC_SYSTEMBTN + i);
		m_pSystemButton[i].SetFont(fa);
		m_pSystemButton[i].ShowWindow(SW_SHOW);
	}

	int nRow = 0;
	CString str_IniValue = _T("");
	for (int i = 0; i < numofSystem; i++)
	{

		char str[10];
		sprintf(str, "System #%d", i + 1);
		nRow = m_ctlSystemList.InsertItem(i, str);//插入行
		nRow = m_ctlSystemStatesList.InsertItem(i, str);//插入行

		m_ctlSystemStatesList.SetItemText(nRow, 1, _T("Unknown"));//设
		m_pAnalyzerStatus[i] = _T("Unknown");
		m_ctlSystemStatesList.SetItemText(nRow, 2, _T("0.0 s"));//?
		m_ctlSystemStatesList.SetItemText(nRow, 3, _T("0 °C"));//?

		str_SectionName.Format(_T("System%d"), i + 1);
		
		m_pIniFileReadWrite->ReadValue(str_SectionName, _T("Name"), str_IniValue);
		m_ctlSystemList.SetItemText(nRow, 1, str_IniValue);//设置

		m_pIniFileReadWrite->ReadValue(str_SectionName, _T("Type"), m_pType[i]);
		//m_pType[i] = _T(str_Content);
		if (m_pType[i] == "T")
		{
			m_ctlSystemList.SetItemText(nRow, 2, _T("Teamwork"));//设置
		}
		else if (m_pType[i] == "S")
		{
			
			m_ctlSystemList.SetItemText(nRow, 2, _T("Stand alone"));//设置
		}
		else if (m_pType[i] == "O")
			m_ctlSystemList.SetItemText(nRow, 2, _T("Office"));//设置
		else
		{
			MessageBox(_T("System Type is incorrect, please check custom.ini file."), _T("NVSuperVisor"), MB_ICONERROR | MB_OK);
			EndDialog(IDCANCEL);
			return FALSE;
		}
		
		//GetPrivateProfileString(str_SectionName, _T("IPAddress"), _T(""), str_Content, 64, iniFileName);
		//ipAddress[i] = _T(str_Content);
		m_pIniFileReadWrite->ReadValue(str_SectionName, _T("IPAddress"), ipAddress[i]);
		m_ctlSystemList.SetItemText(nRow, 3, ipAddress[i]);//设置数据

		//GetPrivateProfileString(str_SectionName, _T("HardIpAddr"), _T(""), str_Content, 64, iniFileName);
		//m_pHardIpAddr[i] = _T(str_Content);
		m_pIniFileReadWrite->ReadValue(str_SectionName, _T("HardIpAddr"), m_pHardIpAddr[i]);
		m_ctlSystemList.SetItemText(nRow, 4, m_pHardIpAddr[i]);//设置数据

		//GetPrivateProfileString(str_SectionName, _T("NumberOfChannel"), _T(""), str_Content, 64, iniFileName);
		//numofChannel[i] = atoi(str_Content);
		m_pIniFileReadWrite->ReadValue(str_SectionName, _T("NumberOfChannel"), numofChannel[i]);
		str_IniValue.Format(_T("%d"), numofChannel[i]);
		m_ctlSystemList.SetItemText(nRow, 5, str_IniValue);//设置数据

		if (i == 0)
		{
			m_ctlSystemList.SetItemText(nRow, 6, _T("Master"));//设
		}
		else
		{
			m_ctlSystemList.SetItemText(nRow, 6, _T("Slave"));//?
		}
	}
	
	m_pIniFileReadWrite->ReadValue(_T("Parameters"), _T("TotalChannels"), totalChannel);

	////开启SYNCPAIR
	//for (int i = 0; i < numofSystem; i++)
	//{
	//	str_SectionName.Format(_T("System%d"), i + 1);
	//	GetPrivateProfileString(str_SectionName, _T("SyncPair"), _T(""), str_Content, 64, iniFileName);
	//	CString str_PairStatus = _T(str_Content);
	//	if (str_PairStatus == _T("NO"))
	//	{
	//		m_pRemoteDesktop->CreateSyncPair(ipAddress[i], i);
	//		Sleep(3000);
	//	}
	//}

	//新建类
	m_pTcpClient_Stop = (CTcpClient**)malloc(sizeof(CTcpClient)*numofSystem);
	m_pTcpClient_States = (CTcpClient**)malloc(sizeof(CTcpClient)*numofSystem);
	m_pTcpClient_SendFiles = (CTcpClient**)malloc(sizeof(CTcpClient)*(numofSystem+m_nNumberOfMonitor));
	m_pTcpClient_OpenNVGate = (CTcpClient**)malloc(sizeof(CTcpClient)*numofSystem);
	m_pTcpClient_LoadWorkbook = (CTcpClient**)malloc(sizeof(CTcpClient)*numofSystem);
	m_pTcpClient_CloseNVGate = (CTcpClient**)malloc(sizeof(CTcpClient)*numofSystem);
	m_pTcpClient_Save = (CTcpClient**)malloc(sizeof(CTcpClient)*numofSystem);
	m_pTcpClient_FanStatus = (CTcpClient**)malloc(sizeof(CTcpClient)*numofSystem);

	for (int i = 0; i < numofSystem; i++)
	{
		
		*(m_pTcpClient_Stop + i) = new CTcpClient(this);
		m_pTcpClient_Stop[i]->SetOnSocketConnect(OnSocketConnection);
		m_pTcpClient_Stop[i]->SetOnSocketClose(OnSocketClose);
		m_pTcpClient_Stop[i]->SetOnOneNetMsg(OnOneNetMsg);
		m_pTcpClient_Stop[i]->SetOnOneMsg(OnOneMessage);
		m_pTcpClient_Stop[i]->SetOnSetCursorType(OnSetCursorType);
		m_pTcpClient_Stop[i]->SetOnButtonStatus(OnSetButtonStatus);
		m_pTcpClient_Stop[i]->SetOnAnalyzerStatus(OnSetAnalyzerStatus);
		m_pTcpClient_Stop[i]->setOnFinishCount(OnSetFinishCount);

		*(m_pTcpClient_Save + i) = new CTcpClient(this);
		m_pTcpClient_Save[i]->SetOnSocketConnect(OnSocketConnection);
		m_pTcpClient_Save[i]->SetOnSocketClose(OnSocketClose);
		m_pTcpClient_Save[i]->SetOnOneNetMsg(OnOneNetMsg);
		m_pTcpClient_Save[i]->SetOnOneMsg(OnOneMessage);
		m_pTcpClient_Save[i]->SetOnSetCursorType(OnSetCursorType);
		m_pTcpClient_Save[i]->SetOnButtonStatus(OnSetButtonStatus);
		m_pTcpClient_Save[i]->SetOnAnalyzerStatus(OnSetAnalyzerStatus);
		m_pTcpClient_Save[i]->setOnFinishCount(OnSetFinishCount);
		

		//分析仪的状态参数的TCPCLIENT类实例
		*(m_pTcpClient_States + i) = new CTcpClient(this);
		m_pTcpClient_States[i]->SetOnSocketConnect(OnSocketConnection);
		m_pTcpClient_States[i]->SetOnSocketClose(OnSocketClose);
		m_pTcpClient_States[i]->SetOnOneNetMsg(OnOneNetMsg);
		m_pTcpClient_States[i]->SetOnOneMsg(OnOneMessage);
		m_pTcpClient_States[i]->SetOnSetCursorType(OnSetCursorType);
		m_pTcpClient_States[i]->SetOnButtonStatus(OnSetButtonStatus);
		m_pTcpClient_States[i]->SetOnAnalyzerStatus(OnSetAnalyzerStatus);
		m_pTcpClient_States[i]->SetOnExeGetAnalyzerStatus(OnExecuteGettingStatus);
		//m_pTcpClient_States[i]->setOnFinishCount(OnSetFinishCount);

		//打开远程NVGate的TCPCLIENT类实例
		*(m_pTcpClient_OpenNVGate + i) = new CTcpClient(this);
		m_pTcpClient_OpenNVGate[i]->SetOnSocketConnect(OnSocketConnection);
		m_pTcpClient_OpenNVGate[i]->SetOnSocketClose(OnSocketClose);
		m_pTcpClient_OpenNVGate[i]->SetOnOneNetMsg(OnOneNetMsg);
		m_pTcpClient_OpenNVGate[i]->SetOnOneMsg(OnOneMessage);
		m_pTcpClient_OpenNVGate[i]->SetOnSetCursorType(OnSetCursorType);
		m_pTcpClient_OpenNVGate[i]->SetOnButtonStatus(OnSetButtonStatus);
		m_pTcpClient_OpenNVGate[i]->SetOnAnalyzerStatus(OnSetAnalyzerStatus);
		m_pTcpClient_OpenNVGate[i]->setOnFinishCount(OnSetFinishCount);

		//Load Workbook的TCPCLIENT类实例
		*(m_pTcpClient_LoadWorkbook + i) = new CTcpClient(this);
		m_pTcpClient_LoadWorkbook[i]->SetOnSocketConnect(OnSocketConnection);
		m_pTcpClient_LoadWorkbook[i]->SetOnSocketClose(OnSocketClose);
		m_pTcpClient_LoadWorkbook[i]->SetOnOneNetMsg(OnOneNetMsg);
		m_pTcpClient_LoadWorkbook[i]->SetOnOneMsg(OnOneMessage);
		m_pTcpClient_LoadWorkbook[i]->SetOnSetCursorType(OnSetCursorType);
		m_pTcpClient_LoadWorkbook[i]->SetOnButtonStatus(OnSetButtonStatus);
		m_pTcpClient_LoadWorkbook[i]->SetOnAnalyzerStatus(OnSetAnalyzerStatus);
		m_pTcpClient_LoadWorkbook[i]->setOnFinishCount(OnSetFinishCount);
		m_pTcpClient_LoadWorkbook[i]->SetOnCurrentProjectName(OnSetCurrentProjectName);

		*(m_pTcpClient_SendFiles + i) = new CTcpClient(this);
		m_pTcpClient_SendFiles[i]->SetProgressTimeInterval(100);
		m_pTcpClient_SendFiles[i]->SetOnSocketConnect(OnSocketConnection);
		m_pTcpClient_SendFiles[i]->SetOnSocketClose(OnSocketClose);
		m_pTcpClient_SendFiles[i]->SetOnOneNetMsg(OnOneNetMsg);
		m_pTcpClient_SendFiles[i]->SetOnOneMsg(OnOneMessage);
		m_pTcpClient_SendFiles[i]->SetOnSendFileSucc(OnSendFileSucc);
		m_pTcpClient_SendFiles[i]->SetOnSendFileFail(OnSendFileFail);
		m_pTcpClient_SendFiles[i]->SetOnSendFileProgress(OnSendFileProgress);
		m_pTcpClient_SendFiles[i]->SetOnSetCursorType(OnSetCursorType);
		m_pTcpClient_SendFiles[i]->SetOnButtonStatus(OnSetButtonStatus);
		m_pTcpClient_SendFiles[i]->SetOnAnalyzerStatus(OnSetAnalyzerStatus);
		m_pTcpClient_SendFiles[i]->setOnFinishCount(OnSetFinishCount);

		//Load Workbook的TCPCLIENT类实例
		*(m_pTcpClient_CloseNVGate + i) = new CTcpClient(this);
		m_pTcpClient_CloseNVGate[i]->SetOnSocketConnect(OnSocketConnection);
		m_pTcpClient_CloseNVGate[i]->SetOnSocketClose(OnSocketClose);
		m_pTcpClient_CloseNVGate[i]->SetOnOneNetMsg(OnOneNetMsg);
		m_pTcpClient_CloseNVGate[i]->SetOnOneMsg(OnOneMessage);
		m_pTcpClient_CloseNVGate[i]->SetOnSetCursorType(OnSetCursorType);
		m_pTcpClient_CloseNVGate[i]->SetOnButtonStatus(OnSetButtonStatus);
		m_pTcpClient_CloseNVGate[i]->SetOnAnalyzerStatus(OnSetAnalyzerStatus);
		//m_pTcpClient_CloseNVGate[i]->setOnFinishCount(OnSetFinishCount);

		*(m_pTcpClient_FanStatus + i) = new CTcpClient(this);
		m_pTcpClient_FanStatus[i]->SetOnSocketConnect(OnSocketConnection);
		m_pTcpClient_FanStatus[i]->SetOnSocketClose(OnSocketClose);
		m_pTcpClient_FanStatus[i]->SetOnOneNetMsg(OnOneNetMsg);
		m_pTcpClient_FanStatus[i]->SetOnOneMsg(OnOneMessage);
		m_pTcpClient_FanStatus[i]->SetOnSetCursorType(OnSetCursorType);
		m_pTcpClient_FanStatus[i]->SetOnButtonStatus(OnSetButtonStatus);
		m_pTcpClient_FanStatus[i]->SetOnAnalyzerStatus(OnSetAnalyzerStatus);
		m_pTcpClient_FanStatus[i]->setOnFinishCount(OnSetFinishCount);
	
		//开始获取分析仪状态
		m_iFinishCount_AnalyzerStatus = 0;
		m_iFinishCount_Stopped = 0;
		//m_iFinishCount_Armed = 0;
		m_pTcpClient_States[i]->GetAnalyzerStatus(ipAddress, i, AfxGetMainWnd()->m_hWnd);

		//m_iFinishCount_FanStatus = 0;
		//m_pTcpClient_FanStatus[i]->GetFanStatus(ipAddress, i, AfxGetMainWnd()->m_hWnd);

		//MessageBox(_T("System Type is incorrect, please check custom.ini file."), _T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		//EndDialog(IDCANCEL);
		//return FALSE;

	}

	//给Monitor PC定义的
	for (int i = 0; i < m_nNumberOfMonitor; i++)
	{
		*(m_pTcpClient_SendFiles + i + numofSystem) = new CTcpClient(this);
		m_pTcpClient_SendFiles[i + numofSystem]->SetProgressTimeInterval(100);
		m_pTcpClient_SendFiles[i + numofSystem]->SetOnSocketConnect(OnSocketConnection);
		m_pTcpClient_SendFiles[i + numofSystem]->SetOnSocketClose(OnSocketClose);
		m_pTcpClient_SendFiles[i + numofSystem]->SetOnOneNetMsg(OnOneNetMsg);
		m_pTcpClient_SendFiles[i + numofSystem]->SetOnOneMsg(OnOneMessage);
		m_pTcpClient_SendFiles[i + numofSystem]->SetOnSendFileSucc(OnSendFileSucc);
		m_pTcpClient_SendFiles[i + numofSystem]->SetOnSendFileFail(OnSendFileFail);
		m_pTcpClient_SendFiles[i + numofSystem]->SetOnSendFileProgress(OnSendFileProgress);
		m_pTcpClient_SendFiles[i + numofSystem]->SetOnSetCursorType(OnSetCursorType);
		m_pTcpClient_SendFiles[i + numofSystem]->SetOnButtonStatus(OnSetButtonStatus);
		m_pTcpClient_SendFiles[i + numofSystem]->SetOnAnalyzerStatus(OnSetAnalyzerStatus);
		m_pTcpClient_SendFiles[i + numofSystem]->setOnFinishCount(OnSetFinishCount);
	}

	//if (!m_pTcpClient->OpenRemoteNVGate(ipAddress, numofSystem, AfxGetMainWnd()->m_hWnd))
	//	AfxMessageBox("发生文件失败");

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CNVSuperVisorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}


// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CNVSuperVisorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

void CNVSuperVisorDlg::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == 100)
	{
		CTime t1;
		t1 = CTime::GetCurrentTime();
		m_bar.SetPaneText(1, t1.Format("%H:%M:%S"));
	}
	CDialog::OnTimer(nIDEvent);
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CNVSuperVisorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CNVSuperVisorDlg::OnBnClickedReadgenerate()
{
	DispInfo(""); // 清空消息
	char strINIInfo[64] = { "" };
	CString sectionName = _T("");

	if (!UpdateData())
		return;

	CWrkList workbookList;
	CString selectWrkName;

	workbookList.SetPlace(0);
	workbookList.DoModal();
	nSelText = *workbookList.m_nSelText;

	if (nSelText != _T(""))
	{
		CString sourceName, targetName;
		sourceName.Format("%s\\DefaultConfig.ini", strAppPath);
		targetName.Format("%s\\LocalConfig.ini", strAppPath);
		CopyFile(sourceName, targetName, FALSE);

		//启动读取生成workbook线程		
		if (!m_pTcpClient_Generate->GetGenerateWorkbook(numofChannel, ipAddress, numofSystem, totalChannel, nSelText, AfxGetMainWnd()->m_hWnd))
			MessageBox(_T("Reading and generating of local NVGate failed, please check."), _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
	}


	

	

	//CInputBox inputBox;
	//inputBox.m_Input = _T("");

	//if (IDOK == inputBox.DoModal()) //显示输入框，接收信息，确定用户是否确定输入
	//{
	//	//inputBox.UpdateData();
	//	str_ConfigurationName = inputBox.m_Input;

	//	//循环看看是否有相同的
	//	//读取Configuration Number
	//	GetPrivateProfileString(_T("Workbook"), _T("Number"), _T("0"), strINIInfo, 64, tempIniFileName);
	//	int m_nWorkbookNumber = atoi(strINIInfo);
	//	for (int i = 0; i < m_nWorkbookNumber; i++)
	//	{
	//		sectionName.Format(_T("Workbook%d"), i+1);
	//		GetPrivateProfileString(_T("Workbook"), sectionName, _T(""), strINIInfo, 64, tempIniFileName);
	//		if (str_ConfigurationName == _T(strINIInfo))
	//		{
	//			MessageBox(_T("The confiugration name is exist already, please enter another name."), _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
	//			return;
	//		}
	//	}

	//	if (str_ConfigurationName != _T(""))
	//	{
	//		//拷贝缺省的INI文件

	//		CString sourceName, targetName;
	//		sourceName.Format("%s\\DefaultConfig.ini", strAppPath);
	//		targetName.Format("%s\\LocalConfig.ini", strAppPath);
	//		CopyFile(sourceName, targetName, FALSE);
	//		
	//		//启动读取生成workbook线程		
	//		if (!m_pTcpClient_Generate->GetGenerateWorkbook(numofChannel, ipAddress, numofSystem, totalChannel, str_ConfigurationName, AfxGetMainWnd()->m_hWnd))
	//			MessageBox(_T("Reading and generating of local NVGate failed, please check."), _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
	//	}
	//	else
	//	{
	//		MessageBox(_T("Please enter confiugration name and try again."), _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
	//	}		
	//}	
}


//void CNVSuperVisorDlg::OnBnClickedSendMsg()
//{
//	char s[99999];
//
//	if (!UpdateData())
//		return;
//
//	sprintf(s, "@00001%s", m_strMsg);
//	m_pTcpClient->SendNetMsg(s, strlen(s) - 6, 0);
//}

void CNVSuperVisorDlg::OnSocketClose(void *pNotifyObj, SOCKET hSocket, EMSocketCloseReason scr)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
	CString strInfo = _T("");

	//int a = pSendDlg->m_cursorType;
	pSendDlg->DispInfo(_T(""));
	strInfo.Format("Disconnected，SocketCloseReason = %d", scr);
	pSendDlg->m_ctlCnnStatus.SetWindowText(strInfo);
}

void CNVSuperVisorDlg::OnSocketConnection(void *pNotifyObj, SOCKET hSocket)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
	CString strInfo;

	strInfo.Format("Connected，Socket - %d", hSocket);
	pSendDlg->m_ctlCnnStatus.SetWindowText(strInfo);
}

void CNVSuperVisorDlg::OnOneNetMsg(void *pNotifyObj, char *Msg, int nMsgLen)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
	char s[9999];
	CString strInfo;

	strncpy(s, Msg, nMsgLen);
	s[nMsgLen] = 0;
	strInfo = s;
	pSendDlg->DispInfo(strInfo);

	//f_recvFinish = TRUE;
}

void CNVSuperVisorDlg::OnSendFileSucc(void *pNotifyObj, char *szPathName)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;

	pSendDlg->DispInfo("Send File succeed!");
}

void CNVSuperVisorDlg::OnSetCursorType(void *pNotifyObj, int cursorType)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
	pSendDlg->m_cursorType = cursorType;
}

void CNVSuperVisorDlg::OnSetAnalyzerStatus(void *pNotifyObj, int indexSystem, CString analyzerStatus, int indexStates)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
	pSendDlg->m_ctlSystemStatesList.SetItemText(indexSystem, indexStates, analyzerStatus);
	//pSendDlg->m_pAnalyzerStatus[indexSystem] = analyzerStatus;
}

//void CNVSuperVisorDlg::SetButtonEnableDisable(void)
//{
//	//Unknown
//	BOOL bool_States = TRUE;
//	int m_StateNumber = 0;
//	CString str_States = _T("");
//
//	for (int i = 0; i < numofSystem; i++)
//	{
//		str_States = m_pAnalyzerStatus[i];
//		if (str_States == _T("Unknown") || str_States == _T(""))
//		{
//			bool_States = bool_States && FALSE;
//			break;
//		}
//		else
//		{
//			bool_States = bool_States && TRUE;
//		}
//	}
//
//	if (!bool_States)   // 包含UNKNOWN状态
//	{
//		m_btnArm.EnableWindow(FALSE);
//		m_btnLoadWK.EnableWindow(FALSE);
//		m_btnOpenRemoteNVGate.EnableWindow(TRUE);
//		m_btnRunAutoZero.EnableWindow(FALSE);
//		m_btnStartMonitor.EnableWindow(FALSE);
//		m_btnStartRecord.EnableWindow(FALSE);
//		m_btnStopandSave.EnableWindow(FALSE);
//	}
//	else
//	{
//		//Stopped
//		//bool_States = TRUE;
//		m_StateNumber = 0;
//		for (int i = 0; i < numofSystem; i++)
//		{
//			str_States = m_pAnalyzerStatus[i];
//			if (str_States == _T("Stopped"))
//			{
//				m_StateNumber += 1;
//				//bool_States = bool_States && FALSE;
//				//break;
//			}
//			//else
//			//{
//			//	bool_States = bool_States && TRUE;
//			//}
//		}
//
//		if (!bool_States)   // 包含UNKNOWN状态
//		{
//			m_btnArm.EnableWindow(TRUE);
//			m_btnLoadWK.EnableWindow(FALSE);
//			m_btnOpenRemoteNVGate.EnableWindow(FALSE);
//			m_btnRunAutoZero.EnableWindow(FALSE);
//			m_btnStartMonitor.EnableWindow(FALSE);
//			m_btnStartRecord.EnableWindow(FALSE);
//			m_btnStopandSave.EnableWindow(FALSE);
//		}
//		else
//		{
//			//Armed
//			bool_States = TRUE;
//			for (int i = 0; i < numofSystem; i++)
//			{
//				str_States = m_pAnalyzerStatus[i];
//				if (str_States != _T("Armed"))
//				{
//					bool_States = bool_States && FALSE;
//					break;
//				}
//				else
//				{
//					bool_States = bool_States && TRUE;
//				}
//			}
//		}
//	}
//
//	
//
//	
//	
//}

void CNVSuperVisorDlg::OnSetFinishCount(void *pNotifyObj, int iSource, int successFlag)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
	CString tempStr = _T("");
	CString sectionName = _T("");
	char strINIInfo[64] = { "" };
	int m_nWorkbookNumber;
	pSendDlg->DispInfo(_T(""));

	switch (iSource)
	{
	case 1: //From Open NVGate
		pSendDlg->m_iFinishCount_OpenNVGate += 1;
		if (pSendDlg->m_iFinishCount_OpenNVGate == pSendDlg->numofSystem)
		{
			pSendDlg->m_cursorType = 0;
			pSendDlg->m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			pSendDlg->m_btnLoadWK.EnableWindow(TRUE);
			pSendDlg->m_btnArm.EnableWindow(TRUE);
			pSendDlg->m_btnStartMonitor.EnableWindow(TRUE);
			pSendDlg->m_btnStartRecord.EnableWindow(TRUE);
			pSendDlg->m_btnStopandSave.EnableWindow(TRUE);
			pSendDlg->m_btnRunAutoZero.EnableWindow(TRUE);

			////启动状态扫描
			////开始获取分析仪状态
			//for (int i = 0; i < pSendDlg->numofSystem; i++)
			//{
			//	pSendDlg->m_iFinishCount_AnalyzerStatus = 0;
			//	pSendDlg->m_iFinishCount_Stopped = 0;
			//	//m_iFinishCount_Armed = 0;
			//	pSendDlg->m_pTcpClient_States[i]->GetAnalyzerStatus(pSendDlg->ipAddress, i, AfxGetMainWnd()->m_hWnd);
			//}
			

		}
		break;
	case 2:  //From Send Workbook
		pSendDlg->m_iFinishCount_SendFiles += 1;
		if (pSendDlg->m_iFinishCount_SendFiles == pSendDlg->numofSystem)
		{
			pSendDlg->m_cursorType = 0;
			pSendDlg->m_btnReadGenerate.EnableWindow(TRUE);
			pSendDlg->m_btnLoadWK.EnableWindow(TRUE);
			pSendDlg->m_btnSendWKFiles.EnableWindow(TRUE);
			pSendDlg->m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			
			////读取Configuration Number
			//GetPrivateProfileString(_T("Workbook"), _T("Number"), _T("0"), strINIInfo, 64, pSendDlg->tempIniFileName);
			//m_nWorkbookNumber = atoi(strINIInfo);
			//
			//for (int i = 0; i < m_nWorkbookNumber;i++)
			//{
			//	sectionName.Format(_T("Workbook%d"), i+1);
			//	GetPrivateProfileString(_T("Workbook"), sectionName, _T(""), strINIInfo, 64, pSendDlg->tempIniFileName);
			//	if (pSendDlg->nSelText == _T(strINIInfo))
			//	{
			//		//写入状态
			//		sectionName.Format(_T("Status%d"), i+1);
			//		::WritePrivateProfileStringA(_T("Workbook"), sectionName, _T("1"), pSendDlg->tempIniFileName);
			//		break;
			//	}
			//	
			//}

			////写入状态
			//sectionName.Format(_T("Status%d"), pSendDlg->nSel);
			//::WritePrivateProfileStringA(_T("Workbook"), sectionName, _T("1"), pSendDlg->tempIniFileName);

		}

		break;
	case 3:   //From Load workbook
		pSendDlg->m_iFinishCount_LoadWorkbook += 1;
		if (pSendDlg->m_iFinishCount_LoadWorkbook == pSendDlg->numofSystem)
		{
			pSendDlg->m_cursorType = 0;
			pSendDlg->m_btnSendWKFiles.EnableWindow(TRUE);
			pSendDlg->m_btnLoadWK.EnableWindow(TRUE);
			pSendDlg->m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			pSendDlg->m_btnArm.EnableWindow(TRUE);
			pSendDlg->m_btnStartMonitor.EnableWindow(TRUE);
			pSendDlg->m_btnStartRecord.EnableWindow(TRUE);
			pSendDlg->m_btnStopandSave.EnableWindow(TRUE);
			pSendDlg->m_btnRunAutoZero.EnableWindow(TRUE);

			if (successFlag == 1)
			{
				pSendDlg->m_iSuccessCount_LoadWorkbook += 1;
			}
			
			if (pSendDlg->m_iSuccessCount_LoadWorkbook == pSendDlg->numofSystem)
			{
				pSendDlg->m_cCfgName.SetWindowTextA(pSendDlg->str_CurrentCFGName);
			}
			else
			{
				pSendDlg->m_cCfgName.SetWindowTextA(_T(""));
			}
			

			////启动创建同步文件夹对
			//for (int i = 0; i < pSendDlg->numofSystem; i++)
			//{
			//	if (pSendDlg->m_strProjectName != _T(""))
			//	{
			//		pSendDlg->m_pRemoteDesktop->CreateSyncPair(pSendDlg->ipAddress[i], pSendDlg->m_strProjectName);
			//		if (i + 1 == pSendDlg->numofSystem)
			//		{
			//			break;
			//		}
			//		else
			//		{
			//			Sleep(3000);
			//		}
			//	}								
			//}

		}

		break;

	case 4:   //From Arm, Start monitor, Start Record, Stop, Run AutoZero
		pSendDlg->m_iFinishCount_Stopped += 1;
		if (pSendDlg->m_iFinishCount_Stopped == pSendDlg->numofSystem)
		{
			pSendDlg->m_cursorType = 0;
			pSendDlg->m_btnLoadWK.EnableWindow(TRUE);
			pSendDlg->m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			pSendDlg->m_btnArm.EnableWindow(TRUE);
			pSendDlg->m_btnStartMonitor.EnableWindow(TRUE);
			pSendDlg->m_btnStartRecord.EnableWindow(TRUE);
			pSendDlg->m_btnStopandSave.EnableWindow(TRUE);
			pSendDlg->m_btnRunAutoZero.EnableWindow(TRUE);
			pSendDlg->b_FinishSaving = TRUE;
		}
		break;
	case 5:   //From Arm, Start monitor, Start Record, Stop, Run AutoZero
		pSendDlg->m_iFinishCount_Stopped += 1;
		if (pSendDlg->m_iFinishCount_Stopped == pSendDlg->numofSystem)
		{
			pSendDlg->m_cursorType = 0;
			pSendDlg->m_btnLoadWK.EnableWindow(TRUE);
			pSendDlg->m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			pSendDlg->m_btnArm.EnableWindow(TRUE);
			pSendDlg->m_btnStartMonitor.EnableWindow(TRUE);
			pSendDlg->m_btnStartRecord.EnableWindow(TRUE);
			pSendDlg->m_btnStopandSave.EnableWindow(TRUE);
			pSendDlg->m_btnRunAutoZero.EnableWindow(TRUE);
			pSendDlg->b_FinishSaving = TRUE;
		}
		break;

	//case 6:

	//	pSendDlg->m_cCfgName.SetWindowTextA(pSendDlg->str_ConfigurationName);
	//	break;


	//case 5:   //Armed状态
	//	pSendDlg->m_iFinishCount_Armed += 1;
	//	if (pSendDlg->m_iFinishCount_Armed == pSendDlg->numofSystem)
	//	{
	//		pSendDlg->m_cursorType = 0;
	//		//pSendDlg->m_btnSendWKFiles.EnableWindow(TRUE);
	//		pSendDlg->m_btnLoadWK.EnableWindow(TRUE);
	//		//pSendDlg->m_btnOpenRemoteNVGate.EnableWindow(TRUE);
	//		pSendDlg->m_btnArm.EnableWindow(FALSE);
	//		pSendDlg->m_iFinishCount_Armed = 0;
	//		pSendDlg->m_btnStartMonitor.EnableWindow(TRUE);
	//		pSendDlg->m_btnStartRecord.EnableWindow(FALSE);
	//		pSendDlg->m_btnStopandSave.EnableWindow(FALSE);
	//		pSendDlg->m_btnRunAutoZero.EnableWindow(FALSE);
	//	}
	//	else
	//	{
	//		pSendDlg->m_btnArm.EnableWindow(TRUE);
	//		pSendDlg->m_btnStartMonitor.EnableWindow(FALSE);
	//		pSendDlg->m_btnStartRecord.EnableWindow(FALSE);
	//		pSendDlg->m_btnStopandSave.EnableWindow(FALSE);
	//		pSendDlg->m_btnRunAutoZero.EnableWindow(FALSE);
	//	}
	//	break;

	default:
		break;
	}


}

void CNVSuperVisorDlg::OnExecuteGettingStatus(void *pNotifyObj, int nSys, CString analyzerStatus)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
	pSendDlg->m_pAnalyzerStatus[nSys] = analyzerStatus;
	//pSendDlg->SetButtonEnableDisable();
	/*pSendDlg->m_iFinishCount_AnalyzerStatus = 0;
	pSendDlg->m_pTcpClient_States[nSys]->GetAnalyzerStatus(pSendDlg->ipAddress, nSys, pSendDlg->m_hWnd);*/
}


void CNVSuperVisorDlg::OnSetButtonStatus(void *pNotifyObj, int m_iID, BOOL m_bStatus)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
	switch (m_iID)
	{
	case 1:
		pSendDlg->m_btnReadGenerate.EnableWindow(m_bStatus);
		break;
	case 2:
		pSendDlg->m_btnSendWKFiles.EnableWindow(m_bStatus);
		break;
	case 4:
		pSendDlg->m_btnOpenRemoteNVGate.EnableWindow(m_bStatus);
		break;
	case 3:
		pSendDlg->m_btnLoadWK.EnableWindow(m_bStatus);
		break;
	case 5:
		pSendDlg->m_btnArm.EnableWindow(m_bStatus);
		break;
	case 6:
		pSendDlg->m_btnStartMonitor.EnableWindow(m_bStatus);
		break;
	case 7:
		pSendDlg->m_btnStartRecord.EnableWindow(m_bStatus);
		break;
	case 8:
		pSendDlg->m_btnStopandSave.EnableWindow(m_bStatus);
		break;
	case 9:
		pSendDlg->m_btnRunAutoZero.EnableWindow(m_bStatus);
		break;
	default:
		break;
	}
}

void CNVSuperVisorDlg::OnSetCurrentProjectName(void *pNotifyObj, CString projectName)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
	pSendDlg->m_strProjectName = projectName;
}


//void CNVSuperVisorDlg::OnGetLocalInformationSucc(void *pNotifyObj)
//{
//	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
//
//	pSendDlg->DispInfo("Getting Local Information Succeed!");
//}

//void CNVSuperVisorDlg::OnGetLocalInformationFail(void *pNotifyObj)
//{
//	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
//
//	pSendDlg->DispInfo("Getting Local Information Failed!");
//}

//显示程序进行过程中的信息
void CNVSuperVisorDlg::OnOneMessage(void *pNotifyObj, CString Msg)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
	pSendDlg->DispInfo(Msg);
}

//跳出错误窗口
void CNVSuperVisorDlg::OnErrorMessage(void *pNotifyObj, CString Msg)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
	pSendDlg->DispError(Msg);
}

void CNVSuperVisorDlg::OnSendFileFail(void *pNotifyObj, char *szPathName, EMSendFileFailReason SendFileFailReadson)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
	CString strInfo;

	strInfo.Format("Socket is closed, Fail Reason = %d", SendFileFailReadson);
	pSendDlg->DispInfo(strInfo);
}

void CNVSuperVisorDlg::OnSendFileProgress(void *pNotifyObj, UWORD nSentBytes, UWORD nTotalBytes)
{
	CNVSuperVisorDlg *pSendDlg = (CNVSuperVisorDlg *)pNotifyObj;
	CString strInfo;

	strInfo.Format("%llu / %llu", nSentBytes, nTotalBytes);
	pSendDlg->DispInfo(strInfo);
}

void CNVSuperVisorDlg::DispInfo(CString strInfo)
{
	//m_ctlInfo.SetWindowText(strInfo);
	m_bar.SetPaneText(0, strInfo);
}

void CNVSuperVisorDlg::DispError(CString errorMsg)
{
	MessageBox(errorMsg, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
}

BOOL CNVSuperVisorDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	HCURSOR hCur;

	if (m_cursorType)
	{
		hCur = LoadCursor(NULL, IDC_WAIT);
	}
	else
	{
		hCur = LoadCursor(NULL, IDC_ARROW);
	}
	::SetCursor(hCur);

	return TRUE;
	//return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}


void CNVSuperVisorDlg::OnBnClickedSendworkbook()
{
	
	CString str_Temp = _T("");

	int nListCount = 0;

	DispInfo(""); // 清空消息

	if (!UpdateData())
		return;

	CWrkList workbookList;
	CString selectWrkName;

	workbookList.SetPlace(0);
	workbookList.DoModal();
	nSelText = *workbookList.m_nSelText;
	//nListCount = workbookList.m_nCount;

	if (workbookList.m_nCount > 0)
	{
		selectWrkName.Format(_T("#%s"), *workbookList.m_nSelText);
		m_iFinishCount_SendFiles = 0;

		//m_cursorType = 1;
		m_btnLoadWK.EnableWindow(FALSE);
		m_btnReadGenerate.EnableWindow(FALSE);
		m_btnSendWKFiles.EnableWindow(FALSE);
		m_btnOpenRemoteNVGate.EnableWindow(FALSE);

		for (int i = 0; i < numofSystem; i++)
		{
			m_pTcpClient_SendFiles[i]->SetFilePackageSize(m_dwFilePackageSize);
			if (!m_pTcpClient_SendFiles[i]->SendFile(ipAddress, i, selectWrkName, 0, AfxGetMainWnd()->m_hWnd))
			{
				OnSetFinishCount(this, 2, 0);
				str_Temp.Format(_T("Sending workbook files to SmartRoute PC of System #%d failed, please check."), i + 1);
				MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
			}
		}

		for (int i = 0; i < m_nNumberOfMonitor; i++)
		{
			m_pTcpClient_SendFiles[i + numofSystem]->SetFilePackageSize(m_dwFilePackageSize);
			if (!m_pTcpClient_SendFiles[i + numofSystem]->SendFile(m_pMonitorIP, i, *workbookList.m_nSelText, 1, AfxGetMainWnd()->m_hWnd))
			{
				//OnSetFinishCount(this, 2);
				str_Temp.Format(_T("Sending workbook files to Monitor PC #%d failed, please check."), i + 1);
				MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
			}
		}

	}


	//if (IDOK == workbookList.DoModal()) //显示输入框，接收信息，确定用户是否确定输入
	//{
	//	nListCount = workbookList.m_Listbox_Workbook.GetCount();
	//	if (nListCount > 0)
	//	{
	//		nSel = workbookList.m_Listbox_Workbook.GetSelCount();
	//		if (nSel > 0)
	//		{
	//			workbookList.m_Listbox_Workbook.GetText(nSel, selectWrkName);
	//			
	//			m_iFinishCount_SendFiles = 0;

	//			//m_cursorType = 1;
	//			m_btnLoadWK.EnableWindow(FALSE);
	//			m_btnReadGenerate.EnableWindow(FALSE);
	//			m_btnSendWKFiles.EnableWindow(FALSE);
	//			m_btnOpenRemoteNVGate.EnableWindow(FALSE);

	//			for (int i = 0; i < numofSystem; i++)
	//			{
	//				m_pTcpClient_SendFiles[i]->SetFilePackageSize(m_dwFilePackageSize);
	//				if (!m_pTcpClient_SendFiles[i]->SendFile(ipAddress, i, selectWrkName, AfxGetMainWnd()->m_hWnd))
	//				{
	//					OnSetFinishCount(this, 2);
	//					str_Temp.Format(_T("Sending workbook files to System #%d failed, please check."), i + 1);
	//					MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
	//				}

	//			}
	//		}
	//	}
	//	
	//		

	//		


	//	

	//}


	//

}


void CNVSuperVisorDlg::OnBnClickedOperateremotenvgate()
{
	// TODO:  在此添加控件通知处理程序代码
	CString str_Temp = _T("");

	DispInfo(""); // 清空消息

	if (!UpdateData())
		return;


	CWrkList workbookList;
	CString selectWrkName;



	for (int i = 0; i < numofSystem; i++)
	{
		if (m_pAnalyzerStatus[i] == _T("Unknown"))
		{
			str_Temp.Format(_T("The states of remote NVGate in System #%d is unknown, please check."), i + 1);
			MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONASTERISK);
			m_iFinishCount_LoadWorkbook = 0;
			m_iSuccessCount_LoadWorkbook = 0;

			//m_cursorType = 1;
			m_btnSendWKFiles.EnableWindow(TRUE);

			m_btnLoadWK.EnableWindow(TRUE);
			m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			m_btnArm.EnableWindow(TRUE);
			m_btnStartMonitor.EnableWindow(TRUE);
			m_btnStartRecord.EnableWindow(TRUE);
			m_btnStopandSave.EnableWindow(TRUE);
			m_btnRunAutoZero.EnableWindow(TRUE);
			return;
		}
	}

	for (int i = 0; i < numofSystem; i++)
	{
		if (m_pAnalyzerStatus[i] == _T("Triggering") || m_pAnalyzerStatus[i] == _T("Recording"))
		{
			str_Temp.Format(_T("The states of remote NVGate in System #%d is triggering or recording, please stop firstly."), i + 1);
			MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONASTERISK);
			m_iFinishCount_LoadWorkbook = 0;
			m_iSuccessCount_LoadWorkbook = 0;
			//m_cursorType = 1;
			m_btnSendWKFiles.EnableWindow(TRUE);
			m_btnLoadWK.EnableWindow(TRUE);
			m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			m_btnArm.EnableWindow(TRUE);
			m_btnStartMonitor.EnableWindow(TRUE);
			m_btnStartRecord.EnableWindow(TRUE);
			m_btnStopandSave.EnableWindow(TRUE);
			m_btnRunAutoZero.EnableWindow(TRUE);
			return;
		}
	}

	workbookList.SetPlace(1);
	workbookList.DoModal();
	nSelText = *workbookList.m_nSelText;
	str_CurrentCFGName = nSelText;

	

	if (workbookList.m_nCount > 0)
	{
		m_iFinishCount_LoadWorkbook = 0;
		m_iSuccessCount_LoadWorkbook = 0;

		//m_cursorType = 1;
		m_btnSendWKFiles.EnableWindow(FALSE);
		m_btnLoadWK.EnableWindow(FALSE);
		m_btnOpenRemoteNVGate.EnableWindow(FALSE);
		m_btnArm.EnableWindow(FALSE);
		m_btnStartMonitor.EnableWindow(FALSE);
		m_btnStartRecord.EnableWindow(FALSE);
		m_btnStopandSave.EnableWindow(FALSE);
		m_btnRunAutoZero.EnableWindow(FALSE);

		for (int i = 0; i < numofSystem; i++)
		{
			if (!m_pTcpClient_LoadWorkbook[i]->OperateRemoteNVGate(ipAddress, i, *workbookList.m_nSelText, AfxGetMainWnd()->m_hWnd))
			{
				OnSetFinishCount(this, 3, 0);
				str_Temp.Format(_T("Loading workbook model in System #%d failed, please check."), i + 1);
				MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
			}

		}
	}
	else
	{
		OnSetFinishCount(this, 3, 0);
	}

}


void CNVSuperVisorDlg::OnBnClickedOpennvgate()
{
	// TODO:  在此添加控件通知处理程序代码
	CString str_Temp = _T("");

	DispInfo(""); // 清空消息

	if (!UpdateData())
		return;

	m_iFinishCount_OpenNVGate = 0;
	//m_cursorType = 1;

	m_btnOpenRemoteNVGate.EnableWindow(FALSE);
	m_btnLoadWK.EnableWindow(FALSE);
	m_btnArm.EnableWindow(FALSE);
	m_btnStartMonitor.EnableWindow(FALSE);
	m_btnStartRecord.EnableWindow(FALSE);
	m_btnStopandSave.EnableWindow(FALSE);
	m_btnRunAutoZero.EnableWindow(FALSE);

	for (int i = 0; i < numofSystem; i++)
	{
		if (m_pAnalyzerStatus[i] == _T("Unknown"))
		{
			if (!m_pTcpClient_OpenNVGate[i]->OpenRemoteNVGate(ipAddress, i, AfxGetMainWnd()->m_hWnd, m_pHardIpAddr, m_pType))
			{
				OnSetFinishCount(this, 1, 0);
				str_Temp.Format(_T("Open remote NVGate in System #%d failed, please check."), i + 1);
				MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
			}				
		}
		else
		{
			OnSetFinishCount(this, 1, 0);
		}
		
	}
}

CButton* CNVSuperVisorDlg::NewMyButton(int nID, CRect rect, int nStyle)
{
	CString m_Caption;
	m_Caption.LoadString(nID); //取按钮标题
	CButton *p_Button = new CButton();
	ASSERT_VALID(p_Button);
	p_Button->Create(m_Caption, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | nStyle, rect, this, nID); //创建按钮
	return p_Button;
}


//void CNVSuperVisorDlg::OnBnClickedOpenremotedesktop()
//{
//	// TODO:  在此添加控件通知处理程序代码
//	int iret = m_pRemoteDesktop->OpenRemoteDesktop();
//
//}
void CNVSuperVisorDlg::SetCurrentProjectName(CString projectName)
{
	m_strProjectName = projectName;
}

void CNVSuperVisorDlg::OnBnClickedAlarmrecorder()
{
	// TODO:  在此添加控件通知处理程序代码
	CString str_Temp = _T("");

	DispInfo(""); // 清空消息

	if (!UpdateData())
		return;

	m_iFinishCount_Stopped = 0;
	//m_cursorType = 1;

	m_btnOpenRemoteNVGate.EnableWindow(FALSE);
	m_btnLoadWK.EnableWindow(FALSE);
	m_btnArm.EnableWindow(FALSE);
	m_btnStartMonitor.EnableWindow(FALSE);
	m_btnStartRecord.EnableWindow(FALSE);
	m_btnStopandSave.EnableWindow(FALSE);
	m_btnRunAutoZero.EnableWindow(FALSE);


	for (int i = 0; i < numofSystem; i++)
	{
		if (m_pAnalyzerStatus[i] == _T("Unknown"))
		{
			str_Temp.Format(_T("The states of remote NVGate in System #%d is unknown, please check."), i + 1);
			MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONASTERISK);
			m_iFinishCount_LoadWorkbook = 0;

			//m_cursorType = 1;
			m_btnSendWKFiles.EnableWindow(TRUE);

			m_btnLoadWK.EnableWindow(TRUE);
			m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			m_btnArm.EnableWindow(TRUE);
			m_btnStartMonitor.EnableWindow(TRUE);
			m_btnStartRecord.EnableWindow(TRUE);
			m_btnStopandSave.EnableWindow(TRUE);
			m_btnRunAutoZero.EnableWindow(TRUE);
			return;
		}
	}


	for (int i = 0; i < numofSystem; i++)
	{
		if (m_pAnalyzerStatus[i] == _T("Triggering") || m_pAnalyzerStatus[i] == _T("Recording"))
		{
			str_Temp.Format(_T("The states of remote NVGate in System #%d is triggering or recording, please stop firstly."), i + 1);
			MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONASTERISK);
			m_iFinishCount_LoadWorkbook = 0;

			//m_cursorType = 1;
			m_btnSendWKFiles.EnableWindow(TRUE);
			m_btnLoadWK.EnableWindow(TRUE);
			m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			m_btnArm.EnableWindow(TRUE);
			m_btnStartMonitor.EnableWindow(TRUE);
			m_btnStartRecord.EnableWindow(TRUE);
			m_btnStopandSave.EnableWindow(TRUE);
			m_btnRunAutoZero.EnableWindow(TRUE);
			return;
		}
	}


	for (int i = 0; i < numofSystem; i++)
	{
		if (m_pAnalyzerStatus[i] != _T("Armed"))
		{
			if (!m_pTcpClient_Stop[i]->AlarmRecorder(ipAddress, i, AfxGetMainWnd()->m_hWnd))
			{
				OnSetFinishCount(this, 4, 0);
				str_Temp.Format(_T("Arming recorder in System #%d failed, please check."), i + 1);
				MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
			}			
		}
		else
		{
			OnSetFinishCount(this, 4, 0);
		}
		
	}

	
}


void CNVSuperVisorDlg::OnBnClickedStartmonitor()
{
	// TODO:  在此添加控件通知处理程序代码

	CString str_Temp = _T("");

	DispInfo(""); // 清空消息

	if (!UpdateData())
		return;

	m_iFinishCount_Stopped = 0;
	//m_cursorType = 1;

	m_btnOpenRemoteNVGate.EnableWindow(FALSE);
	m_btnLoadWK.EnableWindow(FALSE);
	m_btnArm.EnableWindow(FALSE);
	m_btnStartMonitor.EnableWindow(FALSE);
	m_btnStartRecord.EnableWindow(FALSE);
	m_btnStopandSave.EnableWindow(FALSE);
	m_btnRunAutoZero.EnableWindow(FALSE);


	for (int i = 0; i < numofSystem; i++)
	{
		if (m_pAnalyzerStatus[i] == _T("Unknown"))
		{
			str_Temp.Format(_T("The states of remote NVGate in System #%d is unknown, please check."), i + 1);
			MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONASTERISK);
			m_iFinishCount_LoadWorkbook = 0;

			//m_cursorType = 1;
			m_btnSendWKFiles.EnableWindow(TRUE);

			m_btnLoadWK.EnableWindow(TRUE);
			m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			m_btnArm.EnableWindow(TRUE);
			m_btnStartMonitor.EnableWindow(TRUE);
			m_btnStartRecord.EnableWindow(TRUE);
			m_btnStopandSave.EnableWindow(TRUE);
			m_btnRunAutoZero.EnableWindow(TRUE);
			return;
		}
	}


	for (int i = 0; i < numofSystem; i++)
	{
		if (m_pAnalyzerStatus[i] == _T("Recording"))
		{
			str_Temp.Format(_T("The remote NVGate in one system #%d is recording, please stop it firstly."), i + 1);
			MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONASTERISK);

			m_iFinishCount_Stopped = 0;
			//m_cursorType = 1;
			m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			m_btnLoadWK.EnableWindow(TRUE);
			m_btnArm.EnableWindow(TRUE);
			m_btnStartMonitor.EnableWindow(TRUE);
			m_btnStartRecord.EnableWindow(TRUE);
			m_btnStopandSave.EnableWindow(TRUE);
			m_btnRunAutoZero.EnableWindow(TRUE);
			return;

		}
	}

	for (int i = 0; i < numofSystem; i++)
	{			
		if (m_pAnalyzerStatus[i] != _T("Triggering"))
		{
			if (!m_pTcpClient_Stop[i]->StartMonitor(ipAddress, i, AfxGetMainWnd()->m_hWnd))
			{
				str_Temp.Format(_T("Starting monitoring in System #%d failed, please check!"), i + 1);
				MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
				OnSetFinishCount(this, 4, 0);
			}				
		}
		else
		{
			OnSetFinishCount(this, 4, 0);
		}
	}
}


void CNVSuperVisorDlg::OnBnClickedStartrecord()
{
	// TODO:  在此添加控件通知处理程序代码
	CString str_Temp = _T("");

	DispInfo(""); // 清空消息

	if (!UpdateData())
		return;

	m_iFinishCount_Stopped = 0;
	//m_cursorType = 1;

	m_btnOpenRemoteNVGate.EnableWindow(FALSE);
	m_btnLoadWK.EnableWindow(FALSE);
	m_btnArm.EnableWindow(FALSE);
	m_btnStartMonitor.EnableWindow(FALSE);
	m_btnStartRecord.EnableWindow(FALSE);
	m_btnStopandSave.EnableWindow(FALSE);
	m_btnRunAutoZero.EnableWindow(FALSE);
	//m_BtnExit.EnableWindow(FALSE);

	for (int i = 0; i < numofSystem; i++)
	{
		if (m_pAnalyzerStatus[i] == _T("Recording") || m_pAnalyzerStatus[i] != _T("Triggering"))
		{
			str_Temp.Format(_T("The states of remote NVGate in System #%d isn't triggering or is recording, please stop firstly."), i + 1);
			MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONASTERISK);
			m_iFinishCount_LoadWorkbook = 0;
			m_btnSendWKFiles.EnableWindow(TRUE);
			m_btnLoadWK.EnableWindow(TRUE);
			m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			m_btnArm.EnableWindow(TRUE);
			m_btnStartMonitor.EnableWindow(TRUE);
			m_btnStartRecord.EnableWindow(TRUE);
			m_btnStopandSave.EnableWindow(TRUE);
			m_btnRunAutoZero.EnableWindow(TRUE);
			return;
		}
	}
	if (!m_pTcpClient_Stop[0]->StartRecord(ipAddress, 0, AfxGetMainWnd()->m_hWnd))
		MessageBox(_T("Starting record failed, please check!"), _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
}


void CNVSuperVisorDlg::OnBnClickedStoprecord()
{
	// TODO:  在此添加控件通知处理程序代码
	CString str_Temp = _T("");
	DispInfo(""); // 清空消息

	if (!UpdateData())
		return;

	
	m_iFinishCount_Stopped = 0;
	//m_cursorType = 1;

	m_btnOpenRemoteNVGate.EnableWindow(FALSE);
	m_btnLoadWK.EnableWindow(FALSE);
	m_btnArm.EnableWindow(FALSE);
	m_btnStartMonitor.EnableWindow(FALSE);
	m_btnStartRecord.EnableWindow(FALSE);
	m_btnStopandSave.EnableWindow(FALSE);
	m_btnRunAutoZero.EnableWindow(FALSE);


	for (int i = 0; i < numofSystem; i++)
	{
		if (m_pAnalyzerStatus[i] != _T("Unknown"))
		{
			
			if (!m_pTcpClient_Save[i]->StopRecord(ipAddress, i, AfxGetMainWnd()->m_hWnd))
			{
				OnSetFinishCount(this, 4, 0);
				str_Temp.Format(_T("Stop and saving measurement in System #%d failed, please check!"), i + 1);
				MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
			}
		}
		else
		{
			OnSetFinishCount(this, 4, 0);
		}
		
		
		/*if (m_pAnalyzerStatus[i] != _T("Stopped"))
		{*/
			
				
		//}
		//else
		//{
		//	OnSetFinishCount(this, 4);
		//}		
	}
}


void CNVSuperVisorDlg::OnBnClickedOk()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	//m_cursorType = 1;
	
	int nRes = MessageBox(_T("Do you want to quit the program?"), _T("NVSuperVisor"), MB_YESNO | MB_ICONQUESTION);
	switch (nRes)
	{
	case 6:
			
		for (int i = 0; i < numofSystem; i++)
		{
			if (m_pTcpClient_States[i]->m_pGetAnalyzerStatus != NULL)
			{
				m_pTcpClient_States[i]->SetGettingStateFlag();
				Sleep(1);
			}
		}

		for (int i = 0; i < numofSystem; i++)
		{
			if (m_pTcpClient_OpenNVGate[i]->m_pOpenRemoteNVGate != NULL)
			{
				m_pTcpClient_OpenNVGate[i]->HandleOpenNVGateMuteMsg();
				Sleep(1);
			}
		}

		//保存
		nRes = MessageBox(_T("Do you want to stop the measurement?"), _T("NVSuperVisor"), MB_YESNO | MB_ICONQUESTION);
		switch (nRes)
		{
		case 6:
			for (int i = 0; i < numofSystem; i++)
			{
				if (m_pAnalyzerStatus[i] != _T("Unknown"))
				{
					m_pTcpClient_Save[i]->StopRecord(ipAddress, i, AfxGetMainWnd()->m_hWnd);
					Sleep(1);
				}
				
			}

			nRes = MessageBox(_T("Do you want to close the remote NVGate?"), _T("NVSuperVisor"), MB_YESNO | MB_ICONQUESTION);
			switch (nRes)
			{
			case 6:
				//发送关闭NVGate命令
				for (int i = 0; i < numofSystem; i++)
				{
					m_pTcpClient_CloseNVGate[i]->CloseNVGate(ipAddress, i, AfxGetMainWnd()->m_hWnd);
					Sleep(1);
				}
				break;
			case 7:
				break;
			default:
				break;
			}

			m_cursorType = 0;
			break;
		case 7:

			break;
		default:

			break;
		}

		CDialogEx::OnOK();
		
		break;
	default:
		break;
	}

}
void CNVSuperVisorDlg::OnBnClickedBtnautozero()
{
	// TODO:  在此添加控件通知处理程序代码
	CString str_Temp = _T("");
	DispInfo(""); // 清空消息

	if (!UpdateData())
		return;

	m_iFinishCount_Stopped = 0;
	//m_cursorType = 1;

	m_btnOpenRemoteNVGate.EnableWindow(FALSE);
	m_btnLoadWK.EnableWindow(FALSE);
	m_btnArm.EnableWindow(FALSE);
	m_btnStartMonitor.EnableWindow(FALSE);
	m_btnStartRecord.EnableWindow(FALSE);
	m_btnStopandSave.EnableWindow(FALSE);
	m_btnRunAutoZero.EnableWindow(FALSE);


	for (int i = 0; i < numofSystem; i++)
	{
		if (m_pAnalyzerStatus[i] == _T("Unknown"))
		{
			str_Temp.Format(_T("The states of remote NVGate in System #%d is unknown, please check."), i + 1);
			MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONASTERISK);
			m_iFinishCount_LoadWorkbook = 0;

			//m_cursorType = 1;
			m_btnSendWKFiles.EnableWindow(TRUE);

			m_btnLoadWK.EnableWindow(TRUE);
			m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			m_btnArm.EnableWindow(TRUE);
			m_btnStartMonitor.EnableWindow(TRUE);
			m_btnStartRecord.EnableWindow(TRUE);
			m_btnStopandSave.EnableWindow(TRUE);
			m_btnRunAutoZero.EnableWindow(TRUE);
			return;
		}
	}


	for (int i = 0; i < numofSystem; i++)
	{
		if (m_pAnalyzerStatus[i] == _T("Triggering") || m_pAnalyzerStatus[i] == _T("Recording"))
		{
			str_Temp.Format(_T("The states of remote NVGate in System #%d is triggering or recording, please stop firstly."), i + 1);
			MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONASTERISK);
			m_iFinishCount_LoadWorkbook = 0;

			//m_cursorType = 1;
			m_btnSendWKFiles.EnableWindow(TRUE);
			m_btnLoadWK.EnableWindow(TRUE);
			m_btnOpenRemoteNVGate.EnableWindow(TRUE);
			m_btnArm.EnableWindow(TRUE);
			m_btnStartMonitor.EnableWindow(TRUE);
			m_btnStartRecord.EnableWindow(TRUE);
			m_btnStopandSave.EnableWindow(TRUE);
			m_btnRunAutoZero.EnableWindow(TRUE);
			return;
		}
	}

	for (int i = 0; i < numofSystem; i++)
	{
		if (!m_pTcpClient_Stop[i]->RunAutoZero(ipAddress, i, AfxGetMainWnd()->m_hWnd))
		{
			OnSetFinishCount(this, 4, 0);
			str_Temp.Format(_T("Running auto-zero in System #%d failed, please check."), i + 1);
			MessageBox(str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
		}

	}


}


void CNVSuperVisorDlg::OnSystemButton(UINT uID)
{
	// TODO:  在此添加控件通知处理程序代码
	int a = uID - IDC_SYSTEMBTN;
	m_pRemoteDesktop->OpenRemoteDesktop(ipAddress[uID - IDC_SYSTEMBTN]);


}


void CNVSuperVisorDlg::OnClose()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	//m_cursorType = 1;
	int nRes = MessageBox(_T("Do you want to quit the program?"), _T("NVSuperVisor"), MB_YESNO | MB_ICONQUESTION);
	switch (nRes)
	{
	case 6:
		
		for (int i = 0; i < numofSystem; i++)
		{
			m_pTcpClient_States[i]->SetGettingStateFlag();
			Sleep(1);
		}

		for (int i = 0; i < numofSystem; i++)
		{
			if (m_pTcpClient_OpenNVGate[i]->m_pOpenRemoteNVGate != NULL)
			{
				m_pTcpClient_OpenNVGate[i]->HandleOpenNVGateMuteMsg();
				Sleep(1);
			}

		}


		//保存
		nRes = MessageBox(_T("Do you want to stop the measurement?"), _T("NVSuperVisor"), MB_YESNO | MB_ICONQUESTION);
		switch (nRes)
		{
		case 6:
			for (int i = 0; i < numofSystem; i++)
			{
				m_pTcpClient_Save[i]->StopRecord(ipAddress, i, AfxGetMainWnd()->m_hWnd);
				Sleep(1);
			}


			nRes = MessageBox(_T("Do you want to close the remote NVGate?"), _T("NVSuperVisor"), MB_YESNO | MB_ICONQUESTION);
			switch (nRes)
			{
			case 6:
				//发送关闭NVGate命令
				for (int i = 0; i < numofSystem; i++)
				{
					m_pTcpClient_CloseNVGate[i]->CloseNVGate(ipAddress, i, AfxGetMainWnd()->m_hWnd);
					Sleep(1);
				}
				break;
			case 7:
				break;
			default:
				break;
			}

			m_cursorType = 0;
			break;
		default:
			break;
		}
		

		CDialogEx::OnClose();

		break;
	default:
		break;
	}
	
}


void CNVSuperVisorDlg::OnBnClickedSynctoy()
{
	
	char str_Content[64] = { "0" };
	GetPrivateProfileString(_T("Parameters"), _T("ProjectName"), _T(""), str_Content, 64, iniFileName);
	
	// TODO:  在此添加控件通知处理程序代码
	for (int i = 0; i < numofSystem; i++)
	{
		/*m_pRemoteDesktop->RunSync(ipAddress[i], m_strProjectName);*/
		m_pRemoteDesktop->RunSync(ipAddress[i], _T(str_Content));
		Sleep(1000);
	}
	
}
