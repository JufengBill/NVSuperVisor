// TcpClient
// 可以发送网络信息，可以发送文件，支持断点续传
// XSoft
// Contact: hongxing777@msn.com or lovelypengpeng@eyou.com
//
#include "stdafx.h"
#include "TcpClient.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

//////////////////////////////////////////////////////////////////////
// CTcpClient
//////////////////////////////////////////////////////////////////////
CTcpClient::CTcpClient(void *pNotifyObj)
{
	memset(&m_WSAData, 0, sizeof(WSADATA));
	m_pNotifyObj = pNotifyObj;
	m_hHideWnd = NULL;
	m_hSocket = INVALID_SOCKET;
	m_pClientSocketRecvThread = NULL;
	m_pSendFileThread = NULL;
	m_pGenerateModelThread = NULL;
	m_pOperateRemoteNVGate = NULL;
	m_pOpenRemoteNVGate = NULL;
	m_pAlarmSystem = NULL;
	m_pStartMonitor = NULL;
	m_pStartRecord = NULL;
	m_pStopRecord = NULL;
	m_pGetAnalyzerStatus = NULL;
	m_pRunAutoZero = NULL;
	m_pCloseNVGate = NULL;
	m_pGetFanStatus = NULL;

	strcpy(m_szServerIpAddr, "");
	m_wPort = DFT_SOCKETPORT;
	m_dwFilePackageSize = DFT_FILEPACKAGESIZE;
	m_nWaitTimeOut = DFT_WAITTIMEOUT;
	m_nSendTimeOut = DFT_SENDTIMEOUT;
	m_dwProgressTimeInterval = DFT_PROGRESSTIMEINTERVAL;
	m_nSocketRecvThreadPriority = DFT_SOCKETRECVTHREADPRIORITY;
	m_nSendFileThreadPriority = DFT_SENDFILETHREADPRIORITY;

	//bDataFromNVGate = NULL;
	//m_bGetResponseFinish = FALSE;
	//msgLen = 0;
	//m_bTerminateStatus = FALSE;


	// 事件指针
	m_OnSocketClose = NULL;
	m_OnOneNetMsg = NULL;
	m_OnSendFileSucc = NULL;
	//m_OnGetLocalInformationSucc = NULL;
	//m_OnGetLocalInformationFail = NULL;
	m_OnSendFileFail = NULL;
	m_OnSendFileProgress = NULL;
	m_OnSetCursorType = NULL;
	m_OnSetButtonStatus = NULL;
	m_OnSetAnalyzerStatus = NULL;
	m_OnSetCurrentProjectName = NULL;

	m_OnSetFinishCount = NULL;

	StartupSocket(); // 初始化windows socket
	RegisterClientSocketHideWndClass(); // 出错隐藏窗体类
}

CTcpClient::~CTcpClient(void)
{
	m_nCurrent = 0;
	msgLen = 0;
	bDataFromNVGate = NULL;
	m_bGetResponseFinish = FALSE;
	m_bTerminateStatus = FALSE;


	//delete m_sStateInfor;

	Disconnect(); // 断开socket连接
	CleanupSocket();
	UnregisterClass(STR_CLIENTSOCKETHIDEWNDCLASSNAME, AfxGetInstanceHandle());
	DiscardThread();  //销毁文件发送的线程
}

void CTcpClient::SetServerIpAddr(char *szServerIpAddr)
{
	strcpy(m_szServerIpAddr, szServerIpAddr);
}

void CTcpClient::SetPort(WORD wPort)
{
	ASSERT(wPort > 1024);

	m_wPort = wPort;
}

void CTcpClient::SetFilePackageSize(DWORD dwFilePackageSize)
{
	ASSERT(dwFilePackageSize > 0 && dwFilePackageSize <= MAX_FILEPACKAESIZE);

	m_dwFilePackageSize = dwFilePackageSize;
}

void CTcpClient::SetWaitTimeOut(int nWaitTimeOut)
{
	ASSERT(nWaitTimeOut >= 1000);
	m_nWaitTimeOut = nWaitTimeOut;
}

void CTcpClient::SetSendTimeOut(int nSendTimeOut)
{
	ASSERT(nSendTimeOut >= 1000);
	m_nSendTimeOut = nSendTimeOut;
}

void CTcpClient::SetProgressTimeInterval(DWORD dwProgressTimeInterval)
{
	m_dwProgressTimeInterval = dwProgressTimeInterval;
}

void CTcpClient::SetSocketRecvThreadPriority(int nPriority)
{
	m_nSocketRecvThreadPriority = nPriority;
}

void CTcpClient::SetSendFileThreadPriority(int nPriority)
{
	m_nSendFileThreadPriority = nPriority;
}

void CTcpClient::SetOnSocketClose(SocketCloseFun OnSocketClose)
{
	m_OnSocketClose = OnSocketClose;
}

void CTcpClient::SetOnSocketConnect(SocketConnectFun OnSocketConnect)
{
	m_OnSocketConnect = OnSocketConnect;
}


void CTcpClient::SetOnOneNetMsg(OneNetMsgFun OnOneNetMsg)
{
	m_OnOneNetMsg = OnOneNetMsg;
}

void CTcpClient::SetOnSendFileSucc(SendFileFun OnSendFileSucc)
{
	m_OnSendFileSucc = OnSendFileSucc;
}


void CTcpClient::SetOnOneMsg(OneMsgFun OnOneMsg)
{
	m_OnOneMsg = OnOneMsg;
}


void CTcpClient::SetOnSetCursorType(SetCursorTypeFun OnSetCursorType)
{
	m_OnSetCursorType = OnSetCursorType;
}

void CTcpClient::SetOnButtonStatus(SetButtonStatusFun OnSetButtonStatus)
{
	m_OnSetButtonStatus = OnSetButtonStatus;
}

void CTcpClient::SetOnAnalyzerStatus(SetAnalyzerStatusFun OnSetAnalyzerStatue)
{
	m_OnSetAnalyzerStatus = OnSetAnalyzerStatue;
}

void CTcpClient::SetOnExeGetAnalyzerStatus(ExeGetAnalyzerStatusFun OnExeGetAnalyzerStatus)
{
	m_OnExeGetAnalyzerStatus = OnExeGetAnalyzerStatus;
}

void CTcpClient::SetOnCurrentProjectName(SetCurrentProjectNameFun OnSetCurrentProjectName)
{
	m_OnSetCurrentProjectName = OnSetCurrentProjectName;
}

void CTcpClient::setOnFinishCount(SetFinishCountFun OnSetFinishCount)
{
	m_OnSetFinishCount = OnSetFinishCount;
}

void CTcpClient::SetOnSendFileFail(SendFileFailFun OnSendFileFail)
{
	m_OnSendFileFail = OnSendFileFail;
}

void CTcpClient::SetOnSendFileProgress(SendFileProgressFun OnSendFileProgress)
{
	m_OnSendFileProgress = OnSendFileProgress;
}

void CTcpClient::SetTerminateStatus(BOOL b_Status)
{
	m_bTerminateStatus = b_Status;
}

void CTcpClient::SetFinishStatus(BOOL b_Status)
{
	m_bGetResponseFinish = b_Status;
}


//  初始化windows socket
BOOL CTcpClient::StartupSocket(void)
{
	int nErrCode = 0;

	if(m_WSAData.wVersion == 0)
	{
		nErrCode = WSAStartup(0x0101, &m_WSAData);
		if(nErrCode != 0)
			memset(&m_WSAData, 0, sizeof(WSADATA));
	}

	return (nErrCode == 0);
}

// 清除windows socket
void CTcpClient::CleanupSocket(void)
{
  if(m_WSAData.wVersion != 0)
    WSACleanup();

  memset(&m_WSAData, 0, sizeof(WSADATA));
}

void CTcpClient::InitSockAddr(SOCKADDR_IN  *pSockAddr, char *szBindIpAddr, WORD wPort)
{
	int AIp;

	memset(pSockAddr, 0, sizeof(SOCKADDR_IN));

	// 协议族
	pSockAddr->sin_family = PF_INET;
    
	// 绑定地址
	if(strcmp(szBindIpAddr, "") == 0)
		AIp = INADDR_NONE;
	else
		AIp = inet_addr(szBindIpAddr); // 转换点分十进制ip地址

    if(AIp == INADDR_NONE)
		pSockAddr->sin_addr.S_un.S_addr = INADDR_ANY; // 绑定到所有网络接口
    else
      pSockAddr->sin_addr.S_un.S_addr = AIp;

    pSockAddr->sin_port = htons(wPort); // 绑定端口
}

// 隐藏窗体消息处理函数
LRESULT CTcpClient::HideWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CTcpClient *pTcpClient = (CTcpClient *)lParam;	
	STATEINFO *pStateInfo = NULL;
	BTNSTATES *pBtnStates = NULL;
	COUNTINFO *pCountInfor = NULL;
	
	CString tempStr;
	int nValue;

	switch(uMsg)
	{
	case WM_SETCURRENTPROJECTNAME:
		pTcpClient->HandleSetCurrentProjectName((LPCTSTR)wParam);
		break;
	case WM_EXEGETTINGSTATUS:
		pStateInfo = (STATEINFO *)wParam;
		pTcpClient->HandleExeGetAnalyzerStatus(pStateInfo->indexSystem, pStateInfo->stateString);
		break;
	case WM_FINISHCOUNT:   //获取多线程完成数量
		pCountInfor = (COUNTINFO *)wParam;
		pTcpClient->HandleSetFinishCount(pCountInfor->iSource, pCountInfor->successFlag);
		break;
	case WM_ANALYZERINFO:	
		pStateInfo = (STATEINFO *)wParam;
		pTcpClient->HandleSetAnalyzerStatus(*pStateInfo);
		break;
	case WM_SETBUTTONSTATUS:
		pBtnStates = (BTNSTATES *)wParam;
		pTcpClient->HandleSetButtonStatus(pBtnStates->indexButton, pBtnStates->b_ButtonStates);
		break;
	case WM_CURSORTYPE:
		pTcpClient->HandleSetCursor((int)wParam);
		break;
	case WM_SOCKETCLOSE:// socket被关闭
		pTcpClient->HandleSocketCloseMsg();
		break;// socket被关闭
	case WM_ONLYCLOSESOCKET:// 仅仅关闭发送文件socket
		pTcpClient->HandleSocketOnlyCloseMsg();
		break;// socket被关闭
	case WM_DISCARDTHREAD:  //关闭线程
		pTcpClient->DiscardThread();
		break;
	case WM_SOCKETSENDERR: // socket发送出错
		pTcpClient->HandleSocketSendErrMsg();
		break;
	case WM_SOCKETRECVERR: // socket接收出错
		pTcpClient->HandleSocketRecvErrMsg();
		break;
	case WM_RECVERACCEPTFILE: // 接收方接受文件
		pTcpClient->HandleRecverAcceptFileMsg((DWORD)wParam);
		break;
	case WM_RECVERREFUSEFILE: // 接收方拒绝文件
		pTcpClient->HandleRecverRefuseFileMsg();
		break;
	case WM_RECVERSUCC: // 接收方接收文件成功
		pTcpClient->HandleRecverSuccMsg();
		break;
	case WM_RECVERFAIL: // 接收方失败
		pTcpClient->HandleRecverFailMsg();
		break;
	case WM_RECVERCANCEL: // 接收方取消
		pTcpClient->HandleRecverCancelMsg();
		break;
	case WM_SENDERFAIL: // 接收方失败
		pTcpClient->HandleSenderFailMsg();
		break;
	case WM_SENDFILEPROGRESS: // 文件发送进度
		pTcpClient->HandleSendFileProgressMsg();
		break;
	case WM_WAITTIMEOUT:
		pTcpClient->HandleWaitTimeOutMsg();
		break;
	case WM_OPENNVGATESUCC:
		pTcpClient->HandleOpenNVGateSuccMsg();
		break;
	case WM_OPENNVGATEFAIL:
		pTcpClient->HandleOpenNVGateFailMsg();
		break;
	case WM_SETSOFTMSG:
		pTcpClient->HandleGetSoftwareMsg((LPCTSTR)wParam);	
		break;
	case WM_STARTGETTINGINFO:
		pTcpClient->HandleGetSoftwareMsg(_T("Starting to get Local Information..."));
		break;
	case WM_STARTGETTINGINPUT:
		tempStr.Format(_T("Getting the informaiton of inputs... ---- %d / %d"), (int)wParam, pTcpClient->m_totalChannels);
		pTcpClient->HandleGetSoftwareMsg(tempStr);
		break;
	case WM_STARTGETTINGREC :
		tempStr.Format(_T("Getting the informaiton of recorder trackss... ---- %d / %d"), (int)wParam, DFT_MAXRECCHANNELCOUNT);
		pTcpClient->HandleGetSoftwareMsg(tempStr);
		break;
	case WM_STARTGETTINGFFT:
		tempStr.Format(_T("Getting the information of FFT channelss... ---- %d / %d"), (int)wParam, DFT_MAXINPUTCHANNELCOUNT);
		pTcpClient->HandleGetSoftwareMsg(tempStr);
		break;
	case WM_STARTGETTINGTDA:
		tempStr.Format(_T("Getting the informaiton of TDA channels... ---- %d / %d"), (int)wParam, DFT_MAXINPUTCHANNELCOUNT);
		pTcpClient->HandleGetSoftwareMsg(tempStr);
		break;
	case WM_STARTGETTINGORD:
		tempStr.Format(_T("Getting the information of ORD channels... ---- %d / %d"), (int)wParam, DFT_MAXINPUTCHANNELCOUNT);
		pTcpClient->HandleGetSoftwareMsg(tempStr);
		break;
	case WM_STARTGETTINGWTF:
		tempStr.Format(_T("Getting the information of waterfall channels... ---- %d / %d"), (int)wParam, 96);
		pTcpClient->HandleGetSoftwareMsg(tempStr);
		break;
	case WM_GETTINGINFOSUCC:
		pTcpClient->HandleGetSoftwareMsg(_T("Getting local inforamtion succeed!"));
		break;
	case WM_GETTINGINFOFAIL:
		pTcpClient->HandleGetSoftwareMsg(_T("Getting local inforamtion failed!"));
		break;
	case WM_GETSOFTWAREMSG:
		switch ((int)wParam)
		{
		case 6001:
			pTcpClient->HandleGetSoftwareMsg(_T("Connecting to NVGate succeed!"));
			break;
		case 6002:
			pTcpClient->HandleGetSoftwareMsg(_T("Saving current workbook..."));
			break;
		case 6003:
			pTcpClient->HandleGetSoftwareMsg(_T("Starting to get the configuration of Local NVGate..."));
			break;			
		case 6004:
			pTcpClient->HandleGetSoftwareMsg(_T("Getting the configuration of Local NVGate succeed!"));
			break;
		case 6005:
			pTcpClient->HandleGetSoftwareMsg(_T("Starting to generate the workbook models of each sub-system..."));
			break;
		case 6006:
			pTcpClient->HandleGetSoftwareMsg(_T("Loading original workbook..."));
			break;
		case 6007:
			tempStr.Format(_T("Re-connecting Inputs..."), (int)wParam);
			pTcpClient->HandleGetSoftwareMsg(tempStr);
			break;
		case 6008:
			tempStr.Format(_T("Resetting recorder tracks..."), (int)wParam);
			pTcpClient->HandleGetSoftwareMsg(tempStr);
			break;
		case 6009: case 6010: case 6011: case 6012:
			tempStr.Format(_T("Resetting FFT%d channels..."), (int)wParam-6008);
			pTcpClient->HandleGetSoftwareMsg(tempStr);
			break;
		case 6013:
			tempStr.Format(_T("Resetting TDA channels..."), (int)wParam);
			pTcpClient->HandleGetSoftwareMsg(tempStr);
			break;
		case 6014: case 6015:
			tempStr.Format(_T("Resetting ORD%d channels..."), (int)wParam-6013);
			pTcpClient->HandleGetSoftwareMsg(tempStr);
			break;
		case 6016:
			pTcpClient->HandleGetSoftwareMsg(_T("Resetting Waterfall channels..."));
			break;
		case 6017:
			pTcpClient->HandleGetSoftwareMsg(_T("Saving the workbook model of subsystem..."));
			break;
		case 6018:
			pTcpClient->HandleGetSoftwareMsg(_T("Generating the the workbook model of subsystem succeed!"));
			break;
		case 6019:
			pTcpClient->HandleGetSoftwareMsg(_T("Transferring the workbook files to each sub-system..."));
			break;
		case 6020:
			pTcpClient->HandleGetSoftwareMsg(_T("Attempting to connect the subsystems..."));
			break;
		case 6021:
			pTcpClient->HandleGetSoftwareMsg(_T("Connecting the subsystems succeed!"));
			break;
		case 6038:
			pTcpClient->HandleGetSoftwareMsg(_T("Connecting the subsystems failed!"));
			break;
		//case 6039:
		//	pTcpClient->HandleGetSoftwareMsg(_T("Open remote NVGate succeed!"));
		//	break;
		case 6022:
			pTcpClient->HandleGetSoftwareMsg(_T("Connecting to subsystem NVGate succeed!"));
			break;
		case 6023:
			pTcpClient->HandleGetSoftwareMsg(_T("Loading workbook model to remote NVGate..."));
			break;
		case 6024:
			pTcpClient->HandleGetSoftwareMsg(_T("Transferring the workbook files succeed!"));
			break;
		case 6025:
			pTcpClient->HandleGetSoftwareMsg(_T("Arming the recorder of all remote NVGate..."));
			break;
		case 6026:
			pTcpClient->HandleGetSoftwareMsg(_T("Getting analyzer states failed!"));
			break;
		case 6027:
			pTcpClient->HandleGetSoftwareMsg(_T("Stopping the analyzer out of time!"));
			break;
		case 6028:
			pTcpClient->HandleGetSoftwareMsg(_T("Starting Monitoring..."));
			break;
		case 6029:
			pTcpClient->HandleGetSoftwareMsg(_T("Starting Recording..."));
			break;
		case 6030:
			pTcpClient->HandleGetSoftwareMsg(_T("Stopping and Saving Recording..."));
			break;
		case 6031:
			pTcpClient->HandleGetSoftwareMsg(_T("Getting current project name..."));
			break;
		case 6032:
			pTcpClient->HandleGetSoftwareMsg(_T("<Default Project> cannot be the current project name..."));
			break;
		case 6033:
			pTcpClient->HandleGetSoftwareMsg(_T("The current project name is empty"));
			break;
		case 6034:
			pTcpClient->HandleGetSoftwareMsg(_T("Open remote NVGate succeed!"));
			break;
		case 6035:
			pTcpClient->HandleGetSoftwareMsg(_T("Open remote NVGate failed!"));
			break;
		case 6036:
			pTcpClient->HandleGetSoftwareMsg(_T("Attempting to connect remote NVGate..."));
			break;
		case 6037:
			pTcpClient->HandleGetSoftwareMsg(_T("Connecting to NVGate failed!"));
			break;
		case 6039:
			pTcpClient->HandleGetSoftwareMsg(_T("Connecting to remote NVGate succeed!"));
			break;
		case 6040:
			pTcpClient->HandleGetSoftwareMsg(_T("Connecting to remote NVGate failed!"));
			break;
		case 6041:
			pTcpClient->HandleGetSoftwareMsg(_T("Remote NVGate is closed!"));
			break;
		case 6042:
			pTcpClient->HandleGetSoftwareMsg(_T("Loading workbook to remote NVGate succeed!"));
			break;
		case 6043:
			pTcpClient->HandleGetSoftwareMsg(_T("Arming all the measumrent succeed!"));
			break;
		case 6044:
			pTcpClient->HandleGetSoftwareMsg(_T("Starting monitoring succeed!"));
			break;
		case 6045:
			pTcpClient->HandleGetSoftwareMsg(_T("Starting recording succeed!"));
			break;
		case 6046:
			pTcpClient->HandleGetSoftwareMsg(_T("Stopping and saving recording succeed!"));
			break;
		case 6047:
			pTcpClient->HandleGetSoftwareMsg(_T("Attempting to connect local NVGate..."));
			break;
		case 6048:
			pTcpClient->HandleGetSoftwareMsg(_T("Connecting to local NVGate succeed!"));
			break;
		case 6049:
			pTcpClient->HandleGetSoftwareMsg(_T("Connecting to local NVGate failed!"));
			break;
		case 6050:
			pTcpClient->HandleGetSoftwareMsg(_T("Local NVGate is closed!"));
			break;
		//case 6025:
		//	pTcpClient->SendDataToNVGateA(_T("SetSilent 1"));
		//	break;
		case 11:
			tempStr.Format(_T("Resetting Waterfall..."), (int)wParam);
			pTcpClient->HandleGetSoftwareMsg(tempStr);
			break;
		case 12:
			tempStr.Format(_T("Save model as a sub-model..."), (int)wParam);
			pTcpClient->HandleGetSoftwareMsg(tempStr);
			break;
		case 13:
			tempStr.Format(_T("Finishing generate sub model!"), (int)wParam);
			pTcpClient->HandleGetSoftwareMsg(tempStr);
			break;
		case 60:
			pTcpClient->HandleGetSoftwareMsg(_T("Connecting to Local NVGate failed!"));
			break;
		case 61:
			pTcpClient->HandleGetSoftwareMsg(_T("Connecting to Local NVGate succeed!"));
			break;
		case 62:
			pTcpClient->HandleGetSoftwareMsg(_T("Saving current workbook model as SuperModel..."));
			break;
		case 63:
			pTcpClient->HandleGetSoftwareMsg(_T("Generating subsystem workbook model..."));
			break;
		case 64:
			pTcpClient->HandleGetSoftwareMsg(_T("Connecting to Sub-System succeed!"));
			break;
		case 65:
			pTcpClient->HandleGetSoftwareMsg(_T("Connecting to Sub-System failed!"));
			break;
		case 66:
			pTcpClient->HandleGetSoftwareMsg(_T("Sending workbook file to Sub-System..."));
			break;
		case 67:
			pTcpClient->HandleGetSoftwareMsg(_T("Preparing the related files failed!"));
			break;

		default:
			break;
		}
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return 0;
}

BOOL CTcpClient::RegisterClientSocketHideWndClass(void)
{
	WNDCLASS wc;

	memset(&wc, 0, sizeof(WNDCLASS));
	wc.lpfnWndProc = (WNDPROC)HideWndProc;
	wc.hInstance = AfxGetInstanceHandle();
	wc.lpszClassName = STR_CLIENTSOCKETHIDEWNDCLASSNAME;

	return (RegisterClass(&wc) != 0);
}

// 创建隐含窗体
BOOL CTcpClient::AllocateWindow(void)
{
	if(m_hHideWnd == NULL)
	{
		m_hHideWnd = CreateWindow(STR_CLIENTSOCKETHIDEWNDCLASSNAME, NULL, WS_POPUP,  
			CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, AfxGetInstanceHandle(), this);
	}

	return (m_hHideWnd != NULL);
}

// 释放隐含窗体
void CTcpClient::DeallocateWindow(void)
{
	if(m_hHideWnd != NULL)
	{
		DestroyWindow(m_hHideWnd);
		m_hHideWnd = NULL;
	}
}

// 连接服务端，SOURCE = 1 FROM NVGATE
BOOL CTcpClient::Connect(int source)
{
	u_long			ulTmp;
	SOCKADDR_IN		SockAddr;
	
	Disconnect(); // 断开连接

	//if(!AllocateWindow())
	//	return FALSE;
	
	// 创建socket
	m_hSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
	if(m_hSocket == INVALID_SOCKET)
		goto ErrEntry;
	
	// 设置socket为阻塞方式
	ulTmp = 0;
	if(ioctlsocket(m_hSocket, FIONBIO, &ulTmp) != 0)
		goto ErrEntry;

	// 设置socket发送超时
	if(setsockopt(m_hSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&m_nSendTimeOut, 
		sizeof(m_nSendTimeOut)) != 0)
		goto ErrEntry;

	// //设置socket接受超时
	//if (source != 0)
	//{
	//	if (setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&m_nSendTimeOut,
	//			sizeof(m_nSendTimeOut)) != 0)
	//			goto ErrEntry;
	//}
		
	// 连接服务端
	InitSockAddr(&SockAddr, m_szServerIpAddr, m_wPort);
	if(connect(m_hSocket, (sockaddr *)&SockAddr, sizeof(SOCKADDR_IN)) != 0)
		goto ErrEntry;

	//显示连接信息
	m_OnSocketConnect(m_pNotifyObj, m_hSocket);

	SetTerminateStatus(FALSE);

	// 创建接收线程
	m_pClientSocketRecvThread = new CClientSocketRecvThread(this, m_nSocketRecvThreadPriority, source);
	m_pClientSocketRecvThread->SetSocketHandle(m_hSocket);
	m_pClientSocketRecvThread->SetHideWndHandle(m_hHideWnd);
	m_pClientSocketRecvThread->Resume();
		
	m_nSource = source;
	
	return TRUE;

ErrEntry:
	Disconnect();
	return FALSE;
}

// 断开连接
void CTcpClient::Disconnect(void)
{

	// 关闭socket连接
	if(m_hSocket != INVALID_SOCKET)
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}
	
	// 销毁接收线程
	if(m_pClientSocketRecvThread != NULL)
	{
		m_pClientSocketRecvThread->Terminate();
		WaitForSingleObject(m_pClientSocketRecvThread->GetThreadHandle(), INFINITE);
		delete m_pClientSocketRecvThread;
		m_pClientSocketRecvThread = NULL;
	}

}

//销毁信息获取、文件发送线程
void CTcpClient::DiscardThread(void)
{

	HandleSetCursor(0);

	// 清除生成子Workbook线程
	if (m_pGenerateModelThread != NULL)
	{
		m_pGenerateModelThread->CloseGenerateWorkbookThreadMute();
		delete m_pGenerateModelThread;
		m_pGenerateModelThread = NULL;
	}
	
	// 清除文件发送线程
	if (m_pSendFileThread != NULL)
	{
		m_pSendFileThread->CloseSendFileThreadMute();
		delete m_pSendFileThread;
		m_pSendFileThread = NULL;
	}

	// 清除文件发送线程
	if (m_pOperateRemoteNVGate != NULL)
	{
		m_pOperateRemoteNVGate->CloseOperateNVGateThreadMute();
		delete m_pOperateRemoteNVGate;
		m_pOperateRemoteNVGate = NULL;
	}

	// 清除文件发送线程
	if (m_pOpenRemoteNVGate != NULL)
	{
		m_pOpenRemoteNVGate->CloseOpenNVGateThreadMute();
		delete m_pOpenRemoteNVGate;
		m_pOpenRemoteNVGate = NULL;
	}

	// 清除Alarm Recorder线程
	if (m_pAlarmSystem != NULL)
	{
		m_pAlarmSystem->CloseAlarmSystemThreadMute();
		delete m_pAlarmSystem;
		m_pAlarmSystem = NULL;
	}

	// 清除Start Monitor线程
	if (m_pStartMonitor != NULL)
	{
		m_pStartMonitor->CloseStartMonitorThreadMute();
		delete m_pStartMonitor;
		m_pStartMonitor = NULL;
	}

	// 清除Start Monitor线程
	if (m_pStartRecord != NULL)
	{
		m_pStartRecord->CloseStartRecordThreadMute();
		delete m_pStartRecord;
		m_pStartRecord = NULL;
	}

	// 清除Start Monitor线程
	if (m_pStopRecord != NULL)
	{
		m_pStopRecord->CloseStopRecordThreadMute();
		delete m_pStopRecord;
		m_pStopRecord = NULL;
	}

	// 清除Start Monitor线程
	if (m_pGetAnalyzerStatus != NULL)
	{
		m_pGetAnalyzerStatus->CloseGetAnalyzerStatusThreadMute();
		delete m_pGetAnalyzerStatus;
		m_pGetAnalyzerStatus = NULL;
	}

	// 清除Start Monitor线程
	if (m_pRunAutoZero != NULL)
	{
		m_pRunAutoZero->CloseRunAutoZeroThreadMute();
		delete m_pRunAutoZero;
		m_pRunAutoZero = NULL;
	}

	// 清除Start Monitor线程
	if (m_pCloseNVGate != NULL)
	{
		m_pCloseNVGate->CloseNVGateThreadMute();
		delete m_pCloseNVGate;
		m_pCloseNVGate = NULL;
	}

	if (m_pGetFanStatus != NULL)
	{
		m_pGetFanStatus->CloseGetFanStatusThreadMute();
		delete m_pGetFanStatus;
		m_pGetFanStatus = NULL;
	}

	DeallocateWindow(); // 销毁窗体, 不再响应窗体消息
}


// 返回当前socket连接状态
BOOL CTcpClient::IsConnect(void)
{
	return (m_hSocket != INVALID_SOCKET);
}

// 判断是否在发送
BOOL CTcpClient::IsSending(void)
{
	return (m_pSendFileThread != NULL);
}

BOOL CTcpClient::SendFile(CString *ipAddress, int numofSystem, CString cfgName, int nPlace, HWND mHwnd)
{
	
	if(IsSending())
		return FALSE; // 正在发送文件

	//创建隐藏窗体
	if (!AllocateWindow())
		return FALSE;

	// 设置文件发送器的属性	
	m_pSendFileThread = new CSendFileThread(this, m_nSendFileThreadPriority);
	m_pSendFileThread->SetHideWndHandle(m_hHideWnd);
	m_pSendFileThread->SetHwnd(mHwnd);
	m_pSendFileThread->SetCfgName(cfgName);
	m_pSendFileThread->SetSendPlace(nPlace);   //0 = SmartRoute, 1 = Monitor
	//m_pSendFileThread->SetSocketHandle(m_hSocket);
	m_pSendFileThread->SetFilePackageSize(m_dwFilePackageSize);
	m_pSendFileThread->SetWaitTimeOut(m_nWaitTimeOut);
	m_pSendFileThread->SetProgressTimeInterval(m_dwProgressTimeInterval);
	
	m_pSendFileThread->PrepareSendWorkbookFiles(ipAddress, numofSystem);
	m_pSendFileThread->Resume(); // 开始发送
	
	return TRUE;
}

BOOL CTcpClient::GetGenerateWorkbook(int *numofChannel, CString *ipAddress, int numofSystem, int totalChannels, CString configName, HWND mHwnd)
{

	//创建隐藏窗体
	if (!AllocateWindow())
		return FALSE;

	////连接到本地NVGate	
	//SetServerIpAddr((char *)(LPCTSTR)_T("127.0.0.1"));
	//SetPort(3000);
	//if (!Connect(3)) 
	//	return FALSE;
	//

	// 设置生成子Model的属性
	m_pGenerateModelThread = new CGenerateModel(this, m_nSendFileThreadPriority);
	m_pGenerateModelThread->SetHwnd(mHwnd);
	m_pGenerateModelThread->SetHideWndHandle(m_hHideWnd);
	m_pGenerateModelThread->SetSocketHandle(m_hSocket);
	m_pGenerateModelThread->SetConfigName(configName);
	m_pGenerateModelThread->PrepareGetLocalInformation(numofChannel, ipAddress, numofSystem, totalChannels);
	m_totalChannels = totalChannels;
	m_pGenerateModelThread->Resume(); // 开始生成

	return TRUE;
}

BOOL CTcpClient::OperateRemoteNVGate(CString *ipAddress, int numofSystem, CString workbookName, HWND mHwnd)
{
	//创建隐藏窗体
	if (!AllocateWindow())
		return FALSE;

	// 设置生成子Model的属性
	m_pOperateRemoteNVGate = new COperateRemoteNVGate(this, m_nSendFileThreadPriority);
	m_pOperateRemoteNVGate->SetHideWndHandle(m_hHideWnd);
	m_pOperateRemoteNVGate->SetHwnd(mHwnd);
	m_pOperateRemoteNVGate->SetWorkbookName(workbookName);
	//m_pOperateRemoteNVGate->SetSocketHandle(m_hSocket);
	m_pOperateRemoteNVGate->SetIPAddress(ipAddress);
	m_pOperateRemoteNVGate->SetNumOfSystem (numofSystem);
	m_pOperateRemoteNVGate->Resume(); // 开始发送

	return TRUE;
}

BOOL CTcpClient::OpenRemoteNVGate(CString *ipAddress, int numofSystem, HWND mHwnd, CString *HardIpAddr, CString *HardType)
{
	//创建隐藏窗体
	if (!AllocateWindow())
		return FALSE;
	
	// 设置生成子Model的属性
	m_pOpenRemoteNVGate = new COpenRemoteNVGate(this, m_nSendFileThreadPriority);
	m_pOpenRemoteNVGate->SetHideWndHandle(m_hHideWnd);
	m_pOpenRemoteNVGate->SetHwnd(mHwnd);
	m_pOpenRemoteNVGate->SetHardIpAddr(HardIpAddr);
	m_pOpenRemoteNVGate->SetHardType(HardType);
	//m_pOperateRemoteNVGate->SetSocketHandle(m_hSocket);
	m_pOpenRemoteNVGate->SetIPAddress(ipAddress);
	m_pOpenRemoteNVGate->SetNumOfSystem(numofSystem);
	m_pOpenRemoteNVGate->Resume(); // 开始发送

	return TRUE;
}

BOOL CTcpClient::AlarmRecorder(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//创建隐藏窗体
	if (!AllocateWindow())
		return FALSE;

	// 设置生成子Model的属性
	m_pAlarmSystem = new CAlarmSystem(this, m_nSendFileThreadPriority);
	m_pAlarmSystem->SetHideWndHandle(m_hHideWnd);
	m_pAlarmSystem->SetHwnd(mHwnd);
	//m_pOperateRemoteNVGate->SetSocketHandle(m_hSocket);
	m_pAlarmSystem->SetIPAddress(ipAddress);
	m_pAlarmSystem->SetNumOfSystem(numofSystem);
	m_pAlarmSystem->Resume(); // 开始发送

	return TRUE;
}

BOOL CTcpClient::StartMonitor(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//创建隐藏窗体
	if (!AllocateWindow())
		return FALSE;

	// 设置生成子Model的属性
	m_pStartMonitor = new CStartMonitor(this, m_nSendFileThreadPriority);
	m_pStartMonitor->SetHideWndHandle(m_hHideWnd);
	m_pStartMonitor->SetHwnd(mHwnd);
	//m_pOperateRemoteNVGate->SetSocketHandle(m_hSocket);
	m_pStartMonitor->SetIPAddress(ipAddress);
	m_pStartMonitor->SetNumOfSystem(numofSystem);
	m_pStartMonitor->Resume(); // 开始发送

	return TRUE;
}

BOOL CTcpClient::StartRecord(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//创建隐藏窗体
	if (!AllocateWindow())
		return FALSE;

	// 设置生成子Model的属性
	m_pStartRecord = new CStartRecord(this, m_nSendFileThreadPriority);
	m_pStartRecord->SetHideWndHandle(m_hHideWnd);
	m_pStartRecord->SetHwnd(mHwnd);
	//m_pOperateRemoteNVGate->SetSocketHandle(m_hSocket);
	m_pStartRecord->SetIPAddress(ipAddress);
	m_pStartRecord->SetNumOfSystem(numofSystem);
	m_pStartRecord->Resume(); // 开始发送

	return TRUE;
}

BOOL CTcpClient::StopRecord(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//创建隐藏窗体
	if (!AllocateWindow())
		return FALSE;

	// 设置生成子Model的属性
	m_pStopRecord = new CStopRecord(this, m_nSendFileThreadPriority);
	m_pStopRecord->SetHideWndHandle(m_hHideWnd);
	m_pStopRecord->SetHwnd(mHwnd);
	//m_pOperateRemoteNVGate->SetSocketHandle(m_hSocket);
	m_pStopRecord->SetIPAddress(ipAddress);
	//m_pStopRecord-
	m_pStopRecord->SetNumOfSystem(numofSystem);
	m_pStopRecord->Resume(); // 开始发送

	return TRUE;
}

BOOL CTcpClient::GetAnalyzerStatus(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//创建隐藏窗体
	if (!AllocateWindow())
		return FALSE;
	
	// 设置生成子Model的属性
	m_pGetAnalyzerStatus = new CGetAnalyzerStatus(this, m_nSendFileThreadPriority);
	m_pGetAnalyzerStatus->SetHideWndHandle(m_hHideWnd);
	m_pGetAnalyzerStatus->SetHwnd(mHwnd);
	//m_pGetAnalyzerStatus->SetSocketHandle(m_hSocket);
	m_pGetAnalyzerStatus->SetConnectionFlag(TRUE);
	m_pGetAnalyzerStatus->SetStartFlag(TRUE);
	m_pGetAnalyzerStatus->SetIPAddress(ipAddress);
	//m_pStopRecord-
	m_pGetAnalyzerStatus->SetNumOfSystem(numofSystem);
	m_pGetAnalyzerStatus->Resume(); // 开始发送

	return TRUE;
}

BOOL CTcpClient::GetFanStatus(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//创建隐藏窗体
	if (!AllocateWindow())
		return FALSE;

	// 设置生成子Model的属性
	m_pGetFanStatus = new CFanStatus(this, m_nSendFileThreadPriority);
	m_pGetFanStatus->SetHideWndHandle(m_hHideWnd);
	m_pGetFanStatus->SetHwnd(mHwnd);
	//m_pGetAnalyzerStatus->SetSocketHandle(m_hSocket);
	m_pGetFanStatus->SetConnectionFlag(TRUE);
	m_pGetFanStatus->SetStartFlag(TRUE);
	m_pGetFanStatus->SetIPAddress(ipAddress);
	//m_pStopRecord-
	m_pGetFanStatus->SetNumOfSystem(numofSystem);
	m_pGetFanStatus->Resume(); // 开始发送

	return TRUE;
}

void CTcpClient::SetGettingStateFlag()
{
	//if (m_pGetAnalyzerStatus != NULL)
	//{
		m_pGetAnalyzerStatus->CloseGetAnalyzerStatusThreadMute();
	//}	
	//m_pGetAnalyzerStatus->Terminate();
	//m_pGetAnalyzerStatus->CloseGetAnalyzerStatusThreadMute();
}

void CTcpClient::SetGettingFanStateFlag(void)
{
	m_pGetFanStatus->CloseGetFanStatusThreadMute();
}

BOOL CTcpClient::RunAutoZero(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//创建隐藏窗体
	if (!AllocateWindow())
		return FALSE;

	// 设置生成子Model的属性
	m_pRunAutoZero = new CRunAutoZero(this, m_nSendFileThreadPriority);
	m_pRunAutoZero->SetHideWndHandle(m_hHideWnd);
	m_pRunAutoZero->SetHwnd(mHwnd);
	m_pRunAutoZero->SetIPAddress(ipAddress);
	//m_pStopRecord-
	m_pRunAutoZero->SetNumOfSystem(numofSystem);
	m_pRunAutoZero->Resume(); // 开始发送

	return TRUE;
}

BOOL CTcpClient::CloseNVGate(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//创建隐藏窗体
	if (!AllocateWindow())
		return FALSE;

	// 设置生成子Model的属性
	m_pCloseNVGate = new CCloseNVGate(this, m_nSendFileThreadPriority);
	m_pCloseNVGate->SetHideWndHandle(m_hHideWnd);
	m_pCloseNVGate->SetHwnd(mHwnd);
	m_pCloseNVGate->SetIPAddress(ipAddress);
	//m_pStopRecord-
	m_pCloseNVGate->SetNumOfSystem(numofSystem);
	m_pCloseNVGate->Resume(); // 开始发送

	return TRUE;
}

//int CTcpClient::SendDataToNVGateA(CString str_Command)
//{
//	return m_pGenerateModelThread->SendDataToNVGate(str_Command);	
//}

// 取消发送文件
void CTcpClient::CancelSendFile(void)
{
	if(IsSending())
	{
		m_pSendFileThread->CloseSendFileThreadCancel();
		delete m_pSendFileThread;
		m_pSendFileThread = NULL;
	}
}

//设置光标
void CTcpClient::HandleSetCursor(int cursorType)
{
	if (m_OnSetCursorType != NULL)
	{
		m_OnSetCursorType(m_pNotifyObj, cursorType);
	}
}

//设置按钮类型
void CTcpClient::HandleSetButtonStatus(int buttonID, BOOL buttonStatus)
{
	if (m_OnSetButtonStatus != NULL)
	{
		m_OnSetButtonStatus(m_pNotifyObj, buttonID, buttonStatus);
	}
}

// 发送网络消息
BOOL CTcpClient::SendNetMsg(char *Msg, int nMsgLen, int source)
{
	char szMsgLen[6];
	int nTotalLen;
	int nErr;
	int nSent = 0;

	if (source == 1)   //NVGate
	{
		while (nSent < nMsgLen)
		{
			nErr = send(m_hSocket, &(Msg[nSent]), nMsgLen - nSent, 0);
			if (nErr == SOCKET_ERROR)
				return FALSE;
			nSent += nErr;
		}
	}

	else   // File transfering
	{
		ASSERT(nMsgLen <= MAX_NETMSGPACKAGESIZE);

		Msg[0] = MSG_TAG;
		sprintf(szMsgLen, "%05d", nMsgLen);
		strncpy(&(Msg[1]), szMsgLen, 5);

		nTotalLen = nMsgLen + 6;
		while (nSent < nTotalLen)
		{
			nErr = send(m_hSocket, &(Msg[nSent]), nTotalLen - nSent, 0);
			if (nErr == SOCKET_ERROR)
				return FALSE;
			nSent += nErr;
		}
	}

	return TRUE;

}

// 处理socket发送出错窗体消息
// socket发送出错后强制关闭socket连接
void CTcpClient::HandleSocketSendErrMsg(void)
{
	if(IsSending() && m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_SOCKETSENDERR);

	if(IsConnect() && m_OnSocketClose != NULL)
		m_OnSocketClose(m_pNotifyObj, m_hSocket, SCR_SOCKETSENDERR);

	Disconnect();
	DiscardThread();
}

//// 处理socket接收出错窗体消息
void CTcpClient::HandleSocketRecvErrMsg(void)
{
	if(IsSending() && m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_SOCKETRECVERR);

	if(IsConnect () && m_OnSocketClose != NULL)
		m_OnSocketClose(m_pNotifyObj, m_hSocket, SCR_SOCKETRECVERR);

	Disconnect();
	DiscardThread();
}

// 处理socket关闭窗体消息
void CTcpClient::HandleSocketCloseMsg()
{
	
	//if(m_OnSendFileFail != NULL)
	//	m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_SOCKETCLOSE);
		
	//m_bTerminateStatus = TRUE;
	
	//if(m_OnSocketClose != NULL)
	//	m_OnSocketClose(m_pNotifyObj, m_hSocket, SCR_PEERCLOSE);

	SetTerminateStatus(TRUE);

	Disconnect(); // 断开socket连接，断开接受数据线程
	DiscardThread();//关闭发送文件线程
}

// 处理socket关闭窗体消息
void CTcpClient::HandleSocketOnlyCloseMsg(void)
{

	if (IsConnect() && m_OnSocketClose != NULL)
		m_OnSocketClose(m_pNotifyObj, m_hSocket, SCR_PEERCLOSE);

	Disconnect(); // 断开socket连接，断开接受数据线程
}


// 处理完整的网络消息
void CTcpClient::HandleOneNetMsg(char *Msg, int nMsgLen)
{
	switch (m_nSource)
	{
	//case 1:
	//	m_pGenerateModelThread->GetResString(Msg, nMsgLen);
	//	break;
	//case 2:
	//	m_pOperateRemoteNVGate->GetResString(Msg, nMsgLen);
	//	break;
	case 3:
		GetResString(Msg, nMsgLen);
		break;
	//case 4:
	//	m_pStartMonitor->GetResString(Msg, nMsgLen);
	//	break;
	//case 5:
	//	m_pStartRecord->GetResString(Msg, nMsgLen);
	//	break;
	//case 6:
	//	m_pStopRecord->GetResString(Msg, nMsgLen);
	//	break;
	case 0:
		if (m_OnOneNetMsg != NULL)
		{
			m_OnOneNetMsg(m_pNotifyObj, Msg, nMsgLen);
		}
		break;
	default:
		break;
	}
	
	
	//
	//if (m_nSource == 1)
	//{
	//	
	//}
	//else
	//{
	//	
	//}
}


// 处理接收端接受文件窗体消息
void CTcpClient::HandleRecverAcceptFileMsg(DWORD dwRecvedBytes)
{
	if(!IsSending())
		return;
	
	// 触发发送文件线程
	m_pSendFileThread->TrigAcceptFile(dwRecvedBytes);
}

// 处理接收端拒绝文件窗体消息
void CTcpClient::HandleRecverRefuseFileMsg(void)
{
	if(!IsSending())
		return;

	m_pSendFileThread->CloseSendFileThreadMute(); // 关闭文件发送线程
	
	if(m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_RECVERREFUSEFILE);

	delete m_pSendFileThread;
	m_pSendFileThread = NULL;
}

// 处理接收方接收文件成功窗体消息
void CTcpClient::HandleRecverSuccMsg(void)
{
	if(!IsSending())
		return;
	
	// 接收到该窗体消息时文件必需已经被接受
	if(!m_pSendFileThread->IsFileAccepted())
		return;

	m_pSendFileThread->TrigRecvFileSucc();

	//WaitForSingleObject(m_pSendFileThread->GetThreadHandle(), INFINITE); // 等待线程结束

	if(m_OnSendFileSucc != NULL)
		m_OnSendFileSucc(m_pNotifyObj, m_pSendFileThread->GetPathName());

	//delete m_pSendFileThread;
	//m_pSendFileThread = NULL;
}

void CTcpClient::HandleGetSoftwareMsg(CString Msg)
{
	if (m_OnOneMsg != NULL)
		m_OnOneMsg(m_pNotifyObj, Msg);
}

//void CTcpClient::HandleGetErrorMsg(CString Msg)
//{
//	if (m_OnOneErrorMsg != NULL)
//		m_OnOneErrorMsg(m_pNotifyObj, Msg);
//}


// 处理接收方失败窗体消息
void CTcpClient::HandleRecverFailMsg(void)
{
	if(!IsSending())
		return;

	m_pSendFileThread->CloseSendFileThreadMute(); // 关闭文件发送线程

	if(m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_RECVERFAIL);

	delete m_pSendFileThread;
	m_pSendFileThread = NULL;
}

// 处理接收方取消文件接收窗体消息
void CTcpClient::HandleRecverCancelMsg(void)
{
	if(!IsSending())
		return;

	m_pSendFileThread->CloseSendFileThreadMute(); // 关闭文件发送线程

	if(m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_RECVERCANCEL);

	delete m_pSendFileThread;
	m_pSendFileThread = NULL;
}

// 处理文件发送进度窗体消息
void CTcpClient::HandleSendFileProgressMsg(void)
{
	UWORD dwSentBytes; // 已发送字节数
	UWORD dwFileSize; // 文件大小

	if(!IsSending())
		return;

	if(m_OnSendFileProgress != NULL)
	{
		dwSentBytes = m_pSendFileThread->GetSentBytes();
		dwFileSize = m_pSendFileThread->GetFileSize();
		m_OnSendFileProgress(m_pNotifyObj, dwSentBytes, dwFileSize);
	}
}

// 处理文件发送失败窗体消息 
void CTcpClient::HandleSenderFailMsg(void)
{
	if(!IsSending())
		return;

	m_pSendFileThread->CloseSendFileThreadMute(); // 关闭文件发送线程

	if(m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_SENDERFAIL);

	delete m_pSendFileThread;
	m_pSendFileThread = NULL;
}

// 处理等待超时窗体消息
void CTcpClient::HandleWaitTimeOutMsg(void)
{
	if(!IsSending())
		return;

	m_pSendFileThread->CloseSendFileThreadMute(); // 关闭文件发送线程

	if(m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_WAITTIMEOUT);

	delete m_pSendFileThread;
	m_pSendFileThread = NULL;
}

// 处理等待超时窗体消息
void CTcpClient::HandleOpenNVGateSuccMsg(void)
{
	m_pOpenRemoteNVGate->OpenRemoteNVGateSucc();
}

// 处理等待超时窗体消息
void CTcpClient::HandleOpenNVGateFailMsg(void)
{
	m_pOpenRemoteNVGate->OpenRemoteNVGateFail();
}

// 处理主体关闭消息
void CTcpClient::HandleOpenNVGateMuteMsg(void)
{
	m_pOpenRemoteNVGate->OpenRemoteNVGateMute();
}

//处理分析仪状态
void CTcpClient::HandleSetAnalyzerStatus(STATEINFO sStateInfo)
{
	if (m_OnSetAnalyzerStatus != NULL)
		m_OnSetAnalyzerStatus(m_pNotifyObj,sStateInfo.indexSystem,sStateInfo.stateString,sStateInfo.indexStates);
}

//处理分析仪状态
void CTcpClient::HandleExeGetAnalyzerStatus(int indexSystem, CString analyzerStates)
{
	if (m_OnExeGetAnalyzerStatus != NULL)
		m_OnExeGetAnalyzerStatus(m_pNotifyObj, indexSystem, analyzerStates);
}

void CTcpClient::HandleSetCurrentProjectName(CString projectName)
{
	if (m_OnSetCurrentProjectName != NULL)
		m_OnSetCurrentProjectName(m_pNotifyObj, projectName);
}

//处理分析仪状态
void CTcpClient::HandleSetFinishCount(int iSource, int successFlag)
{
	if (m_OnSetFinishCount != NULL)
		m_OnSetFinishCount(m_pNotifyObj, iSource, successFlag);
}

//发送数据给NVGate，并收到反馈信息
//返回 -1：SOCKET断开；1：出现错误代码；0：正确执行
int CTcpClient::SendDataToNVGate(CString str_Command)
{
	char chr_Command[256];
	str_Command = str_Command + '\n';
	SetFinishStatus(FALSE);
	//m_bGetResponseFinish = FALSE;
	sprintf(chr_Command, "%s", str_Command);

	if (!m_bTerminateStatus)
	{
		SendNetMsg(chr_Command, strlen(chr_Command), 1);
	}
	else
	{
		return -1;  //SOCKET断开
	}

	while (!m_bGetResponseFinish)
	{
		if (!m_bTerminateStatus)
		{
			Sleep(1);
		}
		else
		{
			return -1;  //SOCKET断开
		}

	}

	if (*bDataFromNVGate == NVSUC_TAG)
	{
		/*bData = *bDataFromNVGate;
		msgLength = msgLen;*/
		return 0;   //正确执行
	}

	else return 1;    //出现错误
}

//处理收到的反馈信息
void CTcpClient::GetResString(char *Msg, int nMsgLen)
{
	bDataFromNVGate = Msg;
	msgLen = nMsgLen;
	SetFinishStatus(TRUE);
}

//获取字符串
void CTcpClient::MakeString(char* bDataFromNVGate, CString & szString)
{
	szString = bDataFromNVGate + m_nCurrent;
	m_nCurrent += szString.GetLength() + 1;
	return;
}

void CTcpClient::MakeInteger(char* bDataFromNVGATE, CString& SettingValue)
{
	short *a = (short*)(bDataFromNVGATE + m_nCurrent);
	SettingValue.Format("%d", *a);
}

void CTcpClient::MakeLongInteger(char* bDataFromNVGATE, CString& SettingValue)
{
	int *a = (int*)(bDataFromNVGATE + m_nCurrent);
	SettingValue.Format("%d", *a);
}

void CTcpClient::MakeFloatA(char* bDataFromNVGATE, CString& SettingValue)
{
	float *a = (float*)(bDataFromNVGATE + m_nCurrent);
	//if (*a >= 1){//之前为直接强转为int的代码，会把小数丢失
	//	int b = (int)(*a);
	//	SettingValue.Format("%d", b);
	//}
	//else if (*a<1){//为了维护原来的代码，此处
	//	SettingValue.Format("%f", *a);
	//}

	SettingValue.Format("%.1f", *a);

	return;
}

void CTcpClient::MakeFloat(char* bDataFromNVGATE, CString& SettingValue)
{
	float *a = (float*)(bDataFromNVGATE + m_nCurrent);
	//if (*a >= 1){//之前为直接强转为int的代码，会把小数丢失
	//	int b = (int)(*a);
	//	SettingValue.Format("%d", b);
	//}
	//else if (*a<1){//为了维护原来的代码，此处
	//	SettingValue.Format("%f", *a);
	//}

	SettingValue.Format("%.10f", *a);

	return;
}

//////////////////////////////////////////////////////////////////////
// CSendFileThread
//////////////////////////////////////////////////////////////////////
CSendFileThread::CSendFileThread(CTcpClient *pTcpClient, int nPriority): CThread(nPriority)
{
	m_pTcpClient = pTcpClient;
	m_hHideWnd = NULL;
	m_MainHwnd = NULL;
	m_hSocket = INVALID_SOCKET;
	m_dwFilePackageSize = DFT_FILEPACKAGESIZE;
	m_nWaitTimeOut = DFT_WAITTIMEOUT;
	m_dwProgressTimeInterval = DFT_PROGRESSTIMEINTERVAL;

	m_hCloseThreadMute = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hCloseThreadCancel = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hAcceptFile = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hRecvFileSucc = CreateEvent(NULL, TRUE, FALSE, NULL);
	//m_hOpenNVGateFail = CreateEvent(NULL, TRUE, FALSE, NULL);
	//m_hOpenNVGateSucc = CreateEvent(NULL, TRUE, FALSE, NULL);

	// multi read single write variants
	strcpy(m_szPathName, "");
	m_dwSentBytes = 0;
	m_dwFileSize = 0;
	m_dwLastProgressTick = GetTickCount();

	m_bBtnStates = new BTNSTATES();
	ci_CountInfo = new COUNTINFO();

	//m_nCurrent = 0;
	//m_numofSystem = 0;
	//m_totalChannel = 0;
	//msgLen = 0;
	//bDataFromNVGate = NULL;
	//m_bGetResponseFinish = FALSE;

	//获取当前的应用程序路径名，确定INI文件的完整文件名
	CString strAppPath = _T("");
	GetModuleFileName(NULL, strAppPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	strAppPath.ReleaseBuffer();
	int nPos = strAppPath.ReverseFind('\\');
	strAppPath = strAppPath.Left(nPos);
	iniFileName.Format(_T("%s\\Custom.ini"), strAppPath);

}

CSendFileThread::~CSendFileThread(void)
{
	CloseHandle(m_hCloseThreadMute);
	CloseHandle(m_hCloseThreadCancel);
	CloseHandle(m_hAcceptFile);
	CloseHandle(m_hRecvFileSucc);
	delete m_bBtnStates;
	delete ci_CountInfo;
	//CloseHandle(m_hOpenNVGateFail);
	//CloseHandle(m_hOpenNVGateSucc);
}

void CSendFileThread::SetHideWndHandle(HWND hHideWnd)
{
	m_hHideWnd = hHideWnd;
}

void CSendFileThread::SetSocketHandle(SOCKET hSocket)
{

	m_hSocket = hSocket;
}

void CSendFileThread::SetFilePackageSize(DWORD dwFilePackageSize)
{
	m_dwFilePackageSize = dwFilePackageSize;
}

void CSendFileThread::SetWaitTimeOut(int nWaitTimeOut)
{
	m_nWaitTimeOut = nWaitTimeOut;
}

void CSendFileThread::SetProgressTimeInterval(DWORD dwProgressTimeInterval)
{
	m_dwProgressTimeInterval = dwProgressTimeInterval;
}

void CSendFileThread::SetHwnd(HWND m_Hwnd)
{
	m_MainHwnd = m_Hwnd;
}

void CSendFileThread::SetCfgName(CString cfgName)
{
	m_sCfgName = cfgName;
}

void CSendFileThread::SetSendPlace(int nPlace)
{
	m_nSendPlace = nPlace;
}

char *CSendFileThread::GetPathName(void)
{
	return m_szPathName;
}

UWORD CSendFileThread::GetSentBytes(void)
{
	return m_dwSentBytes;
}

UWORD CSendFileThread::GetFileSize(void)
{
	return m_dwFileSize;
}

void CSendFileThread::ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	while(!m_bTerminated)
	{
		if(PostMessage(m_hHideWnd, Msg, wParam, lParam))
			return;
		if(GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
			return;
		Sleep(1000);
	}
}

// 线程执行函数
void CSendFileThread::Execute(void)
{
	
	int iret = 0;
	CString str_Temp = _T("");

	ASSERT(m_hHideWnd != NULL);

	if (m_nSendPlace == 0)
	{
		ci_CountInfo->iSource = 2;
		ci_CountInfo->successFlag = 0;
	}
	else
	{
		ci_CountInfo->iSource = 6;
		ci_CountInfo->successFlag = 0;
	}

	//获取WORKBOOK FOLDER
	char str_Content[64] = { "0" };
	GetPrivateProfileString(_T("Parameters"), _T("WorkbookFolder"), _T(""), str_Content, 64, iniFileName);
	if (_T(str_Content) == "")
	{
		MessageBox(m_MainHwnd, _T("Getting workbook folder failed, please check Custom.ini file."), _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
		//断开SOCKET，销毁线程
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}

	m_sWorkbookFolder = _T(str_Content);

	////鼠标光标为等待
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);

	////禁用按钮
	//m_bBtnStates->b_ButtonStates = FALSE;
	//for (int i = 1; i <= 3; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	//m_bBtnStates->indexButton = 1;
	//ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);

	//发送Workbook文件
	str_Temp.Format(_T("Transferring the workbook files to system #%d..."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	// 等待接收方接受文件
	HANDLE hWaitAcceptFileEvents[3];
	hWaitAcceptFileEvents[0] = m_hCloseThreadMute;
	hWaitAcceptFileEvents[1] = m_hCloseThreadCancel;
	hWaitAcceptFileEvents[2] = m_hAcceptFile;

	// 等待接收方接收文件成功
	HANDLE hWaitRecvFileSuccEvents[3];
	hWaitRecvFileSuccEvents[0] = m_hCloseThreadMute;
	hWaitRecvFileSuccEvents[1] = m_hCloseThreadCancel;
	hWaitRecvFileSuccEvents[2] = m_hRecvFileSucc;

	//for (int nsys = 1; nsys <= m_numofSystem ; nsys++)
	{
		//连接到远程计算机
		if (m_nSendPlace==0)
			str_Temp.Format(_T("Attempting to connect the SmartRoute PC of System #%d..."), m_numofSystem + 1);
		else
			str_Temp.Format(_T("Attempting to connect the Monitor PC #%d..."), m_numofSystem + 1);

		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);
		
		//端口号为8000
		m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
		m_pTcpClient->SetPort(8000);
		if (!m_pTcpClient->Connect(0))
		{
			//恢复鼠标光标
			//ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			

			if (m_nSendPlace == 0)
			{
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
				str_Temp.Format(_T("Connecting the SmartRoute PC of System #%d failed."), m_numofSystem + 1);
			}
			else
			{
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
				str_Temp.Format(_T("Connecting the Monitor PC #%d failed."), m_numofSystem + 1);
			}
			
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			if (m_nSendPlace == 0)
			{
				str_Temp.Format(_T("Connecting the SmartRoute PC of System #%d failed, please check the network connection."), m_numofSystem + 1);
			}
			else
			{
				str_Temp.Format(_T("Connecting the Monitor PC #%d failed, please check the network connection."), m_numofSystem + 1);
			}
			
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);

			//断开SOCKET，销毁线程
			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}
		else
		{			
			if (m_nSendPlace == 0)
			{
				str_Temp.Format(_T("Connecting the SmartRoute PC of System #%d succeed."), m_numofSystem + 1);
			}
			else
			{
				str_Temp.Format(_T("Connecting the Monitor PC #%d succeed."), m_numofSystem + 1);
			}
			//str_Temp.Format(_T("Connecting the SmartRoute PC of System #%d succeed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);
		}

		SetSocketHandle(m_pTcpClient->m_hSocket);
		ASSERT(m_hSocket != INVALID_SOCKET);

		//遍历所有文件
		intptr_t handle;
		_finddata_t findData;
		string pathName;
		CString fileName = _T("");
		
		CString dir = _T("");
		if (m_nSendPlace == 0)
		{
			dir.Format(_T("%s\\%s_%d"), m_sWorkbookFolder, m_sCfgName, m_numofSystem + 1);
		}
		else
		{
			dir.Format(_T("%s\\%s"), m_sWorkbookFolder, m_sCfgName);
		}
		

		//此路径不存在
		handle = _findfirst(pathName.assign(dir).append("\\*").c_str(), &findData);
		if (handle == -1)
		{
			//ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			if (m_nSendPlace == 0)
			{
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
				str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed."), m_numofSystem + 1);
			}
			else
			{
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
				str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed."), m_numofSystem + 1);
			}
				
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("The folder <%s_%d> in workbook folder doesn't exist, please click <Read and Generate> button firstly."), m_sCfgName, m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);

			//断开SOCKET，销毁线程
			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}
					
		do
		{
			//不是子目录，也不是.和..
			if (strcmp(findData.name, ".") != 0 && strcmp(findData.name, "..") != 0 && findData.attrib != _A_SUBDIR)
			{
				fileName.Format(_T("%s\\%s"), dir, _T(findData.name));

				if (!PrepareSendFile((LPSTR)(LPCSTR)fileName))
				{
					ResetSendFile();
					if (m_nSendPlace == 0)
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed."), m_numofSystem + 1);
					}
					else
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed."), m_numofSystem + 1);
					}

					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					if (m_nSendPlace == 0)
					{
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed, please check the network connection."), m_numofSystem + 1);
					}
					else
					{
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed, please check the network connection."), m_numofSystem + 1);
					}

					
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);


					//断开SOCKET，销毁线程
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
				}

				// 请求发送文件
				CString tempDir = _T("");
				//tempDir.Format("SuperModel_%d", nsys);
				if (!SendRequestSendFileNetMsg())
				{
					ResetSendFile();

					if (m_nSendPlace == 0)
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed."), m_numofSystem + 1);
					}
					else
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed."), m_numofSystem + 1);
					}

					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					if (m_nSendPlace == 0)
					{
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed, please check the network connection."), m_numofSystem + 1);
					}
					else
					{
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed, please check the network connection."), m_numofSystem + 1);
					}


					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);


					//断开SOCKET，销毁线程
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
				}

				// 等待接收方接受文件, 一旦接收方接受文件即开始发送文件数据
				switch (WaitForMultipleObjects(3, hWaitAcceptFileEvents, FALSE, m_nWaitTimeOut))
				{
				case WAIT_OBJECT_0: // 关闭线程
					ResetSendFile();

					if (m_nSendPlace == 0)
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed."), m_numofSystem + 1);
					}
					else
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed."), m_numofSystem + 1);
					}

					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					if (m_nSendPlace == 0)
					{
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed, please check the network connection."), m_numofSystem + 1);
					}
					else
					{
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed, please check the network connection."), m_numofSystem + 1);
					}


					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);


					//断开SOCKET，销毁线程
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;

					break;
				case WAIT_OBJECT_0 + 1: // 发送端取消导致线程关闭
					SendSenderCancelNetMsg();
					ResetSendFile();
					if (m_nSendPlace == 0)
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed."), m_numofSystem + 1);
					}
					else
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed."), m_numofSystem + 1);
					}

					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					if (m_nSendPlace == 0)
					{
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed, please check the network connection."), m_numofSystem + 1);
					}
					else
					{
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed, please check the network connection."), m_numofSystem + 1);
					}


					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);


					//断开SOCKET，销毁线程
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;

					break;
				case WAIT_OBJECT_0 + 2: // 接收端接受文件
					Sleep(1000);
					break;
				case WAIT_TIMEOUT: // 等待超时
					//ResetSendFile();
					//ForcePostMessage(WM_WAITTIMEOUT, 0, (LPARAM)m_pTcpClient);
					//if (!SendSenderFailNetMsg())
					//	ForcePostMessage(WM_SOCKETRECVERR, 0, (LPARAM)m_pTcpClient);
					//return;
					SendSenderFailNetMsg();
					ResetSendFile();
					if (m_nSendPlace == 0)
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed."), m_numofSystem + 1);
					}
					else
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed."), m_numofSystem + 1);
					}

					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					if (m_nSendPlace == 0)
					{
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed, please check the network connection."), m_numofSystem + 1);
					}
					else
					{
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed, please check the network connection."), m_numofSystem + 1);
					}


					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);


					//断开SOCKET，销毁线程
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;

					break;
				default: // 出错
					SendSenderFailNetMsg();
					ResetSendFile();
					if (m_nSendPlace == 0)
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed."), m_numofSystem + 1);
					}
					else
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed."), m_numofSystem + 1);
					}

					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					if (m_nSendPlace == 0)
					{
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed, please check the network connection."), m_numofSystem + 1);
					}
					else
					{
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed, please check the network connection."), m_numofSystem + 1);
					}


					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);


					//断开SOCKET，销毁线程
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
					break;
				}

				////禁用按钮
				//m_bBtnStates->b_ButtonStates = FALSE;
				//for (int i = 1; i <= 3; i++)
				//{
				//	m_bBtnStates->indexButton = i;
				//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
				//	Sleep(1);
				//}

				// 发送文件数据
				SendFileData();

				// 等待接收方接收文件成功
				switch(WaitForMultipleObjects(3, hWaitRecvFileSuccEvents, FALSE, m_nWaitTimeOut))
				{
				case WAIT_OBJECT_0: // 关闭线程
					ResetSendFile();
					if (m_nSendPlace == 0)
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed."), m_numofSystem + 1);
					}
					else
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed."), m_numofSystem + 1);
					}

					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					if (m_nSendPlace == 0)
					{
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed, please check the network connection."), m_numofSystem + 1);
					}
					else
					{
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed, please check the network connection."), m_numofSystem + 1);
					}


					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);


					//断开SOCKET，销毁线程
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
					break;
				case WAIT_OBJECT_0 + 1: // 发送端取消导致线程关闭
					SendSenderCancelNetMsg();
					ResetSendFile();
					if (m_nSendPlace == 0)
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed."), m_numofSystem + 1);
					}
					else
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed."), m_numofSystem + 1);
					}

					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					if (m_nSendPlace == 0)
					{
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed, please check the network connection."), m_numofSystem + 1);
					}
					else
					{
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed, please check the network connection."), m_numofSystem + 1);
					}


					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);


					//断开SOCKET，销毁线程
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
					break;
				case WAIT_OBJECT_0 + 2: // 接收端接收文件成功
					ResetSendFile();
					Sleep(1000);
					break;
				case WAIT_TIMEOUT: // 等待超时
					SendSenderFailNetMsg();
					ResetSendFile();
					if (m_nSendPlace == 0)
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed."), m_numofSystem + 1);
					}
					else
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed."), m_numofSystem + 1);
					}

					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					if (m_nSendPlace == 0)
					{
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed, please check the network connection."), m_numofSystem + 1);
					}
					else
					{
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed, please check the network connection."), m_numofSystem + 1);
					}


					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);


					//断开SOCKET，销毁线程
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
					break;
				default: // 出错
					SendSenderFailNetMsg();
					if (m_nSendPlace == 0)
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed."), m_numofSystem + 1);
					}
					else
					{
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed."), m_numofSystem + 1);
					}

					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					if (m_nSendPlace == 0)
					{
						str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d failed, please check the network connection."), m_numofSystem + 1);
					}
					else
					{
						str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d failed, please check the network connection."), m_numofSystem + 1);
					}


					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);


					//断开SOCKET，销毁线程
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
					break;
				}
			}
		} while (_findnext(handle, &findData) == 0);    // 查找目录中的下一个文件
	}



	//发送成功
	if (m_nSendPlace == 0)
	{
		str_Temp.Format(_T("Transferring the workbook files to SmartRoute PC of system #%d succeed."), m_numofSystem + 1);
	}
	else
	{
		str_Temp.Format(_T("Transferring the workbook files to Monitor PC #%d succeed."), m_numofSystem + 1);
	}
	ci_CountInfo->successFlag = 1;
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//if (m_nSendPlace == 0)
	//{
	//	//删除本地Model
	//	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)_T("127.0.0.1"));
	//	m_pTcpClient->SetPort(3000);
	//	if (!m_pTcpClient->Connect(3))
	//	{
	//		//恢复鼠标光标
	//		//ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
	//		
	//		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

	//		str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
	//		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	//		Sleep(1);

	//		str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed, please check the network connection."), m_numofSystem + 1);
	//		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);

	//		//断开SOCKET，销毁线程
	//		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
	//		return;
	//	}
	//	else
	//	{
	//		str_Temp.Format(_T("Connecting the remote NVGate of System #%d succeed."), m_numofSystem + 1);
	//		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	//		Sleep(1);
	//	}

	//	CString wkModelName = _T("");
	//	wkModelName.Format(_T("DeleteModel \\User %s_%d"), m_sCfgName, m_numofSystem + 1);

	//	iret = m_pTcpClient->SendDataToNVGate(wkModelName);
	//	if (iret == -1)  //SOCKET断开	
	//	{
	//		//恢复鼠标光标
	//		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
	//		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Local NVGate is closed."), (LPARAM)m_pTcpClient);
	//		Sleep(1);

	//		MessageBox(m_MainHwnd, _T("Local NVGate is closed, please open the remote NVGate or check the network connection."),
	//			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);

	//		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

	//		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
	//		return;
	//	}
	//}


	//if (m_nSendPlace == 0)
	{
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
	}
	
	//断开连接，销毁线程
	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);

}

// 准备发送文件
BOOL CSendFileThread::PrepareSendFile(char *szPathName)
{
	// 打开待发送的文件
	if (!m_File.Open(szPathName, CFile::modeRead | CFile::shareExclusive))
		return FALSE;
	
	// 获取文件大小
	try
	{
		m_dwFileSize = m_File.GetLength();
	}
	catch(...)
	{
		m_File.Abort();
		return FALSE;
	}

	strcpy(m_szPathName, szPathName); // 保存文件名
	m_dwSentBytes = 0; // 初始化已发送字节数

	return TRUE;
}

// 复位文件发送线程
void CSendFileThread::ResetSendFile(void)
{
	// 关闭文件
	if(m_File.m_hFile != CFile::hFileNull)
	{
		m_File.Close();
	}
}

// 检查文件是否已经被接收端接受
BOOL CSendFileThread::IsFileAccepted(void)
{
	return (WaitForSingleObject(m_hAcceptFile, 0) == WAIT_OBJECT_0);
}

// 发送文件数据
void CSendFileThread::SendFileData(void)
{
	UWORD dwTickCount;
	char Buf[MAX_NETMSGPACKAGESIZE + 6]; // 发送缓冲区
	int nBytes;

	// 消息头
	strncpy(&(Buf[6]), "-19", 3);

	// 发送进度
	ForcePostMessage(WM_SENDFILEPROGRESS, 0, (LPARAM)m_pTcpClient);

	while(!m_bTerminated && m_dwSentBytes < m_dwFileSize)
	{
		// 读取文件数据包
		nBytes = min(m_dwFilePackageSize, m_dwFileSize - m_dwSentBytes);
		try
		{
			m_File.Seek(m_dwSentBytes, CFile::begin);
			m_File.Read(&(Buf[9]), nBytes);
		}
		catch(...)
		{
			ResetSendFile();
			ForcePostMessage(WM_SENDERFAIL, 0, (LPARAM)m_pTcpClient);
			if(!SendSenderFailNetMsg())
				ForcePostMessage(WM_SOCKETSENDERR, 0, (LPARAM)m_pTcpClient);
			return;
		}

		// 发送
		if(!m_pTcpClient->SendNetMsg(Buf, 3 + nBytes,0))
		{
			ResetSendFile();
			ForcePostMessage(WM_SOCKETSENDERR, 0, (LPARAM)m_pTcpClient);
			return;
		}

		m_dwSentBytes += nBytes;

		// 发送进度
		dwTickCount = GetTickCount();
		if(dwTickCount < m_dwLastProgressTick) m_dwLastProgressTick = 0;
		if(dwTickCount - m_dwLastProgressTick > m_dwProgressTimeInterval || m_dwSentBytes >= m_dwFileSize)
		{
			ForcePostMessage(WM_SENDFILEPROGRESS, 0, (LPARAM)m_pTcpClient); // 文件接收进度
			m_dwLastProgressTick = dwTickCount;
		}
		else
			Sleep(0); // 强迫windows调度线程
	}
}

// 关闭文件发送线程
// 不发送任何网络消息给接收端
void CSendFileThread::CloseSendFileThreadMute(void)
{
	Terminate();
	SetEvent(m_hCloseThreadMute); // 关闭发送文件线程
	WaitForSingleObject(m_hThread, INFINITE); // 等待线程结束
}

// 关闭文件发送线程
// 发送取消发送文件网络消息给接收端
void CSendFileThread::CloseSendFileThreadCancel(void)
{
	Terminate();
	SetEvent(m_hCloseThreadCancel); // 关闭发送文件线程
	WaitForSingleObject(m_hThread, INFINITE); // 等待线程结束
}

// 发送请求发送文件网络消息
BOOL CSendFileThread::SendRequestSendFileNetMsg()
{
	int nFileNameLen;
	int nFolderNameLen;
	char Buf[19 + MAX_PATH];
	char szFileSize[13];

	CString tempString = m_File.GetFilePath();
	CString tempFileName = m_File.GetFileName();

	int nPos = tempString.ReverseFind('\\');
	CString str_filePath = tempString.Left(nPos);

	nPos = str_filePath.ReverseFind('\\');
	str_filePath = str_filePath.Right(str_filePath.GetLength() - nPos - 1);


	CString strFileName = _T("");
	strFileName.Format(_T("%s\\%s"), str_filePath,tempFileName);
	nFileNameLen = strFileName.GetLength();

	sprintf(szFileSize, "%012llu", m_File.GetLength());
	strncpy(&(Buf[6]), "-16", 3);
	strncpy(&(Buf[9]), szFileSize, 12);

	strncpy(&(Buf[21]), strFileName, nFileNameLen);

	return m_pTcpClient->SendNetMsg(Buf, 15 + nFileNameLen,0);
}

// 发送取消发送文件网络消息
BOOL CSendFileThread::SendSenderCancelNetMsg(void)
{
	char Msg[9];

	strncpy(&(Msg[6]), "-21", 3);
	return m_pTcpClient->SendNetMsg(Msg, 3,0);
}

// 发送发送文件失败网络消息
BOOL CSendFileThread::SendSenderFailNetMsg(void)
{
	char Msg[9];

	strncpy(&(Msg[6]), "-23", 3);
	return m_pTcpClient->SendNetMsg(Msg, 3,0);
}

// 接收端接受文件
void CSendFileThread::TrigAcceptFile(DWORD dwRecvedBytes)
{
	// 检查该消息的合法性
	if(WaitForSingleObject(m_hAcceptFile, 0) == WAIT_OBJECT_0)
		return;

	m_dwSentBytes = dwRecvedBytes;
	SetEvent(m_hAcceptFile);
}

// 接收端接收文件成功
void CSendFileThread::TrigRecvFileSucc(void)
{
	if(WaitForSingleObject(m_hRecvFileSucc, 0) == WAIT_OBJECT_0)
		return;

	SetEvent(m_hRecvFileSucc);
}


BOOL CSendFileThread::PrepareSendWorkbookFiles(CString *ipAddress, int numofSystem)
{
	m_pIpAddress = ipAddress;
	m_numofSystem = numofSystem;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// CClientSocketRecvThread
//////////////////////////////////////////////////////////////////////
CClientSocketRecvThread::CClientSocketRecvThread(CTcpClient *pTcpClient, int nPriority, int source):
CThread(nPriority)
{
	m_pTcpClient = pTcpClient;
	m_hSocket = INVALID_SOCKET;
	m_nRecvBufSize = 0;
	m_hHideWnd = NULL;
	m_source = source;
}

CClientSocketRecvThread::~CClientSocketRecvThread(void)
{
}

void CClientSocketRecvThread::SetSocketHandle(SOCKET hSocket)
{
	m_hSocket = hSocket;
}

void CClientSocketRecvThread::SetHideWndHandle(HWND hHideWnd)
{
	m_hHideWnd = hHideWnd;
}

void CClientSocketRecvThread::ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	while(!m_bTerminated)
	{
		if(PostMessage(m_hHideWnd, Msg, wParam, lParam))
			return;
		if(GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
			return;
		Sleep(1000);
	}
}

// 线程执行体
void CClientSocketRecvThread::Execute(void)
{
	char Buf[SOCKET_RECVBUFSIZE]; // 接收缓冲区
	int nErr;

	while(!m_bTerminated)
	{
		nErr = recv(m_hSocket, Buf, SOCKET_RECVBUFSIZE, 0);
		if(nErr == SOCKET_ERROR) // 有错误发生
		{
			nErr = WSAGetLastError();
			if(nErr == WSAECONNRESET) // TODO: 请帮忙测试有无其它值会引起socket复位
			{
				// socket连接被peer端关闭，虚电路复位等错误，出现此类型的错误时socket必须被关闭。
				//ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
				m_pTcpClient->SetTerminateStatus(TRUE);
				break;
			}
			else
			{				
				// 正常的socket错误，接收数据出错
				//ForcePostMessage(WM_SOCKETRECVERR, 0, (LPARAM)m_pTcpClient);
				m_pTcpClient->SetTerminateStatus(TRUE);
				return;
			}
		}
		else if(nErr == 0) // socket被被动关闭
		{
			//ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			m_pTcpClient->SetTerminateStatus(TRUE);
			break;
		}
		else // 接收到数据
		{
			DoRecv(Buf, nErr); // 处理接收到的数据
		}
	}; // while

}

// 将网络消息组装完整,然后调用相应的处理函数
void CClientSocketRecvThread::DoRecv(char *Buf, int nBytes)
{
	int i;
	char szMsgLen[11];
	int nMsgLen; // 消息长度
	int nTotalLen;

	// 拷贝数据到接收缓冲区
	if (m_nRecvBufSize + nBytes > NETMSG_RECVBUFSIZE)
		m_nRecvBufSize = 0; // 清空接收缓冲区中的数据
	memcpy(&(m_RecvBuf[m_nRecvBufSize]), Buf, nBytes);
	m_nRecvBufSize += nBytes;


	if (m_source == 0)   // From File transfering
	{
		while (!m_bTerminated)
		{
			if (m_nRecvBufSize == 0) return; // 缓冲区中无数据,退出

			// 缓冲区中第一个字节必须是消息标志
			if (m_RecvBuf[0] != MSG_TAG) // 第一个字节不是消息标志
			{
				for (i = 0; i < m_nRecvBufSize; i++)
				if (m_RecvBuf[i] == MSG_TAG) break;
				m_nRecvBufSize -= i;
				memcpy(m_RecvBuf, &(m_RecvBuf[i]), m_nRecvBufSize);
			}

			// 获取消息内容字节数
			if (m_nRecvBufSize < 6) return; // 消息长度未完整接收,退出
			strncpy(szMsgLen, &(m_RecvBuf[1]), 5);
			szMsgLen[5] = 0;
			nMsgLen = atoi(szMsgLen);
			nTotalLen = 1 + 5 + nMsgLen;

			// 判断消息是否完整接收
			if (nTotalLen > m_nRecvBufSize) return; // 消息没有接收完整
			HandleRecvedNetMsg(&(m_RecvBuf[6]), nMsgLen); // 处理该网络消息
			m_nRecvBufSize -= nTotalLen;
			memcpy(m_RecvBuf, &(m_RecvBuf[nTotalLen]), m_nRecvBufSize); // 从消息缓冲区中去除该消息
		};
	}
	else
	{
		while(!m_bTerminated)
		{
			if(m_nRecvBufSize == 0) return; // 缓冲区中无数据,退出

			// 缓冲区中第一个字节必须是消息标志
			if (m_RecvBuf[0] != NVSUC_TAG) // 第一个字节不是接受成功的标志
			{
				// 获取消息内容字节数
				if (m_nRecvBufSize < 10) return; // 消息长度未完整接收,退出
				nMsgLen = 10;
				nTotalLen = 10;
				HandleRecvedNetMsg(&(m_RecvBuf[0]), nMsgLen); // 处理该网络消息
				m_nRecvBufSize -= nTotalLen;
				memcpy(m_RecvBuf, &(m_RecvBuf[nTotalLen]), m_nRecvBufSize); // 从消息缓冲区中去除该消息
			}
			else
			{
				// 获取消息内容字节数
				if (m_nRecvBufSize < 10) return; // 消息长度未完整接收,退出

				strncpy(szMsgLen, &(m_RecvBuf[1]), 9);
				szMsgLen[10] = 0;
				nMsgLen = atoi(szMsgLen);
				nTotalLen = 10 + nMsgLen;

				// 判断消息是否完整接收
				if (nTotalLen > m_nRecvBufSize) return; // 消息没有接收完整
				HandleRecvedNetMsg(&(m_RecvBuf[0]), nTotalLen); // 处理该网络消息
				m_nRecvBufSize -= nTotalLen;
				memcpy(m_RecvBuf, &(m_RecvBuf[nTotalLen]), m_nRecvBufSize); // 从消息缓冲区中去除该消息
			}
		
		};
	}

}

// 处理接收到的网络消息
void CClientSocketRecvThread::HandleRecvedNetMsg(char *Msg, int nMsgLen)
{
	char szCmd[4];
	int nCmd;

	strncpy(szCmd, Msg, 3);
	szCmd[3] = 0;
	nCmd = atoi(szCmd);

	switch(nCmd)
	{
	case -17: // 接收端接受文件
		{
			DWORD dwSentBytes;
			char szSentBytes[11];

			strncpy(szSentBytes, &(Msg[3]), 10);
			szSentBytes[10] = 0;
			dwSentBytes = atoi(szSentBytes);
			ForcePostMessage(WM_RECVERACCEPTFILE, dwSentBytes, (LPARAM)m_pTcpClient);
		}
		break;
	case -18: // 接收端拒绝接受文件
		ForcePostMessage(WM_RECVERREFUSEFILE, 0, (LPARAM)m_pTcpClient);
		break;
	case -20: // 接收端接收文件成功
		ForcePostMessage(WM_RECVERSUCC, 0, (LPARAM)m_pTcpClient);
		break;
	case -22: // 接收端取消接收文件
		ForcePostMessage(WM_RECVERCANCEL, 0, (LPARAM)m_pTcpClient);
		break;
	case -24: // 接收端接收文件出错
		ForcePostMessage(WM_RECVERFAIL, 0, (LPARAM)m_pTcpClient);
		break;
	case -25: // 打开NVGATE成功
		ForcePostMessage(WM_OPENNVGATESUCC, 0, (LPARAM)m_pTcpClient);
		break;
	case -27: // 打开NVGATE成功
		ForcePostMessage(WM_OPENNVGATEFAIL, 0, (LPARAM)m_pTcpClient);
		break;
	default:
		m_pTcpClient->HandleOneNetMsg(Msg, nMsgLen);
		break;
	}
}

//////////////////////////////////////////////////////////////////////
// CGenerateModelThread
//////////////////////////////////////////////////////////////////////

CGenerateModel::CGenerateModel(CTcpClient *pTcpClient, int nPriority) : CThread(nPriority)
{
	m_pTcpClient = pTcpClient;
	m_hHideWnd = NULL;
	m_MainHwnd = NULL;
	m_hSocket = INVALID_SOCKET;

	m_nCurrent = 0;
	m_numofSystem = 0;
	m_totalChannel = 0;
	msgLen = 0;
	bDataFromNVGate = NULL;
	m_bBtnStates = new BTNSTATES();

	m_pIniFileReadWrite = new CIniFileReadWrite();
	//m_bGetResponseFinish = FALSE;

	//获取当前的应用程序路径名，确定INI文件的完整文件名
	CString strAppPath = m_pIniFileReadWrite->GetCurrentPath();

	/*GetModuleFileName(NULL, strAppPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	strAppPath.ReleaseBuffer();
	int nPos = strAppPath.ReverseFind('\\');
	strAppPath = strAppPath.Left(nPos);*/
	iniFileName = strAppPath + _T("LocalConfig.ini");
	tempIniFileName = strAppPath + _T("%s\\Tempsetting.ini");
}

CGenerateModel::~CGenerateModel()
{
	delete m_bBtnStates;
	delete m_pIniFileReadWrite;
}

void CGenerateModel::SetHideWndHandle(HWND hHideWnd)
{
	m_hHideWnd = hHideWnd;
}

void CGenerateModel::SetSocketHandle(SOCKET hSocket)
{

	m_hSocket = hSocket;
}

void CGenerateModel::SetConfigName(CString configName)
{
	m_strConfigName = configName;
}

void CGenerateModel::SetHwnd(HWND mHwnd)
{
	m_MainHwnd = mHwnd;
}

BOOL CGenerateModel::PrepareGetLocalInformation(int *numofChannel, CString *ipAddress, int numofSystem, int totalChannels)
{
	m_pNumofChannel = numofChannel;
	m_pIpAddress = ipAddress;
	m_numofSystem = numofSystem;
	m_totalChannel = totalChannels;	
	return TRUE;
}

void CGenerateModel::Execute(void)
{
	int iret = 0;
	CString str_Temp = _T("");

	ASSERT(m_hHideWnd != NULL);

	//鼠标光标为等待
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);
	m_pTcpClient->SetTerminateStatus(FALSE);

	//禁用按钮
	m_bBtnStates->b_ButtonStates = FALSE;
	for (int i = 1; i <= 4; i++)
	{
		m_bBtnStates->indexButton = i;
		ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
		Sleep(1);
	}

	//连接到本地NVGate
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Attempting to connect local NVGate..."), (LPARAM)m_pTcpClient);
	Sleep(1);

	//端口号为3000
	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)_T("127.0.0.1"));
	m_pTcpClient->SetPort(3000);
	if (!m_pTcpClient->Connect(3))
	{
		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Connecting local NVGate failed."), (LPARAM)m_pTcpClient);
		Sleep(1);

		MessageBox(m_MainHwnd, _T("Connecting to the local NVGate failed, please open the remote NVGate or check the network connection."), 
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//启用按钮
		m_bBtnStates->b_ButtonStates = TRUE;
		for (int i = 1; i <= 4; i++)
		{
			m_bBtnStates->indexButton = i;
			ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			Sleep(1);
		}
		//断开SOCKET，销毁线程
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}
	else
	{
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Connecting local NVGate succeed."), (LPARAM)m_pTcpClient);
		Sleep(1);
	}

	SetSocketHandle(m_pTcpClient->m_hSocket);

	ASSERT(m_hSocket != INVALID_SOCKET);

	//获取当前项目名称
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting current project name of local NVGate..."), (LPARAM)m_pTcpClient);
	iret = m_pTcpClient->SendDataToNVGate(_T("GetCurrentProjectName"));
	if (iret == -1)  //SOCKET断开	
	{
		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Local NVGate is closed."), (LPARAM)m_pTcpClient);
		Sleep(1);

		MessageBox(m_MainHwnd, _T("Local NVGate is closed, please open the remote NVGate or check the network connection."), 
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		
		//启用按钮
		m_bBtnStates->b_ButtonStates = TRUE;
		for (int i = 1; i <= 4; i++)
		{
			m_bBtnStates->indexButton = i;
			ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			Sleep(1);
		}
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}

	m_pTcpClient->m_nCurrent = 10;
	m_pTcpClient->MakeString(m_pTcpClient->bDataFromNVGate, str_Temp);

	if (str_Temp == _T("Default Project"))
	{
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("The current project cannot be <Default Project>."), (LPARAM)m_pTcpClient);
		Sleep(1);

		MessageBox(m_MainHwnd, _T("The current project cannot be <Default Project>, please save as a new name."),
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		
		//启用按钮
		m_bBtnStates->b_ButtonStates = TRUE;
		for (int i = 1; i <= 4; i++)
		{
			m_bBtnStates->indexButton = i;
			ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			Sleep(1);
		}
		//断开连接，销毁线程，关闭隐藏窗体
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}
	else
	{
		m_pIniFileReadWrite->WriteValue(_T("Project"), _T("Name"), str_Temp);
		//::WritePrivateProfileStringA(_T("Project"), _T("Name"), str_Temp, iniFileName);
	}

	
	//Load Workbook
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Loading workbook model..."), (LPARAM)m_pTcpClient);
	Sleep(1);

	iret = m_pTcpClient->SendDataToNVGate(_T("SetSilent 1"));
	if (iret == -1)  //SOCKET断开	
	{
		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Local NVGate is closed."), (LPARAM)m_pTcpClient);
		Sleep(1);

		MessageBox(m_MainHwnd, _T("Local NVGate is closed, please open the remote NVGate or check the network connection."),
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		
		//启用按钮
		m_bBtnStates->b_ButtonStates = TRUE;
		for (int i = 1; i <= 4; i++)
		{
			m_bBtnStates->indexButton = i;
			ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			Sleep(1);
		}
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}

	str_Temp.Format(_T("LoadWorkbookModel %s User"), m_strConfigName);
	iret = m_pTcpClient->SendDataToNVGate(str_Temp);
	if (iret == -1)  //SOCKET断开	
	{
		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Local NVGate is closed."), (LPARAM)m_pTcpClient);
		Sleep(1);
		MessageBox(m_MainHwnd, _T("Local NVGate is closed, please open the remote NVGate or check the network connection."),
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		
		//启用按钮
		m_bBtnStates->b_ButtonStates = TRUE;
		for (int i = 1; i <= 4; i++)
		{
			m_bBtnStates->indexButton = i;
			ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			Sleep(1);
		}
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}

	//获取本地NVGate的信息

	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting the configuration of local NVGate..."), (LPARAM)m_pTcpClient);
	Sleep(1);
	
	iret = GetLocalNVGateData();
	if (iret == -1)  //SOCKET断开	
	{
		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Local NVGate is closed."), (LPARAM)m_pTcpClient);
		Sleep(1);

		MessageBox(m_MainHwnd, _T("Local NVGate is closed, please open the remote NVGate or check the network connection."),
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		
		//启用按钮
		m_bBtnStates->b_ButtonStates = TRUE;
		for (int i = 1; i <= 4; i++)
		{
			m_bBtnStates->indexButton = i;
			ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			Sleep(1);
		}
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}
	else
	{
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting the configuration of local NVGate succeed."), (LPARAM)m_pTcpClient);
		Sleep(1);
	}

	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Generating workbook models for each system..."), (LPARAM)m_pTcpClient);
	Sleep(1);

	iret = GenerateSubModel();
	if (iret == -1)  //SOCKET断开	
	{
		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Local NVGate is closed."), (LPARAM)m_pTcpClient);
		Sleep(1);
		MessageBox(m_MainHwnd, _T("Local NVGate is closed, please open the remote NVGate or check the network connection."),
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		
		//启用按钮
		m_bBtnStates->b_ButtonStates = TRUE;
		for (int i = 1; i <= 4; i++)
		{
			m_bBtnStates->indexButton = i;
			ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			Sleep(1);
		}
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}
	else
	{
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Generating workbook models for each system succeed."), (LPARAM)m_pTcpClient);
		Sleep(1);
		//ForcePostMessage(WM_FINISHCOUNT, 6, (LPARAM)m_pTcpClient);

	}
	
	//断开连接，销毁线程，关闭隐藏窗体
	//启用按钮
	m_bBtnStates->b_ButtonStates = TRUE;
	for (int i = 1; i <= 4; i++)
	{
		m_bBtnStates->indexButton = i;
		ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
		Sleep(1);
	}

	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Reading and generating workbook model of local NVGate succeed."), (LPARAM)m_pTcpClient);
	Sleep(1);

	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// 关闭生成子Workbook发送线程
void CGenerateModel::CloseGenerateWorkbookThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // 等待线程结束
}

//获取本地NVGate的WORKBOOK配置信息
int CGenerateModel::GetLocalNVGateData(void)
{
	int iret = 0;
	CString b[5] = { _T("") };
	int iflag = 0;
	char* next_b = NULL;

	CString strValue = _T("");
	CString sectionName = _T("");
	CString keyName = _T("");

	//int moduleID[5] = { 0, 10, 11, 18, 19 };
	CString FFTChIndex[5][DFT_MAXINPUTCHANNELCOUNT + 1] = { _T("") };
	CString ORDChIndex[3][DFT_MAXINPUTCHANNELCOUNT + 1] = { _T("") };
	CString TDAChIndex[DFT_MAXINPUTCHANNELCOUNT + 1] = { _T("") };

	//获取Input的信息，包括：Label，Input Type, Transducer, Physical Qty
	//Sensitivity, Range Peak, Polarity, Offset, Coupling, Sampling Rate
	//Gauge Type, Bridge Resistance, Bridge Gain, Bridge Offset, Input Filter
	//Auto Range, AutoZero

	//前面32个通道的参数和后面通道不同，分开获取
	CString tempStr = _T("");

	int numberofTO = 0;
	BOOL bFirstFlag = TRUE;//对于Tracked Order，倒序，以第一次读到非0

	CString isActive = _T("0");   //输入通道是否激活

	int* startIndex = new int;
	int* indexChannel = new int;

	CString inputParaName[17] = { _T("Label"), _T("Input Type"), _T("Transducer"), _T("Physical Qty"),
		_T("Sensitivity"), _T("Range Peak"), _T("Polarity"), _T("Offset Comp"),
		_T("Coupling"), _T("Sampling Rate"), _T("Gauge Type"), _T("Bridge Resistance"),
		_T("Bridge Gain"), _T("Bridge Offset"), _T("Input Filter"), _T("Auto Range"),
		_T("Auto Zero") };
	CString strParameterArray[17] = { _T("label"), _T("inputType"), _T("transducer"), _T("physicalQty"),
		_T("sensitivity"), _T("rangePk"), _T("polarity"), _T("offset"),
		_T("coupling"), _T("869"), _T("gaugeType"), _T("bridgeResistance"),
		_T("bridgeGain"), _T("bridgeOffset"), _T("inputFilter"),
		_T("enableAutorange"), _T("enableBridgeAutoZero") };

	//ForcePostMessage(WM_STARTGETTINGINPUT, 0, (LPARAM)m_pTcpClient);

	//输入通道信息读取
	for (int i = 1; i <= m_totalChannel; i++)
	{
		ForcePostMessage(WM_STARTGETTINGINPUT, i, (LPARAM)m_pTcpClient);

		tempStr.Format(_T("frontEnd input%d active"), i);
		iret = GetSettingValue(tempStr, isActive);
		//SOCKET断开，或者返回错误代码
		if (iret == -1 || iret == 1)
		{
			delete startIndex;
			delete indexChannel;
			startIndex = NULL;
			indexChannel = NULL;
			return iret;
		}

		*startIndex = 0;
		*indexChannel = 0;

		GetIndex(i, *startIndex, *indexChannel, 0, m_numofSystem, m_pNumofChannel);
		sectionName.Format(_T("SYS%d_Input%d"), *startIndex, *indexChannel);
		//sectionName.Format(_T("Input%d"), i);

		//tempStr.Format(_T("%d"), *startIndex);
		//::WritePrivateProfileStringA(sectionName, _T("SystemIndex"), tempStr, iniFileName);
		//tempStr.Format(_T("%d"), *indexChannel);
		//::WritePrivateProfileStringA(sectionName, _T("ChIndex"), tempStr, iniFileName);

		if (isActive == _T("1"))
		{
			//输入通道激活
			::WritePrivateProfileStringA(sectionName, _T("Active"), "ON", iniFileName);
			////转速通道激活
			//::WritePrivateProfileStringA(sectionName, _T("REC"), "ON", iniFileName);

			for (int j = 0; j < 17; j++)
			{
				tempStr.Format("frontEnd input%d %s", i, strParameterArray[j]);
				iret = GenNVGatePara(tempStr, i, inputParaName[j], _T("Input"));
				if (iret == -1)
				{
					delete startIndex;
					delete indexChannel;
					startIndex = NULL;
					indexChannel = NULL;
					return -1;
				}
			}

			//读取EXPANDXPOD的信息
			int indexXPod = 0;
			if ((int)i % 8 == 0)
			{
				indexXPod = i / 8;
			}
			else
			{
				indexXPod = i / 8 + 1;
			}

			tempStr.Format(_T("frontEnd expander%d xpType"), indexXPod);
			
			iret = GetSettingValue(tempStr, str_Setting);
			if (iret == -1)
			{
				delete startIndex;
				delete indexChannel;
				startIndex = NULL;
				indexChannel = NULL;
				return -1;
			}

			::WritePrivateProfileStringA(sectionName, _T("XPType"), str_Setting, iniFileName);

			tempStr.Format(_T("frontEnd expander%d bridgeMaxCurrent"), indexXPod);
			iret = GetSettingValue(tempStr, str_Setting);
			if (iret == -1)
			{
				delete startIndex;
				delete indexChannel;
				startIndex = NULL;
				indexChannel = NULL;
				return -1;
			}

			::WritePrivateProfileStringA(sectionName, _T("BridgeMaxCurrent"), str_Setting, iniFileName);

			tempStr.Format(_T("frontEnd expander%d bridgeVoltage"), indexXPod);
			iret = GetSettingValue(tempStr, str_Setting);
			if (iret == -1)
			{
				delete startIndex;
				delete indexChannel;
				startIndex = NULL;
				indexChannel = NULL;
				return -1;
			}

			::WritePrivateProfileStringA(sectionName, _T("BridgeVoltage"), str_Setting, iniFileName);
		
		}
		else
		{
			::WritePrivateProfileStringA(sectionName, _T("Active"), "OFF", iniFileName);
		}

	}

	//Recorder 判断转速是否增加进来，总共228个通道
	//前面38个通道的参数不一样
	//如果有多于2个转速，以后再添加

	for (int i = 1; i <= DFT_MAXRECCHANNELCOUNT; i++)
	{
		ForcePostMessage(WM_STARTGETTINGREC, i, (LPARAM)m_pTcpClient);

		//获取记录仪通道的Active
		tempStr.Format(_T("recorder track%d active"), i);
		iret = GetSettingValue(tempStr, str_Setting);
		if (iret == -1)
		{
			delete startIndex;
			delete indexChannel;
			startIndex = NULL;
			indexChannel = NULL;
			return -1;
		}

		//通道数超出了授权权限，退出REC的循环
		if (iret == 1)
		{
			break;
		}

		if (str_Setting == _T("1"))
		{
			//获取REC中的source，得到对应的通道数
			tempStr.Format("recorder track%d 100", i);
			iret = GetSettingValue(tempStr, str_Setting);
			if (iret == -1)
			{
				delete startIndex;
				delete indexChannel;
				startIndex = NULL;
				indexChannel = NULL;
				return -1;
			}

			*startIndex = 0;
			*indexChannel = 0;

			if (str_Setting == _T("001.066.000"))
			{
				::WritePrivateProfileStringA(_T("Ext. Sync. 1"), _T("REC"), _T("ON"), iniFileName);
				//if (i <= 38)
				//{
				//	tempStr.Format("%d %d 014", 3, 364 + i);
				//}
				//else
				//{
					tempStr.Format("recorder track%d 14", i);
				//}
				iret = GetSettingValue(tempStr, str_Setting);
				if (iret == -1)
				{
					delete startIndex;
					delete indexChannel;
					startIndex = NULL;
					indexChannel = NULL;
					return -1;
				}
				::WritePrivateProfileStringA(_T("Ext. Sync. 1"), _T("REC.Sampling"), str_Setting, iniFileName);
			}
			else if (str_Setting == _T("001.067.000"))
			{
				::WritePrivateProfileStringA(_T("Ext. Sync. 2"), _T("REC"), _T("ON"), iniFileName);
				//if (i <= 38)
				//{
				//	tempStr.Format("%d %d 014", 3, 364 + i);
				//}
				//else
				//{
					tempStr.Format("recorder track%d 14", i);
				//}
				iret = GetSettingValue(tempStr, str_Setting);
				if (iret == -1)
				{
					delete startIndex;
					delete indexChannel;
					startIndex = NULL;
					indexChannel = NULL;
					return -1;
				}
				::WritePrivateProfileStringA(_T("Ext. Sync. 2"), _T("REC.Sampling"), str_Setting, iniFileName);
			}
			else if (str_Setting == _T("001.068.000"))
			{
				::WritePrivateProfileStringA(_T("Ext. Sync. 3"), _T("REC"), _T("ON"), iniFileName);
				//if (i <= 38)
				//{
				//	tempStr.Format("%d %d 014", 3, 364 + i);
				//}
				//else
				//{
				tempStr.Format("recorder track%d 14", i);
				//}
				iret = GetSettingValue(tempStr, str_Setting);
				if (iret == -1)
				{
					delete startIndex;
					delete indexChannel;
					startIndex = NULL;
					indexChannel = NULL;
					return -1;
				}
				::WritePrivateProfileStringA(_T("Ext. Sync. 3"), _T("REC.Sampling"), str_Setting, iniFileName);
			}
			else if (str_Setting == _T("001.069.000"))
			{
				::WritePrivateProfileStringA(_T("Ext. Sync. 4"), _T("REC"), _T("ON"), iniFileName);
				//if (i <= 38)
				//{
				//	tempStr.Format("%d %d 014", 3, 364 + i);
				//}
				//else
				//{
				tempStr.Format("recorder track%d 14", i);
				//}
				iret = GetSettingValue(tempStr, str_Setting);
				if (iret == -1)
				{
					delete startIndex;
					delete indexChannel;
					startIndex = NULL;
					indexChannel = NULL;
					return -1;
				}
				::WritePrivateProfileStringA(_T("Ext. Sync. 4"), _T("REC.Sampling"), str_Setting, iniFileName);
			}
			else if (str_Setting == _T("none"))
			{
				continue;
			}
			else
			{
				GetIndex(GetCHIndexFromString(str_Setting), *startIndex, *indexChannel, 0, m_numofSystem, m_pNumofChannel);
				sectionName.Format(_T("SYS%d_Input%d"), *startIndex, *indexChannel);
				::WritePrivateProfileStringA(sectionName, _T("REC"), "ON", iniFileName);

				tempStr.Format("recorder track%d 14", i);
				iret = GetSettingValue(tempStr, str_Setting);
				if (iret == -1)
				{
					delete startIndex;
					delete indexChannel;
					startIndex = NULL;
					indexChannel = NULL;
					return -1;
				}
				::WritePrivateProfileStringA(sectionName, _T("REC.Sampling"), str_Setting, iniFileName);

			}
		} //当前Recorder Track已经Active		
	}

	//读取FFT的参数，並把相关的信息写入到对应的INPUT中
	//FFT的通道数为32通道
	for (int x = 1; x <= 4; x++)   //4个FFT模块
	{
		for (int i = 1; i <= DFT_MAXINPUTCHANNELCOUNT; i++)
		{
			//tempStr.Format(_T("Getting the information of FFT%d channels ---- %d / %d"), x, i, DFT_MAXINPUTCHANNELCOUNT);
			//SendSoftwareMessage(tempStr);

			ForcePostMessage(WM_STARTGETTINGFFT, i, (LPARAM)m_pTcpClient);
			//m_pTcpClient->HandleGetSoftwareMsg(tempStr);

			//获取FFT Channel是否激活，如果没有激活就不用去获取了
			tempStr.Format(_T("fft%d channel%d active"), x, i);
			iret = GetSettingValue(tempStr, str_Setting);

			//Socket断开，网络通讯出现问题
			if (iret == -1)
			{
				delete startIndex;
				delete indexChannel;
				startIndex = NULL;
				indexChannel = NULL;
				return -1;
			}

			//返回错误代码，应该是通过授权超出，退出本次循环
			if (iret == 1)
			{
				break; //退出当前FFT的通道循环
			}

			if (str_Setting == _T("1"))
			{
				//获取FFT CHANNLE 的Source
				tempStr.Format("fft%d channel%d 100", x, i);
				iret = GetSettingValue(tempStr, str_Setting);

				//Socket断开，网络通讯出现问题
				if (iret == -1)
				{
					delete startIndex;
					delete indexChannel;
					startIndex = NULL;
					indexChannel = NULL;
					return -1;
				}

				//SOURCE获取正常
				if (str_Setting != _T("none") && str_Setting != _T(""))
				{
					*startIndex = 0;
					*indexChannel = 0;

					//解析出通道号，并得到子系统和子系统中的通道号
					GetIndex(GetCHIndexFromString(str_Setting), *startIndex, *indexChannel, 0, m_numofSystem, m_pNumofChannel);
					sectionName.Format(_T("SYS%d_Input%d"), *startIndex, *indexChannel);
					FFTChIndex[x][i] = sectionName;

					//FFT标志为ON，即为开启状态
					keyName.Format(_T("FFT%d"), x);
					::WritePrivateProfileStringA(sectionName, keyName, "ON", iniFileName);

					//获取该通道的FFT的Input Filter的参数
					tempStr.Format("fft%d channel%d inputFilter", x, i);
					iret = GetSettingValue(tempStr, str_Setting);
					//Socket断开，网络通讯出现问题
					if (iret == -1)
					{
						delete startIndex;
						delete indexChannel;
						startIndex = NULL;
						indexChannel = NULL;
						return -1;
					}
					//获取正常，写入
					keyName.Format(_T("FFT%d.InputFilter"), x);
					::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

					//获取该通道的FFT的Weighting Window的参数
					tempStr.Format("fft%d channel%d 082", x, i);
					iret = GetSettingValue(tempStr, str_Setting);
					//Socket断开，网络通讯出现问题
					if (iret == -1)
					{
						delete startIndex;
						delete indexChannel;
						startIndex = NULL;
						indexChannel = NULL;
						return -1;
					}
					//获取正常，写入
					keyName.Format(_T("FFT%d.Weighting"), x);
					::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

					//获取添加的阶次信息，总共8个阶次
					CString CBTParaName[3] = { _T("Active"), _T("Order"), _T("Band") };

					//获取第一行的信息，如果第一行为ON，后面的每行都能得到信息，不管是否激活
					tempStr.Format("fft%d channel%d 215 0 0", x, i);
					iret = GetSettingValue(tempStr, str_Setting);
					//Socket断开，网络通讯出现问题
					if (iret == -1)
					{
						delete startIndex;
						delete indexChannel;
						startIndex = NULL;
						indexChannel = NULL;
						return -1;
					}
					//获取出错，证明没有添加阶次，可以退出本次循环，进入到下一个通道了
					if (iret == 1)
					{
						keyName.Format(_T("FFT%d.TO.States"), x);
						::WritePrivateProfileStringA(sectionName, keyName, _T("OFF"), iniFileName);
						keyName.Format(_T("FFT%d.TO.Number"), x);
						::WritePrivateProfileStringA(sectionName, keyName, _T("0"), iniFileName);
						continue;  //进入下一次循环了
					}

					//获取正常，激活，所有后续的数据都可以读取到
					if (iret == 0)
					{
						//针对于第一行的所有数据写入
						{
							keyName.Format(_T("FFT%d.TO.States"), x);
							::WritePrivateProfileStringA(sectionName, keyName, _T("ON"), iniFileName);

							keyName.Format(_T("FFT%d.TO.Number"), x);
							::WritePrivateProfileStringA(sectionName, keyName, _T("0"), iniFileName);

							//把第一行的数据写入
							keyName.Format(_T("FFT%d.TO%d.%s"), x, 0, CBTParaName[0]);
							::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

							for (int k = 1; k < 3; k++)
							{
								tempStr.Format("fft%d channel%d 215 %d %d", x, i, 0, k);
								iret = GetSettingValue(tempStr, str_Setting);
								if (iret == -1)  //SOCKET断开
								{
									delete startIndex;
									delete indexChannel;
									startIndex = NULL;
									indexChannel = NULL;
									return -1;
								}
								keyName.Format(_T("FFT%d.TO%d.%s"), x, 0, CBTParaName[k]);
								::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);
							}
						}

						//倒序循环，获取数目
						bFirstFlag = TRUE;
						for (int j = 7; j > 0; j--)
						{
							tempStr.Format("fft%d channel%d 215 %d 0", x, i, j);
							iret = GetSettingValue(tempStr, str_Setting);

							//Socket断开
							if (iret == -1)
							{
								delete startIndex;
								delete indexChannel;
								startIndex = NULL;
								indexChannel = NULL;
								return -1;
							}

							if (iret == 0)
							{
								if (bFirstFlag)
								{
									if (str_Setting == _T("1"))
									{
										CString numToStr = _T("");
										numToStr.Format("%d", j);
										keyName.Format(_T("FFT%d.TO.Number"), x);
										::WritePrivateProfileStringA(sectionName, keyName, numToStr, iniFileName);
										bFirstFlag = FALSE;

										keyName.Format(_T("FFT%d.TO%d.%s"), x, j, CBTParaName[0]);
										::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

										//写入另外的参数
										for (int k = 1; k < 3; k++)
										{
											tempStr.Format("fft%d channel%d 215 %d %d", x, i, j, k);
											iret = GetSettingValue(tempStr, str_Setting);
											if (iret == -1)
											{
												delete startIndex;
												delete indexChannel;
												startIndex = NULL;
												indexChannel = NULL;
												return -1;
											}
											keyName.Format(_T("FFT%d.TO%d.%s"), x, j, CBTParaName[k]);
											::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);
										}
									}
								}
								else
								{
									//写入三个参数
									keyName.Format(_T("FFT%d.TO%d.%s"), x, j, CBTParaName[0]);
									::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

									//写入另外的参数
									for (int k = 1; k < 3; k++)
									{
										tempStr.Format("fft%d channel%d 215 %d %d", x, i, j, k);
										iret = GetSettingValue(tempStr, str_Setting);
										if (iret == -1)
										{
											delete startIndex;
											delete indexChannel;
											startIndex = NULL;
											indexChannel = NULL;
											return -1;
										}
										keyName.Format(_T("FFT%d.TO%d.%s"), x, j, CBTParaName[k]);
										::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);
									}
								}
							}
						}

					}
				}
				else
				{
					FFTChIndex[x][i] = _T("none");
				}

			}//当前FFT的Channel已经激活

		}
	}

	//读取TDA的参数
	// TDA有32个通道

	for (int i = 1; i <= DFT_MAXINPUTCHANNELCOUNT; i++)
	{

		ForcePostMessage(WM_STARTGETTINGTDA, i, (LPARAM)m_pTcpClient);

		//获取FFT Channel是否激活，如果没有激活就不用去获取了
		tempStr.Format(_T("tda channel%d active"), i);
		iret = GetSettingValue(tempStr, str_Setting);

		//Socket断开，网络通讯出现问题
		if (iret == -1)
		{
			delete startIndex;
			delete indexChannel;
			startIndex = NULL;
			indexChannel = NULL;
			return -1;
		}

		//返回错误代码，应该是通过授权超出，退出本次循环
		if (iret == 1)
		{
			break; //退出当前FFT的通道循环
		}

		if (str_Setting == _T("1"))
		{
			tempStr.Format("tda channel%d 100", i);
			iret = GetSettingValue(tempStr, str_Setting);

			if (iret == -1)    //Socket断开
			{
				delete startIndex;
				delete indexChannel;
				startIndex = NULL;
				indexChannel = NULL;
				return -1;
			}

			if (str_Setting != _T("none") && str_Setting != _T(""))
			{
				*startIndex = 0;
				*indexChannel = 0;

				//解析出通道号，并得到子系统和子系统中的通道号
				GetIndex(GetCHIndexFromString(str_Setting), *startIndex, *indexChannel, 0, m_numofSystem, m_pNumofChannel);
				sectionName.Format(_T("SYS%d_Input%d"), *startIndex, *indexChannel);
				TDAChIndex[i] = sectionName;

				::WritePrivateProfileStringA(sectionName, _T("TDA"), "ON", iniFileName);

				tempStr.Format("tda channel%d inputFilter", i);
				iret = GetSettingValue(tempStr, str_Setting);
				if (iret == -1)
				{
					delete startIndex;
					delete indexChannel;
					startIndex = NULL;
					indexChannel = NULL;
					return -1;
				}
				::WritePrivateProfileStringA(sectionName, _T("TDA.InputFilter"), str_Setting, iniFileName);


				tempStr.Format("tda channel%d 23", i);
				iret = GetSettingValue(tempStr, str_Setting);
				if (iret == -1)
				{
					delete startIndex;
					delete indexChannel;
					startIndex = NULL;
					indexChannel = NULL;
					return -1;
				}
				::WritePrivateProfileStringA(sectionName, _T("TDA.Duration"), str_Setting, iniFileName);
			}
			else
			{
				TDAChIndex[i] = "none";
			}
		}

	}

	//读取ORD的参数，並把相关的信息写入到对应的INPUT中
	//ORD的通道数为32通道

	//int moduleIDORD[3] = { 0, 14, 17 };
	for (int x = 1; x <= 2; x++)   //2个ORD模块
	{
		for (int i = 1; i <= DFT_MAXINPUTCHANNELCOUNT; i++)
		{
			ForcePostMessage(WM_STARTGETTINGORD, i, (LPARAM)m_pTcpClient);
			//获取ORD Channel是否激活，如果没有激活就不用去获取了
			tempStr.Format(_T("soa%d channel%d active"), x, i);
			iret = GetSettingValue(tempStr, str_Setting);

			//Socket断开，网络通讯出现问题
			if (iret == -1)
			{
				delete startIndex;
				delete indexChannel;
				startIndex = NULL;
				indexChannel = NULL;
				return -1;
			}

			//返回错误代码，应该是通过授权超出，退出本次循环
			if (iret == 1)
			{
				break; //退出当前FFT的通道循环
			}

			if (str_Setting == _T("1"))
			{
				//获取ORDER的Channel的数据源
				tempStr.Format("soa%d channel%d 100", x, i);
				iret = GetSettingValue(tempStr, str_Setting);

				//SOCKET出错，退出
				if (iret == -1)
				{
					delete startIndex;
					delete indexChannel;
					startIndex = NULL;
					indexChannel = NULL;
					return -1;
				}

				if (str_Setting != _T("none") && str_Setting != _T(""))
				{
					*startIndex = 0;
					*indexChannel = 0;

					//解析出通道号，并得到子系统和子系统中的通道号
					GetIndex(GetCHIndexFromString(str_Setting), *startIndex, *indexChannel, 0, m_numofSystem, m_pNumofChannel);
					sectionName.Format(_T("SYS%d_Input%d"), *startIndex, *indexChannel);

					ORDChIndex[x][i] = sectionName;

					keyName.Format(_T("ORD%d"), x);
					::WritePrivateProfileStringA(sectionName, keyName, "ON", iniFileName);

					tempStr.Format("soa%d channel%d inputFilter", x, i);
					iret = GetSettingValue(tempStr, str_Setting);
					if (iret == -1)
					{
						delete startIndex;
						delete indexChannel;
						startIndex = NULL;
						indexChannel = NULL;
						return -1;
					}
					keyName.Format(_T("ORD%d.InputFilter"), x);
					::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

					tempStr.Format("soa%d channel%d 082", x, i);
					iret = GetSettingValue(tempStr, str_Setting);
					if (iret == -1)
					{
						delete startIndex;
						delete indexChannel;
						startIndex = NULL;
						indexChannel = NULL;
						return -1;
					}
					keyName.Format(_T("ORD%d.Weighting"), x);
					::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);


					//获取添加的阶次信息，总共8个阶次
					CString CBTParaName[3] = { _T("Active"), _T("Order") };
					//获取第一行的信息，如果第一行为ON，后面的每行都能得到信息，不管是否激活
					tempStr.Format("soa%d channel%d 215 0 0", x, i);
					iret = GetSettingValue(tempStr, str_Setting);
					//Socket断开，网络通讯出现问题
					if (iret == -1)
					{
						delete startIndex;
						delete indexChannel;
						startIndex = NULL;
						indexChannel = NULL;
						return -1;
					}
					//获取出错，证明没有添加阶次，可以退出本次循环，进入到下一个通道了
					if (iret == 1)
					{
						keyName.Format(_T("ORD%d.TO.States"), x);
						::WritePrivateProfileStringA(sectionName, keyName, _T("OFF"), iniFileName);
						keyName.Format(_T("ORD%d.TO.Number"), x);
						::WritePrivateProfileStringA(sectionName, keyName, _T("0"), iniFileName);
						continue;  //进入下一次循环了
					}

					//获取正常，激活，所有后续的数据都可以读取到
					if (iret == 0)
					{
						//针对于第一行的所有数据写入
						{
							keyName.Format(_T("ORD%d.TO.States"), x);
							::WritePrivateProfileStringA(sectionName, keyName, _T("ON"), iniFileName);

							keyName.Format(_T("ORD%d.TO.Number"), x);
							::WritePrivateProfileStringA(sectionName, keyName, _T("0"), iniFileName);

							//把第一行的数据写入
							keyName.Format(_T("ORD%d.TO%d.%s"), x, 0, CBTParaName[0]);
							::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

							for (int k = 1; k < 2; k++)
							{
								tempStr.Format("soa%d channel%d 215 %d %d", x, i, 0, k);
								iret = GetSettingValue(tempStr, str_Setting);
								if (iret == -1)  //SOCKET断开
								{
									delete startIndex;
									delete indexChannel;
									startIndex = NULL;
									indexChannel = NULL;
									return -1;
								}
								keyName.Format(_T("SOA%d.TO%d.%s"), x, 0, CBTParaName[k]);
								::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);
							}
						}

						//倒序循环，获取数目
						bFirstFlag = TRUE;
						for (int j = 7; j > 0; j--)
						{
							tempStr.Format("soa%d channel%d 215 %d 0", x, i, j);
							iret = GetSettingValue(tempStr, str_Setting);

							//Socket断开
							if (iret == -1)
							{
								delete startIndex;
								delete indexChannel;
								startIndex = NULL;
								indexChannel = NULL;
								return -1;
							}

							if (iret == 0)
							{
								if (bFirstFlag)
								{
									if (str_Setting == _T("1"))
									{
										CString numToStr = _T("");
										numToStr.Format("%d", j);
										keyName.Format(_T("ORD%d.TO.Number"), x);
										::WritePrivateProfileStringA(sectionName, keyName, numToStr, iniFileName);
										bFirstFlag = FALSE;

										keyName.Format(_T("ORD%d.TO%d.%s"), x, j, CBTParaName[0]);
										::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

										//写入另外的参数
										for (int k = 1; k < 2; k++)
										{
											tempStr.Format("soa%d channel%d 215 %d %d", x, i, j, k);
											iret = GetSettingValue(tempStr, str_Setting);
											if (iret == -1)
											{
												delete startIndex;
												delete indexChannel;
												startIndex = NULL;
												indexChannel = NULL;
												return -1;
											}
											keyName.Format(_T("ORD%d.TO%d.%s"), x, j, CBTParaName[k]);
											::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);
										}
									}
								}
								else
								{
									//写入三个参数
									keyName.Format(_T("ORD%d.TO%d.%s"), x, j, CBTParaName[0]);
									::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

									//写入另外的参数
									for (int k = 1; k < 2; k++)
									{
										tempStr.Format("soa%d channel%d 215 %d %d", x, i, j, k);
										iret = GetSettingValue(tempStr, str_Setting);
										if (iret == -1)
										{
											delete startIndex;
											delete indexChannel;
											startIndex = NULL;
											indexChannel = NULL;
											return -1;
										}
										keyName.Format(_T("ORD%d.TO%d.%s"), x, j, CBTParaName[k]);
										::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);
									}
								}
							}
						}

					}

				}
				else
				{
					ORDChIndex[x][i] = _T("none");
				}
			}

		}
	}

	//读取waterfall的参数
	//Waterfall有96个通道
	for (int i = 1; i <= 96; i++)
	{
		ForcePostMessage(WM_STARTGETTINGWTF, i, (LPARAM)m_pTcpClient);

		int intTemp1 = 0;
		int intTemp2 = 0;
		int intTemp3 = 0;

		tempStr.Format("16 %d 294", 775 + i);
		iret = GetSettingValue(tempStr, str_Setting);
		if (iret == -1)
		{
			delete startIndex;
			delete indexChannel;
			startIndex = NULL;
			indexChannel = NULL;
			return -1;
		}

		//超出授权
		if (iret == 1)
		{
			break;
		}

		if (str_Setting != _T("None"))
		{
			//找出：的位置
			intTemp1 = str_Setting.Find(":");
			if (intTemp1 != -1)
			{
				CString moduleName = str_Setting.Mid(0, intTemp1);
				intTemp2 = str_Setting.Find("[");
				intTemp3 = str_Setting.Find("]");
				CString moduleCh = str_Setting.Mid(intTemp2 + 1, intTemp3 - intTemp2 - 1);
				CString processName = str_Setting.Mid(intTemp1 + 2, intTemp2 - intTemp1 - 3);

				if (moduleName == _T("Tach"))
				{
					//目前考虑两种情况 Ext和Frac
					if (processName == _T("Ext Ang speed"))
					{
						sectionName.Format(_T("Ext. Sync. %s"), moduleCh);
						::WritePrivateProfileStringA(sectionName, _T("WTF"), _T("ON"), iniFileName);
					}
					else if (processName == _T("Frac Ang speed"))
					{
						sectionName.Format(_T("Fraction Tach. %s"), moduleCh);
						::WritePrivateProfileStringA(sectionName, _T("WTF"), _T("ON"), iniFileName);
					}
				}
				else
				{
					int tempContent = moduleName == _T("FFT1") ? 1 : moduleName == _T("FFT2") ? 2 : moduleName == _T("FFT3") ? 3 :
						moduleName == _T("FFT4") ? 4 : moduleName == _T("TDA") ? 5 : moduleName == _T("Ord1") ? 6 :
						(moduleName == _T("Ord2") ? 7 : 0);
					switch (tempContent)
					{
					case 1: case 2: case 3: case 4:
						if (processName == _T("Orders"))
						{
							sectionName = FFTChIndex[tempContent][atoi(moduleCh)];
							keyName.Format(_T("%s.WTF"), moduleName);
							::WritePrivateProfileStringA(sectionName, keyName, "ON", iniFileName);
						}
						break;
					case 5:
						if (processName == _T("DC") || processName == _T("RMS") || processName == _T("Pk") ||
							processName == _T("Pk-Pk") || processName == _T("Max") || processName == _T("Min"))
						{
							sectionName = TDAChIndex[atoi(moduleCh)];
							keyName.Format(_T("%s.WTF.%s"), moduleName, processName);
							::WritePrivateProfileStringA(sectionName, keyName, "ON", iniFileName);
						}
						break;
					case 6: case 7:
						if (processName == _T("Orders"))
						{
							sectionName = ORDChIndex[tempContent - 5][atoi(moduleCh)];
							keyName.Format(_T("%s.WTF"), moduleName);
							::WritePrivateProfileStringA(sectionName, keyName, "ON", iniFileName);
						}
						break;
					default:
						break;

					}
				}
			}

		}

	}

	delete startIndex;
	delete indexChannel;
	startIndex = NULL;
	indexChannel = NULL;

	return 0;
}

//生成子系统的Workbook
int CGenerateModel::GenerateSubModel(void)
{
	CString tempStr = _T("");
	CString sectionName = _T("");
	CString keyName = _T("");
	char strINIInfo[64] = { "" };
	CString iniKeyValue = _T("");
	int iret = 0;
	int indexXpod = 0;

	CString inputParaName[17] = { _T("Label"), _T("Input Type"), _T("Transducer"), _T("Physical Qty"),
		_T("Sensitivity"), _T("Range Peak"), _T("Polarity"), _T("Offset Comp"),
		_T("Coupling"), _T("Sampling Rate"), _T("Gauge Type"), _T("Bridge Resistance"),
		_T("Bridge Gain"), _T("Bridge Offset"), _T("Input Filter"), _T("Auto Range"),
		_T("Auto Zero") };
	CString strParameterArray[17] = { _T("label"), _T("inputType"), _T("transducer"), _T("physicalQty"),
		_T("sensitivity"), _T("rangePk"), _T("polarity"), _T("offset"),
		_T("coupling"), _T("869"), _T("gaugeType"), _T("bridgeResistance"),
		_T("bridgeGain"), _T("bridgeOffset"), _T("inputFilter"),
		_T("enableAutorange"), _T("enableBridgeAutoZero") };

	for (int nsys = 1; nsys <= m_numofSystem; nsys++)
	{
		tempStr.Format(_T("Generating workbook model for System #%d..."), nsys);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);
		Sleep(1000);

		//Load Default Workbook SuperModel
		//ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Loading original workbbok model..."), (LPARAM)m_pTcpClient);

		iret = m_pTcpClient->SendDataToNVGate(_T("SetSilent 1"));
		if (iret == -1) return -1;

		//tempStr.Format(_T("LoadWorkbookModel %s User"), m_strConfigName);
		////iret = m_pTcpClient->SendDataToNVGate(_T("LoadWorkbookModel SuperModel User"));
		//iret = m_pTcpClient->SendDataToNVGate(tempStr);
		//if (iret == -1) return -1;

		//设置延迟写入，等所有操作结束后再写入
		iret = m_pTcpClient->SendDataToNVGate("TransactionBegin");
		if (iret == -1) return -1;

		//重新断开连接输入通道
		tempStr.Format(_T("Re-connecting inputs for System #%d..."), nsys);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);

		if (nsys > 1)  //不是第一台子系统
		{
			for (int i = 1; i <= m_pNumofChannel[nsys - 1]; i++)
			{
				sectionName.Format(_T("SYS%d_Input%d"), nsys, i);
				//获取通道的Active的信息
				GetPrivateProfileString(sectionName, _T("Active"), _T(""), strINIInfo, 64, iniFileName);
				iniKeyValue = _T(strINIInfo);

				if (iniKeyValue == _T("ON"))
				{
					tempStr.Format(_T("SetSettingValue frontEnd input%d active 1"), i);
					iret = m_pTcpClient->SendDataToNVGate(tempStr);
					if (iret == -1) return -1;

					//XPOD
					if ((int)i % 8 == 0)
					{
						indexXpod = i / 8;
					}
					else
					{
						indexXpod = i / 8 + 1;
					}

					//写入XPOD的信息
					GetPrivateProfileString(sectionName, _T("XPType"), _T(""), strINIInfo, 64, iniFileName);
					iniKeyValue = _T(strINIInfo);
					tempStr.Format(_T("SetSettingValue frontEnd expander%d xpType %s"), indexXpod, iniKeyValue);
					iret = m_pTcpClient->SendDataToNVGate(tempStr);
					if (iret == -1) return -1;

					GetPrivateProfileString(sectionName, _T("BridgeMaxCurrent"), _T(""), strINIInfo, 64, iniFileName);
					iniKeyValue = _T(strINIInfo);
					tempStr.Format(_T("SetSettingValue frontEnd expander%d bridgeMaxCurrent %s"), indexXpod, iniKeyValue);
					iret = m_pTcpClient->SendDataToNVGate(tempStr);
					if (iret == -1) return -1;

					GetPrivateProfileString(sectionName, _T("BridgeVoltage"), _T(""), strINIInfo, 64, iniFileName);
					iniKeyValue = _T(strINIInfo);
					tempStr.Format(_T("SetSettingValue frontEnd expander%d bridgeVoltage %s"), indexXpod, iniKeyValue);
					iret = m_pTcpClient->SendDataToNVGate(tempStr);
					if (iret == -1) return -1;

					//设置其他的参数
					for (int j = 0; j < 17; j++)
					{
						GetPrivateProfileString(sectionName, inputParaName[j], _T(""), strINIInfo, 64, iniFileName);
						iniKeyValue = _T(strINIInfo);
						iniKeyValue.Replace(_T(" "), _T("\\ "));
						tempStr.Format(_T("SetSettingValue frontEnd input%d %s %s"), i, strParameterArray[j], iniKeyValue);
						iret = m_pTcpClient->SendDataToNVGate(tempStr);
						if (iret == -1) return -1;
					}
				}
				else  //断开这个通道
				{
					tempStr.Format(_T("SetSettingValue frontEnd input%d active 0"), i);
					iret = m_pTcpClient->SendDataToNVGate(tempStr);

					tempStr.Format(_T("SetToDefault frontEnd input%d 0"), i);
					iret = m_pTcpClient->SendDataToNVGate(tempStr);
					if (iret == -1) return -1;
				}
			}
		}

		if (nsys == 1)
		{
			//断开多余的通道
			for (int i = m_pNumofChannel[nsys - 1] + 1; i <= m_totalChannel; i++)
			{
				tempStr.Format(_T("SetSettingValue frontEnd input%d active 0"), i);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				tempStr.Format(_T("SetToDefault frontEnd input%d 0"), i);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				if (iret == -1) return -1;
			}
		}

		//记录仪设置
		//记录仪设为缺省
		tempStr.Format(_T("Re-setting recorder tracks for System #%d..."), nsys);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);

		iret = m_pTcpClient->SendDataToNVGate(_T("SetToDefaultCollection recorder track"));
		GetPrivateProfileString(sectionName, _T("REC"), _T(""), strINIInfo, 64, iniFileName);
		iniKeyValue = _T(strINIInfo);

		iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue recorder bandwiths 871 0"));
		GetPrivateProfileString(sectionName, _T("REC"), _T(""), strINIInfo, 64, iniFileName);
		iniKeyValue = _T(strINIInfo);

		//添加激活的输入通道到RECORDER
		for (int i = 1; i <= m_pNumofChannel[nsys - 1]; i++)
		{
			sectionName.Format(_T("SYS%d_Input%d"), nsys, i);
			//获取通道的Active的信息
			GetPrivateProfileString(sectionName, _T("REC"), _T(""), strINIInfo, 64, iniFileName);
			iniKeyValue = _T(strINIInfo);

			if (iniKeyValue == _T("ON"))
			{
				//3代表recorder
				tempStr.Format(_T("ConnectInput frontEnd.input%d recorder track%d"), i, i);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				if (iret == -1) return -1;

				//输入采样率
				GetPrivateProfileString(sectionName, _T("REC.Sampling"), _T(""), strINIInfo, 64, iniFileName);
				iniKeyValue = _T(strINIInfo);
				tempStr.Format(_T("SetSettingValue recorder track%d 14 %s"), i, iniKeyValue);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				if (iret == -1) return -1;
			}
		}

		//设置通道中Automatic和记录仪位置的值
		iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue recorder recMode 873 0"));  //Automatic = Disable
		iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue recorder recMode 200 1"));  //Record = On PC

		//添加转速通道
		GetPrivateProfileString(_T("Ext. Sync. 1"), _T("REC"), _T(""), strINIInfo, 64, iniFileName);
		iniKeyValue = _T(strINIInfo);
		if (iniKeyValue == _T("ON"))
		{
			//3代表recorder
			tempStr.Format(_T("ConnectInput frontEnd.extSync1 recorder track%d"), m_pNumofChannel[nsys - 1] + 1);
			iret = m_pTcpClient->SendDataToNVGate(tempStr);
			if (iret == -1) return -1;
		}

		GetPrivateProfileString(_T("Ext. Sync. 2"), _T("REC"), _T(""), strINIInfo, 64, iniFileName);
		iniKeyValue = _T(strINIInfo);
		if (iniKeyValue == _T("ON"))
		{
			//3代表recorder
			tempStr.Format(_T("ConnectInput frontEnd.extSync2 recorder track%d"), m_pNumofChannel[nsys - 1] + 2);
			iret = m_pTcpClient->SendDataToNVGate(tempStr);
			if (iret == -1) return -1;
		}

		GetPrivateProfileString(_T("Ext. Sync. 3"), _T("REC"), _T(""), strINIInfo, 64, iniFileName);
		iniKeyValue = _T(strINIInfo);
		if (iniKeyValue == _T("ON"))
		{
			//3代表recorder
			tempStr.Format(_T("ConnectInput frontEnd.extSync3 recorder track%d"), m_pNumofChannel[nsys - 1] + 3);
			iret = m_pTcpClient->SendDataToNVGate(tempStr);
			if (iret == -1) return -1;
		}

		GetPrivateProfileString(_T("Ext. Sync. 4"), _T("REC"), _T(""), strINIInfo, 64, iniFileName);
		iniKeyValue = _T(strINIInfo);
		if (iniKeyValue == _T("ON"))
		{
			//3代表recorder
			tempStr.Format(_T("ConnectInput frontEnd.extSync4 recorder track%d"), m_pNumofChannel[nsys - 1] + 4);
			iret = m_pTcpClient->SendDataToNVGate(tempStr);
			if (iret == -1) return -1;
		}

		//iret = SendDataToNVGate("TransactionEnd");
		//if (iret == -1) return -1;

		//iret = SendDataToNVGate("TransactionBegin");
		//if (iret == -1) return -1;

		//读取FFT1的信息，并写入到NVGATE
		for (int x = 1; x <= 4; x++)   //4个FFT
		{
			tempStr.Format(_T("Re-setting FFT%d channels for System #%d..."), x, nsys);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);
			//FFT的Channel变成默认的
			tempStr.Format(_T("SetToDefaultCollection fft%d channel"), x);
			iret = m_pTcpClient->SendDataToNVGate(tempStr);
			//SOCKET退出
			if (iret == -1) return -1;

			for (int i = 1; i <= m_pNumofChannel[nsys - 1]; i++)
			{
				//读取INI文件
				sectionName.Format(_T("SYS%d_Input%d"), nsys, i);
				keyName.Format(_T("FFT%d"), x);
				GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
				iniKeyValue = _T(strINIInfo);

				//此通道的FFT已经激活
				if (iniKeyValue == ("ON"))
				{
					//连接输入通道到FFT
					tempStr.Format(_T("ConnectInput frontEnd.input%d fft%d channel%d"), i, x, i);
					iret = m_pTcpClient->SendDataToNVGate(tempStr);
					if (iret == -1) return -1;

					if (iret == 0)
					{
						//写入参数

						//Input Filter
						keyName.Format(_T("FFT%d.InputFilter"), x);
						GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
						iniKeyValue = _T(strINIInfo);

						tempStr.Format(_T("SetSettingValue fft%d channel%d inputFilter %s"), x, i, iniKeyValue);
						iret = m_pTcpClient->SendDataToNVGate(tempStr);
						if (iret == -1) return -1;

						//Weighting Window
						keyName.Format(_T("FFT%d.Weighting"), x);
						GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
						iniKeyValue = _T(strINIInfo);

						tempStr.Format(_T("SetSettingValue fft%d channel%d 82 %s"), x, i, iniKeyValue);
						iret = m_pTcpClient->SendDataToNVGate(tempStr);
						if (iret == -1) return -1;

						//Tracked Order
						keyName.Format(_T("FFT%d.TO.States"), x);
						GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
						iniKeyValue = _T(strINIInfo);

						if (iniKeyValue == _T("ON"))
						{
							//跟踪阶次开关，1为打开，0个关闭
							CString str_State = _T("");
							CString str_Order = _T("");
							CString str_BandWidth = _T("");

							//获取TO的数量
							keyName.Format(_T("FFT%d.TO.Number"), x);
							GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
							str_State = _T(strINIInfo);
							int nToNumber = atoi(str_State);

							for (int j = 0; j <= nToNumber; j++)
							{
								keyName.Format(_T("FFT%d.TO%d.Active"), x, j);
								GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
								str_State = _T(strINIInfo);

								keyName.Format(_T("FFT%d.TO%d.Order"), x, j);
								GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
								str_Order = _T(strINIInfo);

								keyName.Format(_T("FFT%d.TO%d.Band"), x, j);
								GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
								str_BandWidth = _T(strINIInfo);

								tempStr.Format(_T("AddArrayLine fft%d channel%d 215 1 %s %s %s"), x, i, str_State, str_Order, str_BandWidth);
								iret = m_pTcpClient->SendDataToNVGate(tempStr);
								if (iret == -1) return -1;

							}

						}

					}
				}
			}
		}

		//读取TDA的信息，并写入到NVGate中
		tempStr.Format(_T("Re-setting TDA channels for System #%d..."), nsys);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);
		{
			iret = m_pTcpClient->SendDataToNVGate(_T("SetToDefaultCollection tda channel"));
			if (iret == -1) return -1;

			for (int i = 1; i <= m_pNumofChannel[nsys - 1]; i++)
			{
				//读取INI文件
				sectionName.Format(_T("SYS%d_Input%d"), nsys, i);
				GetPrivateProfileString(sectionName, _T("TDA"), _T(""), strINIInfo, 64, iniFileName);
				iniKeyValue = _T(strINIInfo);
				if (iniKeyValue == ("ON"))
				{
					//连接输入通道到FFT
					tempStr.Format(_T("ConnectInput frontEnd.input%d tda channel%d"), i, i);
					iret = m_pTcpClient->SendDataToNVGate(tempStr);
					if (iret == -1) return -1;

					if (iret == 0)
					{
						//Input Filter
						GetPrivateProfileString(sectionName, _T("TDA.InputFilter"), _T(""), strINIInfo, 64, iniFileName);
						iniKeyValue = _T(strINIInfo);

						tempStr.Format(_T("SetSettingValue tda channel%d inputFilter %s"), i, iniKeyValue);
						iret = m_pTcpClient->SendDataToNVGate(tempStr);
						if (iret == -1) return -1;

						//Duration
						GetPrivateProfileString(sectionName, _T("TDA.Duration"), _T(""), strINIInfo, 64, iniFileName);
						iniKeyValue = _T(strINIInfo);

						tempStr.Format(_T("SetSettingValue tda channel%d 23 %s"), i, iniKeyValue);
						iret = m_pTcpClient->SendDataToNVGate(tempStr);
						if (iret == -1) return -1;


					}

				}
			}
		}

		//读取ORD的信息，并写入到NVGATE
		for (int x = 1; x <= 2; x++)   //2个soa
		{
			tempStr.Format(_T("Re-setting ORD%d channels for System #%d..."), x, nsys);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);

			tempStr.Format(_T("SetToDefaultCollection soa%d channel"), x);
			iret = m_pTcpClient->SendDataToNVGate(tempStr);
			if (iret == -1) return -1;

			for (int i = 1; i <= m_pNumofChannel[nsys - 1]; i++)
			{
				//读取INI文件
				sectionName.Format(_T("SYS%d_Input%d"), nsys, i);
				keyName.Format(_T("ORD%d"), x);
				GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
				iniKeyValue = _T(strINIInfo);

				//此通道的FFT已经激活
				if (iniKeyValue == ("ON"))
				{
					//连接输入通道到FFT
					tempStr.Format(_T("ConnectInput frontEnd.input%d soa%d channel%d"), i, x, i);
					iret = m_pTcpClient->SendDataToNVGate(tempStr);
					if (iret == -1) return -1;

					if (iret == 0)
					{
						//Input Filter
						keyName.Format(_T("ORD%d.InputFilter"), x);
						GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
						iniKeyValue = _T(strINIInfo);

						tempStr.Format(_T("SetSettingValue soa%d channel%d inputFilter %s"), x, i, iniKeyValue);
						iret = m_pTcpClient->SendDataToNVGate(tempStr);
						if (iret == -1) return -1;

						//Weighting Window
						keyName.Format(_T("ORD%d.Weighting"), x);
						GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
						iniKeyValue = _T(strINIInfo);

						tempStr.Format(_T("SetSettingValue soa%d channel%d 82 %s"), x, i, iniKeyValue);
						iret = m_pTcpClient->SendDataToNVGate(tempStr);
						if (iret == -1) return -1;

						//Tracked Order
						keyName.Format(_T("ORD%d.TO.States"), x);
						GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
						iniKeyValue = _T(strINIInfo);

						if (iniKeyValue == _T("ON"))
						{
							//跟踪阶次开关，1为打开，0个关闭
							CString str_State = _T("");
							CString str_Order = _T("");
							//CString str_BandWidth = _T("");

							//获取TO的数量
							keyName.Format(_T("ORD%d.TO.Number"), x);
							GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
							str_State = _T(strINIInfo);
							int nToNumber = atoi(str_State);

							for (int j = 0; j <= nToNumber; j++)
							{
								keyName.Format(_T("ORD%d.TO%d.Active"), x, j);
								GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
								str_State = _T(strINIInfo);

								keyName.Format(_T("ORD%d.TO%d.Order"), x, j);
								GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
								str_Order = _T(strINIInfo);

								//keyName.Format(_T("FFT%d.TO%d.Band"), x, j);
								//GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
								//str_BandWidth = _T(strINIInfo);

								tempStr.Format(_T("AddArrayLine soa%d channel%d 215 1 %s %s"), x, i, str_State, str_Order);
								iret = m_pTcpClient->SendDataToNVGate(tempStr);
								if (iret == -1) return -1;

							}

						}

					}
				}
			}
		}

		tempStr.Format(_T("Re-setting waterfall channels for System #%d..."), nsys);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);
		//获取WTF的信息，并写入！
		for (int i = 1; i <= m_pNumofChannel[nsys - 1]; i++)
		{
			sectionName.Format(_T("SYS%d_Input%d"), nsys, i);
			for (int x = 1; x <= 4; x++)  //4个FFT
			{
				keyName.Format(_T("FFT%d.WTF"), x);
				GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
				iniKeyValue = _T(strINIInfo);

				if (iniKeyValue == ("ON"))
				{
					tempStr.Format(_T("SelectWtfChannels 44 fft%d %d"), x, i);
					iret = m_pTcpClient->SendDataToNVGate(tempStr);
					if (iret == -1) return -1;
				}
			}

			for (int x = 1; x <= 2; x++)  //2个ORD
			{
				keyName.Format(_T("ORD%d.WTF"), x);
				GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
				iniKeyValue = _T(strINIInfo);

				if (iniKeyValue == ("ON"))
				{
					tempStr.Format(_T("SelectWtfChannels 44 soa%d %d"), x, i);
					iret = m_pTcpClient->SendDataToNVGate(tempStr);
					if (iret == -1) return -1;
				}
			}

			GetPrivateProfileString(sectionName, _T("TDA.WTF.DC"), _T(""), strINIInfo, 64, iniFileName);
			iniKeyValue = _T(strINIInfo);

			if (iniKeyValue == ("ON"))
			{
				tempStr.Format(_T("SelectWtfChannels 9 tda %d"), i);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				if (iret == -1) return -1;
			}

			GetPrivateProfileString(sectionName, _T("TDA.WTF.RMS"), _T(""), strINIInfo, 64, iniFileName);
			iniKeyValue = _T(strINIInfo);

			if (iniKeyValue == ("ON"))
			{
				tempStr.Format(_T("SelectWtfChannels 10 tda %d"), i);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				if (iret == -1) return -1;
			}

			GetPrivateProfileString(sectionName, _T("TDA.WTF.Min"), _T(""), strINIInfo, 64, iniFileName);
			iniKeyValue = _T(strINIInfo);

			if (iniKeyValue == ("ON"))
			{
				tempStr.Format(_T("SelectWtfChannels 15 tda %d"), i);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				if (iret == -1) return -1;
			}

			GetPrivateProfileString(sectionName, _T("TDA.WTF.Max"), _T(""), strINIInfo, 64, iniFileName);
			iniKeyValue = _T(strINIInfo);

			if (iniKeyValue == ("ON"))
			{
				tempStr.Format(_T("SelectWtfChannels 14 tda %d"), i);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				if (iret == -1) return -1;
			}

			GetPrivateProfileString(sectionName, _T("TDA.WTF.Pk"), _T(""), strINIInfo, 64, iniFileName);
			iniKeyValue = _T(strINIInfo);

			if (iniKeyValue == ("ON"))
			{
				tempStr.Format(_T("SelectWtfChannels 271 tda %d"), i);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				if (iret == -1) return -1;
			}

			GetPrivateProfileString(sectionName, _T("TDA.WTF.Pk-Pk"), _T(""), strINIInfo, 64, iniFileName);
			iniKeyValue = _T(strINIInfo);

			if (iniKeyValue == ("ON"))
			{
				tempStr.Format(_T("SelectWtfChannels 272 tda %d"), i);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				if (iret == -1) return -1;
			}
		}

		iret = m_pTcpClient->SendDataToNVGate("TransactionEnd");
		if (iret == -1) return -1;

		//保存为一个新的模板

		tempStr.Format(_T("Saving workbook model for System #%d..."), nsys);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);

		iret = m_pTcpClient->SendDataToNVGate(_T("SetSilent 1"));
		if (iret == -1) return -1;

		tempStr.Format(_T("SaveWorkbookModel #%s_%d"), m_strConfigName, nsys);
		iret = m_pTcpClient->SendDataToNVGate(tempStr);
		if (iret == -1) return -1;
	}

	////读取Configuration Number
	//GetPrivateProfileString(_T("Workbook"), _T("Number"), _T("0"), strINIInfo, 64, tempIniFileName);
	//int m_nWorkbookNumber = atoi(strINIInfo);

	//for (int i = 0; i < m_nWorkbookNumber; i++)
	//{

	//}
	
	////+1 写入
	//CString tempIniValue = _T("");
	//tempIniValue.Format(_T("%d"), m_nWorkbookNumber + 1);
	//::WritePrivateProfileStringA(_T("Workbook"), _T("Number"), tempIniValue, tempIniFileName);

	////写入Configuration Name
	//sectionName.Format(_T("Workbook%d"), m_nWorkbookNumber + 1);
	//::WritePrivateProfileStringA(_T("Workbook"), sectionName, m_strConfigName, tempIniFileName);

	////写入状态
	//sectionName.Format(_T("Status%d"), m_nWorkbookNumber + 1);
	//::WritePrivateProfileStringA(_T("Workbook"), sectionName, _T("0"), tempIniFileName);

	return 0;
}

//获取NVGate Input的参数，並写入到INI文件中
int CGenerateModel::GenNVGatePara(CString commandPara, int icount, CString paraName, CString section)
{
	CString tempStr = _T("");
	CString sectionName = _T("");
	int chIndex = 1;
	int iret = 0;
	int* startIndex = new int;
	int* indexChannel = new int;
	*startIndex = 0;
	*indexChannel = 0;

	//判断iCount的系统位置
	GetIndex(icount, *startIndex, *indexChannel, 0, m_numofSystem, m_pNumofChannel);
	//int iIndex = *startIndex+1;

	tempStr.Format(commandPara, icount);
	//发送获取信息
	iret = GetSettingValue(tempStr, str_Setting);
	if (iret == -1)
	{
		delete startIndex;
		delete indexChannel;
		startIndex = NULL;
		indexChannel = NULL;
		return -1;
	}

	sectionName.Format("SYS%d_%s%d", *startIndex, section, *indexChannel);
	//sectionName.Format("Input%d", icount);
	::WritePrivateProfileStringA(sectionName, paraName, str_Setting, iniFileName);

	delete startIndex;
	delete indexChannel;
	startIndex = NULL;
	indexChannel = NULL;

	return 0;

}

//获取NVGATE中的参数设置值
int CGenerateModel::GetSettingValue(CString commandPara, CString& settingValue)
{
	CString str_Command = _T("");
	int iret = 0;

	str_Command.Format(_T("GetSettingValue %s"), commandPara);

	iret = m_pTcpClient->SendDataToNVGate(str_Command);
	if (iret == -1 || iret == 1) return iret;

	//识别Setting的值
	m_pTcpClient->m_nCurrent = 10;
	CString type = _T("");
	m_pTcpClient->MakeString(m_pTcpClient->bDataFromNVGate, type);

	if (type == "String" || type == "Enumerated" || type == "Signal File")
	{
		m_pTcpClient->MakeString(m_pTcpClient->bDataFromNVGate, settingValue);
		return 0;
	}
	else if (type == "Boolean")
	{
		m_pTcpClient->MakeInteger(m_pTcpClient->bDataFromNVGate, settingValue);
		return 0;
	}
	else if (type == "Scalar")
	{
		m_pTcpClient->MakeFloat(m_pTcpClient->bDataFromNVGate, settingValue);
		return 0;
	}
	else
	{
		settingValue = _T("");
		return 1;
	}
}

//获取本地NVGate中的通道号在几号子系统，以及子系统中对应的物理通道号
//icount = 本地NVGate系统中的通道号
//indSystem = 子系统号
//indChannel = 子系统中的通道号
void CGenerateModel::GetIndex(int icount, int &indSystem, int &indChannel, int tempCount, int nbSystem, const int* m_pNumberOfInput)
{

	if (icount > m_pNumberOfInput[indSystem] + tempCount)
	{
		indSystem += 1;
		tempCount += m_pNumberOfInput[indSystem - 1];
		GetIndex(icount, indSystem, indChannel, tempCount, nbSystem, m_pNumberOfInput);
	}
	else
	{
		indChannel = icount - tempCount;
		indSystem += 1;
	}
	return;
}

//从字符串中获取通道号
//格式是001.001.000，或者frontEnd.inputx 中间那个数字或者x就是通道数
int CGenerateModel::GetCHIndexFromString(CString str_Setting)
{
	int indexChannel = 0;
	CString b[5] = { _T("") };
	int iflag = 0;
	char* next_b = NULL;

	//分解字符串
	USES_CONVERSION;
	char* strContent = T2A(str_Setting.GetBuffer(0));
	str_Setting.ReleaseBuffer();

	b[iflag] = strtok_s(strContent, ".", &next_b);
	while (b[iflag] != _T(""))
	{
		// Get next token:
		if (b[iflag] != _T(""))
		{
			iflag++;
			b[iflag] = strtok_s(NULL, ".", &next_b);
		}
	}
	next_b = NULL;   //变成空指针

	//CString astr = b[1].Mid(1, 1);

	if (b[1].Mid(0, 1) == _T("i"))
	{
		indexChannel = atoi(b[1].Mid(5));
	}
	else
	{
		indexChannel = atoi(b[1]);
	}

	return indexChannel;
}

void CGenerateModel::ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	while (!m_bTerminated)
	{
		if (PostMessage(m_hHideWnd, Msg, wParam, lParam))
			return;
		if (GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
			return;
		Sleep(1000);
	}
}

//////////////////////////////////////////////////////////////////////
// COperateRemoteNVGateThread
//////////////////////////////////////////////////////////////////////

COperateRemoteNVGate::COperateRemoteNVGate(CTcpClient *pTcpClient, int nPriority) : CThread(nPriority)
{
	//m_pGenerateModel = new CGenerateModel(pTcpClient, nPriority);
	m_pTcpClient = pTcpClient;
	m_hHideWnd = NULL;
	m_MainHwnd = NULL;
	m_hSocket = INVALID_SOCKET;

	m_nWaitTimeOut = DFT_WAITTIMEOUT;

	m_numofSystem = 0;
	m_pIpAddress = NULL;

	m_bBtnStates = new BTNSTATES();
	m_pRemoteDesktop = new CRemoteDesktop();
	ci_CountInfo = new COUNTINFO();

	//bDataFromNVGate = NULL;
	//msgLen = 0;
	//m_bGetResponseFinish = FALSE;

	//获取当前的应用程序路径名，确定INI文件的完整文件名
	CString strAppPath = _T("");
	GetModuleFileName(NULL, strAppPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	strAppPath.ReleaseBuffer();
	int nPos = strAppPath.ReverseFind('\\');
	strAppPath = strAppPath.Left(nPos);
	iniFileName.Format(_T("%s\\Custom.ini"), strAppPath);
}


COperateRemoteNVGate::~COperateRemoteNVGate(void)
{
	//delete m_pGenerateModel;
	delete m_bBtnStates;
	delete m_pRemoteDesktop;
	delete ci_CountInfo;
}

void COperateRemoteNVGate::Execute(void)
{
	int iret = 0;
	CString str_Temp = _T("");
	char strINIInfo[64] = { "" };
	ASSERT(m_hHideWnd != NULL);

	CString str_Feedback = _T("");
	ci_CountInfo->iSource = 3;
	ci_CountInfo->successFlag = 0;
	
	// 等待打开远程NVGate成功
	//HANDLE hWaitOpenNVGateEvents[2];
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);
	m_pTcpClient->SetTerminateStatus(FALSE);

	////禁用按钮
	//m_bBtnStates->b_ButtonStates = FALSE;
	//for (int i = 2; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	str_Temp.Format(_T("Attempting to connect the local NVGate of System #%d..."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//端口号为3000
	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)_T("127.0.0.1"));
	m_pTcpClient->SetPort(3000);
	if (!m_pTcpClient->Connect(3))
	{
		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

		//设置当前项目名称
		ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
		Sleep(1);

		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

		str_Temp.Format(_T("Connecting the local NVGate failed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Connecting to the local NVGate failed, please open the local NVGate or check the network connection."), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);


		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}
	else
	{
		str_Temp.Format(_T("Connecting the local NVGate succeed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);
	}

	SetSocketHandle(m_pTcpClient->m_hSocket);
	ASSERT(m_hSocket != INVALID_SOCKET);

	//获取当前项目名称
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting current project name of local NVGate..."), (LPARAM)m_pTcpClient);
	iret = m_pTcpClient->SendDataToNVGate(_T("GetCurrentProjectName"));
	if (iret == -1)  //SOCKET断开	
	{
		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//设置当前项目名称
		ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
		Sleep(1);
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Local NVGate is closed."), (LPARAM)m_pTcpClient);
		Sleep(1);
		MessageBox(m_MainHwnd, _T("Local NVGate is closed, please open the remote NVGate or check the network connection."),
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}

	m_pTcpClient->m_nCurrent = 10;
	m_pTcpClient->MakeString(m_pTcpClient->bDataFromNVGate, str_Temp);

	if (str_Temp == _T("Default Project"))
	{
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//设置当前项目名称
		ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
		Sleep(1);
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
		
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("The current project cannot be <Default Project>."), (LPARAM)m_pTcpClient);
		Sleep(1);

		MessageBox(m_MainHwnd, _T("The current project cannot be <Default Project>, please save as a new name."),
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//断开连接，销毁线程，关闭隐藏窗体
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}
	else
	{
		::WritePrivateProfileStringA(_T("Parameters"), _T("ProjectName"), str_Temp, iniFileName);
	}

	//加载项目
	//for (int nsys = 1; nsys <= m_numofSystem; nsys++)
	{
		//连接到远程计算机
		str_Temp.Format(_T("Attempting to connect the remote NVGate of System #%d..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		//端口号为3000
		m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
		m_pTcpClient->SetPort(3000);
		if (!m_pTcpClient->Connect(3))
		{
			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

			//设置当前项目名称
			ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
			Sleep(1);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Connecting to the remote NVGate #%d failed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//断开SOCKET，销毁线程
			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}
		else
		{
			str_Temp.Format(_T("Connecting the remote NVGate of System #%d succeed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);
		}

		SetSocketHandle(m_pTcpClient->m_hSocket);
		ASSERT(m_hSocket != INVALID_SOCKET);

		//加载Workbook
		str_Temp.Format(_T("Loading workbook model to remote NVGate of System #%d..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		iret = m_pTcpClient->SendDataToNVGate(_T("SetSilent 1"));
		if (iret == -1)  //SOCKET断开	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			//设置当前项目名称
			ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
			Sleep(1);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}

		GetPrivateProfileString(_T("Parameters"), _T("ProjectName"), _T(""), strINIInfo, 64, iniFileName);

		CString str_Temp2 = _T(strINIInfo);
		if (str_Temp2 == _T(""))
		{
			//ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			//设置当前项目名称
			ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
			Sleep(1);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting the current project of local NVGate failed!"), (LPARAM)m_pTcpClient);
			Sleep(1);

			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting the current project of local NVGate failed!"), (LPARAM)m_pTcpClient);
			MessageBox(m_MainHwnd, _T("Getting the current project of local NVGate failed!"), _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//设置当前项目名称
			ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
			Sleep(1);

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			
			return;
		}


		str_Temp2.Replace(_T(" "), _T("\\ "));

		str_Temp.Format(_T("NewProject %s"), str_Temp2);
		iret = m_pTcpClient->SendDataToNVGate(str_Temp);
		if (iret == -1)  //SOCKET断开	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			//设置当前项目名称
			ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
			Sleep(1);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		}
		else
		{
			if (iret == 1)
			{				
				str_Temp.Format(_T("LoadProject %s 0"), strINIInfo);
				iret = m_pTcpClient->SendDataToNVGate(str_Temp);
				if (iret == -1)  //SOCKET断开	
				{
					str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					//恢复鼠标光标
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					//设置当前项目名称
					ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
					Sleep(1);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
				}
				
			}
		}

		str_Temp.Format(_T("LoadWorkbookModel #%s_%d User"), m_sWorkbookName, m_numofSystem + 1);
		iret = m_pTcpClient->SendDataToNVGate(str_Temp);
		switch (iret)
		{
		case -1:
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			//设置当前项目名称
			ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
			Sleep(1);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		case 0:
			break;
		case 1:
			
			str_Temp.Format(_T("The workbook %s_%d isn't existing."), m_sWorkbookName, m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("The workbook %s_%d isn't existing, please send workbook files firstly."), m_sWorkbookName, m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			//设置当前项目名称
			ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
			Sleep(1);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		default:
			break;
		}
		if (iret == -1)  //SOCKET断开	
		{
			
		}

		

		iret = m_pTcpClient->SendDataToNVGate("GetFanInfo");
		if (iret == -1)  //SOCKET断开	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			//设置当前项目名称
			ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
			Sleep(1);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}

		m_pTcpClient->m_nCurrent = 10;
		m_pTcpClient->MakeLongInteger(m_pTcpClient->bDataFromNVGate, str_Feedback);
		CString str_FanStatur = str_Feedback;

		if (str_FanStatur != _T("1"))
		{
			//打开风扇
			iret = m_pTcpClient->SendDataToNVGate(_T("SetFanStatus 0"));
			if (iret == -1)  //SOCKET断开	
			{
				str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);

				str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
				MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

				//恢复鼠标光标
				ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
				//设置当前项目名称
				ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
				Sleep(1);
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

				ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
				return;
			}
		}

		iret = m_pTcpClient->SendDataToNVGate("GetFanInfo");
		if (iret == -1)  //SOCKET断开	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			//设置当前项目名称
			ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
			Sleep(1);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}

		m_pTcpClient->m_nCurrent = 14;
		m_pTcpClient->MakeLongInteger(m_pTcpClient->bDataFromNVGate, str_Feedback);
		int i_Temp = atoi(str_Feedback) + 1;

		
		for (int i = i_Temp; i <= 10; i++)
		{
			if (i_Temp == 10)
			{
				break;
			}
			str_Temp.Format(_T("SetFanSpeed %d"), i_Temp);
			iret = m_pTcpClient->SendDataToNVGate(str_Temp);
			if (iret == -1)  //SOCKET断开	
			{
				str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);

				str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
				MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

				//恢复鼠标光标
				ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
				//设置当前项目名称
				ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
				Sleep(1);
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
				ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
				return;
			}

			//读取FAN
			iret = m_pTcpClient->SendDataToNVGate("GetFanInfo");
			if (iret == -1)  //SOCKET断开	
			{
				str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);

				str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
				MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

				//恢复鼠标光标
				ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
				//设置当前项目名称
				ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
				Sleep(1);
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
				ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
				return;
			}

			m_pTcpClient->m_nCurrent = 14;
			m_pTcpClient->MakeLongInteger(m_pTcpClient->bDataFromNVGate, str_Feedback);

			i_Temp = atoi(str_Feedback) + 1;

		}


		////保存该项目
		//iret = m_pTcpClient->SendDataToNVGate(_T("SaveProject"));
		//if (iret == -1)  //SOCKET断开	
		//{
		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		//	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		//	Sleep(1);

		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		//	MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//	//恢复鼠标光标
		//	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//	//设置当前项目名称
		//	ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
		//	Sleep(1);
		//	ForcePostMessage(WM_FINISHCOUNT, 3, (LPARAM)m_pTcpClient);

		//	////启用按钮
		//	//m_bBtnStates->b_ButtonStates = TRUE;
		//	//for (int i = 3; i <= 9; i++)
		//	//{
		//	//	m_bBtnStates->indexButton = i;
		//	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
		//	//	Sleep(1);
		//	//}

		//	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		//	return;
		//}

		//断开连接
		//m_pTcpClient->Disconnect();
	}

	//销毁线程
	//恢复鼠标光标
	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

	str_Temp.Format(_T("Loading workbook model to remote NVGate of System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//设置当前项目名称
	ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(strINIInfo), (LPARAM)m_pTcpClient);
	Sleep(1);

	ci_CountInfo->successFlag = 1;
	ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
	Sleep(1);

	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

void COperateRemoteNVGate::ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	while (!m_bTerminated)
	{
		if (PostMessage(m_hHideWnd, Msg, wParam, lParam))
			return;
		if (GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
			return;
		Sleep(1000);
	}
}

void COperateRemoteNVGate::SetHideWndHandle(HWND hHideWnd)
{
	m_hHideWnd = hHideWnd;
}

void COperateRemoteNVGate::SetHwnd(HWND m_Hwnd)
{
	m_MainHwnd = m_Hwnd;
}

void COperateRemoteNVGate::SetWorkbookName(CString wrkName)
{
	m_sWorkbookName = wrkName;
}

void COperateRemoteNVGate::SetSocketHandle(SOCKET hSocket)
{

	m_hSocket = hSocket;
}

void COperateRemoteNVGate::SetIPAddress(CString *ipAddress)
{
	m_pIpAddress = ipAddress;
}

void COperateRemoteNVGate::SetNumOfSystem(int numofSystem)
{
	m_numofSystem = numofSystem;
}

//void COperateRemoteNVGate::OpenRemoteNVGateSucc()
//{
//	SetEvent(m_hOpenNVGateSucc);
//}

// 关闭文件发送线程
// 不发送任何网络消息给接收端
void COperateRemoteNVGate::CloseOperateNVGateThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // 等待线程结束
}


//////////////////////////////////////////////////////////////////////
// COpenRemoteNVGateThread
//////////////////////////////////////////////////////////////////////

COpenRemoteNVGate::COpenRemoteNVGate(CTcpClient *pTcpClient, int nPriority) : CThread(nPriority)
{
	m_pTcpClient = pTcpClient;
	m_hHideWnd = NULL;
	m_hSocket = INVALID_SOCKET;

	m_MainHwnd = NULL;

	m_nWaitTimeOut = DFT_WAITTIMEOUT;
	m_bBtnStates = new BTNSTATES();
	ci_CountInfo = new COUNTINFO();

	//获取当前的应用程序路径名，确定INI文件的完整文件名
	CString strAppPath = _T("");
	GetModuleFileName(NULL, strAppPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	strAppPath.ReleaseBuffer();
	int nPos = strAppPath.ReverseFind('\\');
	strAppPath = strAppPath.Left(nPos);
	iniFileName.Format(_T("%s\\Tempsetting.ini"), strAppPath);

}


COpenRemoteNVGate::~COpenRemoteNVGate()
{
	/*CloseHandle(m_hOpenNVGateFail);
	CloseHandle(m_hOpenNVGateSucc);*/
	delete m_bBtnStates;
	delete ci_CountInfo;
}

void COpenRemoteNVGate::Execute(void)
{
	int iret = 0;
	CString str_Temp = _T("");
	BOOL b_Temp = TRUE;
	CString str_KeyName = _T("");
	str_KeyName.Format(_T("System%d"), m_numofSystem + 1);

	//int i_pNVGateStartFlag;

	ASSERT(m_hHideWnd != NULL);

	ci_CountInfo->iSource = 1;
	ci_CountInfo->successFlag = 0;

	// 等待打开远程NVGate成功
	HANDLE hWaitOpenNVGateEvents[3];
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);

	//连接到远程计算机
	str_Temp.Format(_T("Attempting to connect the controller PC of System #%d..."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//端口号为8000
	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
	m_pTcpClient->SetPort(8000);
	if (!m_pTcpClient->Connect(0))
	{
		//统计完成数量，请交由主进程去处理
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

		//写入到NVGate的状态INI文件中
		::WritePrivateProfileStringA(_T("NVGateStatus"), str_KeyName, _T("OFF"), iniFileName);

		//显示错误信息
		str_Temp.Format(_T("Connecting the controller PC of System #%d failed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		//提示错误报警
		str_Temp.Format(_T("Connecting the controller PC of System #%d failed, please check the network connection!"), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//断开SOCKET，销毁线程
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}
	else
	{
		str_Temp.Format(_T("Connecting the controller PC of System #%d succeed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);
	}


	SetSocketHandle(m_pTcpClient->m_hSocket);

	ASSERT(m_hSocket != INVALID_SOCKET);

	m_hOpenNVGateFail = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hOpenNVGateSucc = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hOpenNVGateMute = CreateEvent(NULL, TRUE, FALSE, NULL);

	hWaitOpenNVGateEvents[0] = m_hOpenNVGateFail;
	hWaitOpenNVGateEvents[1] = m_hOpenNVGateSucc;
	hWaitOpenNVGateEvents[2] = m_hOpenNVGateMute;

	////远程打开NVGate
	SendRequestOpenNVGateNetMsg(m_numofSystem);

	switch (WaitForMultipleObjects(3, hWaitOpenNVGateEvents, FALSE, m_nWaitTimeOut))
	{
	case WAIT_OBJECT_0: // 打开错误
		CloseHandle(m_hOpenNVGateFail);
		CloseHandle(m_hOpenNVGateSucc);
		CloseHandle(m_hOpenNVGateMute);

		//统计完成数量，请交由主进程去处理
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

		//ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//写入到NVGate的状态INI文件中
		::WritePrivateProfileStringA(_T("NVGateStatus"), str_KeyName, _T("OFF"), iniFileName);

		str_Temp.Format(_T("Open remote NVGate in System #%d failed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Open remote NVGate in System #%d failed, please check the network connection!"), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		//delete[] i_pNVGateStartFlag;
		return;
		break;
	case WAIT_OBJECT_0 + 1: // 打开成功
		CloseHandle(m_hOpenNVGateFail);
		CloseHandle(m_hOpenNVGateSucc);
		CloseHandle(m_hOpenNVGateMute);
		break;
	case WAIT_OBJECT_0 + 2: // 主体关闭
		CloseHandle(m_hOpenNVGateFail);
		CloseHandle(m_hOpenNVGateSucc);
		CloseHandle(m_hOpenNVGateMute);

		//统计完成数量，请交由主进程去处理
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

		//ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//写入到NVGate的状态INI文件中
		::WritePrivateProfileStringA(_T("NVGateStatus"), str_KeyName, _T("OFF"), iniFileName);

		str_Temp.Format(_T("Open remote NVGate in System #%d failed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
		break;
	case WAIT_TIMEOUT: // 等待超时
		CloseHandle(m_hOpenNVGateFail);
		CloseHandle(m_hOpenNVGateSucc);
		CloseHandle(m_hOpenNVGateMute);
		//统计完成数量，请交由主进程去处理
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

		//写入到NVGate的状态INI文件中
		::WritePrivateProfileStringA(_T("NVGateStatus"), str_KeyName, _T("OFF"), iniFileName);

		//ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		str_Temp.Format(_T("Open remote NVGate in System #%d failed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Open remote NVGate in System #%d failed, please check the network connection!"), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		//delete[] i_pNVGateStartFlag;
		return;
		break;
	default:
		CloseHandle(m_hOpenNVGateFail);
		CloseHandle(m_hOpenNVGateSucc);
		CloseHandle(m_hOpenNVGateMute);
		//统计完成数量，请交由主进程去处理
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

		//写入到NVGate的状态INI文件中
		::WritePrivateProfileStringA(_T("NVGateStatus"), str_KeyName, _T("OFF"), iniFileName);

		//ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		str_Temp.Format(_T("Open remote NVGate in System #%d failed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Open remote NVGate in System #%d failed, please check the network connection!"), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		//delete[] i_pNVGateStartFlag;
		return;
		break;
	}

	//销毁线程
	//恢复鼠标光标
	//delete[] i_pNVGateStartFlag;
	//统计完成数量，请交由主进程去处理
	ci_CountInfo->successFlag = 1;
	ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

	//写入到NVGate的状态INI文件中
	::WritePrivateProfileStringA(_T("NVGateStatus"), str_KeyName, _T("ON"), iniFileName);

	//m_pTcpClient->GetAnalyzerStatus(m_pIpAddress, m_numofSystem, AfxGetMainWnd()->m_hWnd);

	//ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
	str_Temp.Format(_T("Open remote NVGate in System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);
	
	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// 关闭文件发送线程
// 不发送任何网络消息给接收端
void COpenRemoteNVGate::CloseOpenNVGateThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // 等待线程结束
}

void COpenRemoteNVGate::ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	while (!m_bTerminated)
	{
		if (PostMessage(m_hHideWnd, Msg, wParam, lParam))
			return;
		if (GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
			return;
		Sleep(1000);
	}
}

void COpenRemoteNVGate::SetHideWndHandle(HWND hHideWnd)
{
	m_hHideWnd = hHideWnd;
}

void COpenRemoteNVGate::SetSocketHandle(SOCKET hSocket)
{
	m_hSocket = hSocket;
}



void COpenRemoteNVGate::SetIPAddress(CString *ipAddress)
{
	m_pIpAddress = ipAddress;
}

void COpenRemoteNVGate::SetNumOfSystem(int numofSystem)
{
	m_numofSystem = numofSystem;
}

void COpenRemoteNVGate::SetHardIpAddr(CString *HardIpAddr)
{
	m_pHardIpAddr = HardIpAddr;
}

void COpenRemoteNVGate::SetHardType(CString *HardType)
{
	m_PHardType = HardType;
}

void COpenRemoteNVGate::SetHwnd(HWND m_Hwnd)
{
	m_MainHwnd = m_Hwnd;
}

void COpenRemoteNVGate::OpenRemoteNVGateSucc()
{
	SetEvent(m_hOpenNVGateSucc);
}

void COpenRemoteNVGate::OpenRemoteNVGateFail()
{
	SetEvent(m_hOpenNVGateFail);
}

void COpenRemoteNVGate::OpenRemoteNVGateMute()
{
	SetEvent(m_hOpenNVGateMute);
}

// 发送请求发送文件网络消息
BOOL COpenRemoteNVGate::SendRequestOpenNVGateNetMsg(int nSys)
{
	int nFileNameLen;
	int nFolderNameLen;
	char Buf[19 + MAX_PATH];

	
	//char szFileSize[13];
	CString str_CommandLine = _T("");

	if (m_PHardType[nSys] == _T("T"))
	{
		str_CommandLine.Format(_T("-online -teamwork -ListHardIpAddr=%s -default -silent -silentEx -noRunPauseStop"),m_pHardIpAddr[nSys]);
	}
	else if (m_PHardType[nSys] == _T("S"))
	{
		str_CommandLine.Format(_T("-online -ListHardIpAddr=%s -default -silent -silentEx -noRunPauseStop"), m_pHardIpAddr[nSys]);
	}
	else
	{
		str_CommandLine = _T("-offline -default -silent -silentEx -noRunPauseStop");
	}
	
	nFileNameLen = str_CommandLine.GetLength();

	strncpy(&(Buf[6]), "-46", 3);
	//strncpy(&(Buf[9]), szFileSize, 12);

	strncpy(&(Buf[9]), str_CommandLine, nFileNameLen);

	return m_pTcpClient->SendNetMsg(Buf, 6 + nFileNameLen, 0);
}

//////////////////////////////////////////////////////////////////////
// CAlarmRecorderThread
//////////////////////////////////////////////////////////////////////


CAlarmSystem::CAlarmSystem(CTcpClient *pTcpClient, int nPriority) : CThread(nPriority)
{
	m_pTcpClient = pTcpClient;
	m_hHideWnd = NULL;
	m_hSocket = INVALID_SOCKET;

	//m_nCurrent = 0;
	m_numofSystem = 0;
	m_pIpAddress = NULL;
	//msgLen = 0;
	//bDataFromNVGate = NULL;
	//m_bGetResponseFinish = FALSE;
	//m_nWaitTimeOut = DFT_WAITTIMEOUT;

	m_bBtnStates = new BTNSTATES();
	ci_CountInfo = new COUNTINFO();
}


CAlarmSystem::~CAlarmSystem()
{
	delete m_bBtnStates;
	delete ci_CountInfo;
}

void CAlarmSystem::Execute(void)
{
	
	int iret = 0;
	CString str_Temp = _T("");
	CString str_Feedback = _T("");
	CString str_StopFlag = _T("");

	
	ci_CountInfo->iSource = 4;
	ci_CountInfo->successFlag = 0;

	ASSERT(m_hHideWnd != NULL);
	
	//等待鼠标的光标
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);
	m_pTcpClient->SetTerminateStatus(FALSE);

	////禁用按钮
	//m_bBtnStates->b_ButtonStates = FALSE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	//加载项目
	//for (int nsys = 1; nsys <= m_numofSystem; nsys++)
	{
		//连接到远程计算机
		str_Temp.Format(_T("Attempting to connect the remote NVGate of System #%d..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		//端口号为3000
		m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
		m_pTcpClient->SetPort(3000);
		if (!m_pTcpClient->Connect(3))
		{
			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Connecting to the remote NVGate #%d failed, please open the remote NVGate or check the network connection."), m_numofSystem+1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			////启用按钮
			//m_bBtnStates->b_ButtonStates = TRUE;
			//for (int i = 3; i <= 9; i++)
			//{
			//	m_bBtnStates->indexButton = i;
			//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			//	Sleep(1);
			//}
			//断开SOCKET，销毁线程
			
			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}
		else
		{
			str_Temp.Format(_T("Connecting the remote NVGate of System #%d succeed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);
		}

		SetSocketHandle(m_pTcpClient->m_hSocket);

		ASSERT(m_hSocket != INVALID_SOCKET);

		//Alarm the recorder
		str_Temp.Format(_T("Saving current project in NVGate of System #%d ..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue 7 208 396 0"));
		if (iret == -1)  //SOCKET断开	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}

		iret = m_pTcpClient->SendDataToNVGate(_T("SaveProject"));
		if (iret == -1)  //SOCKET断开	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}


		//Alarm the recorder
		str_Temp.Format(_T("Arming the remote NVGate of System #%d ..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		//停止所有主机的测量，防止有主机在运行
		iret = m_pTcpClient->SendDataToNVGate("GetGeneralAnalyzerState");
		if (iret == -1)  //SOCKET断开	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem+1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			////启用按钮
			//m_bBtnStates->b_ButtonStates = TRUE;
			//for (int i = 3; i <= 9; i++)
			//{
			//	m_bBtnStates->indexButton = i;
			//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			//	Sleep(1);
			//}

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}

		m_pTcpClient->m_nCurrent = 10;
		m_pTcpClient->MakeInteger(m_pTcpClient->bDataFromNVGate, str_Feedback);
		if (str_Feedback == _T("0"))
		{
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting analyzer states failed."), (LPARAM)m_pTcpClient);
			Sleep(1);

			MessageBox(m_MainHwnd, _T("Getting analyzer states failed, please check!"), _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			////启用按钮
			//m_bBtnStates->b_ButtonStates = TRUE;
			//for (int i = 3; i <= 9; i++)
			//{
			//	m_bBtnStates->indexButton = i;
			//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			//	Sleep(1);
			//}

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}
		

		if (str_Feedback != "11")
		{
			if (str_Feedback != "4")
			{
				//设置选项为Not Save
				iret = m_pTcpClient->SendDataToNVGate(_T("SetSaveOption 0"));
				if (iret == -1)  //SOCKET断开	
				{
					str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem+1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					//恢复鼠标光标
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					////启用按钮
					//m_bBtnStates->b_ButtonStates = TRUE;
					//for (int i = 3; i <= 9; i++)
					//{
					//	m_bBtnStates->indexButton = i;
					//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
					//	Sleep(1);
					//}

					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
				}

				//停止一切测量，不保存
				iret = m_pTcpClient->SendDataToNVGate(_T("Stop"));
				if (iret == -1)  //SOCKET断开	
				{
					str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					//恢复鼠标光标
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
				}

				//设置保存选项为save without name confirmation
				iret = m_pTcpClient->SendDataToNVGate(_T("SetSaveOption 3"));
				if (iret == -1)  //SOCKET断开	
				{
					str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					//恢复鼠标光标
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					////启用按钮
					//m_bBtnStates->b_ButtonStates = TRUE;
					//for (int i = 3; i <= 9; i++)
					//{
					//	m_bBtnStates->indexButton = i;
					//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
					//	Sleep(1);
					//}

					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
				}

				//获取分析仪状态
				iret = m_pTcpClient->SendDataToNVGate(_T("GetGeneralAnalyzerState"));
				if (iret == -1)  //SOCKET断开	
				{
					str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					//恢复鼠标光标
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					////启用按钮
					//m_bBtnStates->b_ButtonStates = TRUE;
					//for (int i = 3; i <= 9; i++)
					//{
					//	m_bBtnStates->indexButton = i;
					//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
					//	Sleep(1);
					//}

					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
				}

				m_pTcpClient->m_nCurrent = 10;
				m_pTcpClient->MakeInteger(m_pTcpClient->bDataFromNVGate, str_StopFlag);

				DWORD nStartTime = GetTickCount();
				DWORD nCurrentTime = 0;
				DWORD nDeltaTime = 0;
				/*DWORD nRefreshTime = pResultPara->refreshRate;*/
				DWORD nTimeout = 40000;  //200ms

				//直到分析仪的状态为STOP,40s timeout
				while (str_StopFlag != "4")
				{
					nCurrentTime = GetTickCount();
					nDeltaTime = nCurrentTime - nStartTime;
					//超时
					if (nDeltaTime > nTimeout)
					{
						ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

						str_Temp.Format(_T("Stopping the measurement of System #%d is timeout."), m_numofSystem + 1);
						ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
						Sleep(1);

						str_Temp.Format(_T("Stopping the measurement of System #%d is timeout."), m_numofSystem + 1);
						MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

						////启用按钮
						//m_bBtnStates->b_ButtonStates = TRUE;
						//for (int i = 3; i <= 9; i++)
						//{
						//	m_bBtnStates->indexButton = i;
						//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
						//	Sleep(1);
						//}

						ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
						return;
					}
					//nStartTime = GetTickCount();

					//获取分析仪状态
					iret = m_pTcpClient->SendDataToNVGate(_T("GetGeneralAnalyzerState"));
					if (iret == -1)  //SOCKET断开	
					{
						str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
						ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
						Sleep(1);

						str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
						MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

						//恢复鼠标光标
						ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

						////启用按钮
						//m_bBtnStates->b_ButtonStates = TRUE;
						//for (int i = 3; i <= 9; i++)
						//{
						//	m_bBtnStates->indexButton = i;
						//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
						//	Sleep(1);
						//}

						ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
						return;
					}

					m_pTcpClient->m_nCurrent = 10;
					m_pTcpClient->MakeInteger(m_pTcpClient->bDataFromNVGate, str_StopFlag);

					if (str_Feedback == _T("0"))
					{
						ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

						ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting analyzer states failed."), (LPARAM)m_pTcpClient);
						Sleep(1);

						MessageBox(m_MainHwnd, _T("Getting analyzer states failed, please check!"), _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

						////启用按钮
						//m_bBtnStates->b_ButtonStates = TRUE;
						//for (int i = 3; i <= 9; i++)
						//{
						//	m_bBtnStates->indexButton = i;
						//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
						//	Sleep(1);
						//}

						ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
						return;
					}
				}
			}
		}


		Sleep(1000);

		//变成ARM状态, ARM的ID是3 624 407
		//if (str_Feedback != "Armed")
		{
			iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue 3 624 407 1"));
			if (iret == -1)  //SOCKET断开	
			{
				str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);

				str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
				MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

				//恢复鼠标光标
				ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

				//启用按钮
				m_bBtnStates->b_ButtonStates = TRUE;
				for (int i = 3; i <= 9; i++)
				{
					m_bBtnStates->indexButton = i;
					ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
					Sleep(1);
				}

				ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
				return;
			}

			if (iret == 1)
			{
				str_Temp.Format(_T("Arming the remote NVGate of System #%d failed."), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);

				str_Temp.Format(_T("Arming the Remote NVGate of System #%d failed, please check the NVGate workbook configuration."), m_numofSystem + 1);
				MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

				//恢复鼠标光标
				ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

				////启用按钮
				//m_bBtnStates->b_ButtonStates = TRUE;
				//for (int i = 3; i <= 9; i++)
				//{
				//	m_bBtnStates->indexButton = i;
				//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
				//	Sleep(1);
				//}

				ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
				return;
			}

			//获取分析仪状态
			iret = m_pTcpClient->SendDataToNVGate(_T("GetGeneralAnalyzerState"));
			if (iret == -1)  //SOCKET断开	
			{
				str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);

				str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
				MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

				//恢复鼠标光标
				ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

				////启用按钮
				//m_bBtnStates->b_ButtonStates = TRUE;
				//for (int i = 3; i <= 9; i++)
				//{
				//	m_bBtnStates->indexButton = i;
				//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
				//	Sleep(1);
				//}

				ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
				return;
			}

			m_pTcpClient->m_nCurrent = 10;
			m_pTcpClient->MakeInteger(m_pTcpClient->bDataFromNVGate, str_StopFlag);
			if (str_Feedback == _T("0"))
			{
				ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting analyzer states failed."), (LPARAM)m_pTcpClient);
				Sleep(1);

				MessageBox(m_MainHwnd, _T("Getting analyzer states failed, please check!"), _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

				////启用按钮
				//m_bBtnStates->b_ButtonStates = TRUE;
				//for (int i = 3; i <= 9; i++)
				//{
				//	m_bBtnStates->indexButton = i;
				//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
				//	Sleep(1);
				//}

				ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
				return;
			}

			DWORD nStartTime = GetTickCount();
			DWORD nCurrentTime = 0;
			DWORD nDeltaTime = 0;
			/*DWORD nRefreshTime = pResultPara->refreshRate;*/
			DWORD nTimeout = 40000;  //200ms

			//直到分析仪的状态为STOP,40s timeout
			while (str_StopFlag != "11")
			{
				nCurrentTime = GetTickCount();
				nDeltaTime = nCurrentTime - nStartTime;
				//超时
				if (nDeltaTime > nTimeout)
				{
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					str_Temp.Format(_T("Arming the remote NVGate of System #%d is timeout."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Arming the remote NVGate of System #%d is timeout."), m_numofSystem + 1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					////启用按钮
					//m_bBtnStates->b_ButtonStates = TRUE;
					//for (int i = 3; i <= 9; i++)
					//{
					//	m_bBtnStates->indexButton = i;
					//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
					//	Sleep(1);
					//}

					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
				}
				//nStartTime = GetTickCount();

				//获取分析仪状态
				iret = m_pTcpClient->SendDataToNVGate(_T("GetGeneralAnalyzerState"));
				if (iret == -1)  //SOCKET断开	
				{
					str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					//恢复鼠标光标
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					////启用按钮
					//m_bBtnStates->b_ButtonStates = TRUE;
					//for (int i = 3; i <= 9; i++)
					//{
					//	m_bBtnStates->indexButton = i;
					//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
					//	Sleep(1);
					//}

					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
				}

				m_pTcpClient->m_nCurrent = 10;
				m_pTcpClient->MakeInteger(m_pTcpClient->bDataFromNVGate, str_StopFlag);
				if (str_Feedback == _T("0"))
				{
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting analyzer states failed."), (LPARAM)m_pTcpClient);
					Sleep(1);

					MessageBox(m_MainHwnd, _T("Getting analyzer states failed, please check!"), _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					////启用按钮
					//m_bBtnStates->b_ButtonStates = TRUE;
					//for (int i = 3; i <= 9; i++)
					//{
					//	m_bBtnStates->indexButton = i;
					//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
					//	Sleep(1);
					//}

					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
				}

			}
		}

	}

	//销毁线程
	//恢复鼠标光标
	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
	ci_CountInfo->successFlag = 1;
	ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

	str_Temp.Format(_T("Arming the remote NVGate of System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	////启用按钮
	//m_bBtnStates->b_ButtonStates = TRUE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// 关闭文件发送线程
// 不发送任何网络消息给接收端
void CAlarmSystem::CloseAlarmSystemThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // 等待线程结束
}

void CAlarmSystem::ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	while (!m_bTerminated)
	{
		if (PostMessage(m_hHideWnd, Msg, wParam, lParam))
			return;
		if (GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
			return;
		Sleep(1000);
	}
}

void CAlarmSystem::SetHideWndHandle(HWND hHideWnd)
{
	m_hHideWnd = hHideWnd;
}

void CAlarmSystem::SetSocketHandle(SOCKET hSocket)
{

	m_hSocket = hSocket;
}

void CAlarmSystem::SetIPAddress(CString *ipAddress)
{
	m_pIpAddress = ipAddress;
}

void CAlarmSystem::SetNumOfSystem(int numofSystem)
{
	m_numofSystem = numofSystem;
}

void CAlarmSystem::SetHwnd(HWND m_Hwnd)
{
	m_MainHwnd = m_Hwnd;
}

////发送数据给NVGate，并收到反馈信息
////返回 -1：SOCKET断开；1：出现错误代码；0：正确执行
//int CAlarmSystem::SendDataToNVGate(CString str_Command)
//{
//	char chr_Command[256];
//	str_Command = str_Command + '\n';
//	m_bGetResponseFinish = FALSE;
//	sprintf(chr_Command, "%s", str_Command);
//
//	if (!m_bTerminated)
//	{
//		m_pTcpClient->SendNetMsg(chr_Command, strlen(chr_Command), 1);
//	}
//	else
//	{
//		return -1;  //SOCKET断开
//	}
//
//	while (!m_bGetResponseFinish)
//	{
//		printf("%d", m_bTerminated);
//		if (!m_bTerminated)
//		{
//			Sleep(1);
//		}
//		else
//		{
//			return -1;  //SOCKET断开
//		}
//
//	}
//
//	if (bDataFromNVGate[0] == NVSUC_TAG) return 0;   //正确执行
//	else return 1;    //出现错误
//}
//
////处理收到的反馈信息
//void CAlarmSystem::GetResString(char *Msg, int nMsgLen)
//{
//
//	bDataFromNVGate = Msg;
//	msgLen = nMsgLen;
//	m_bGetResponseFinish = TRUE;
//}
//
////获取字符串
//void CAlarmSystem::MakeString(char* bDataFromNVGate, CString & szString)
//{
//	szString = bDataFromNVGate + m_nCurrent;
//	m_nCurrent += szString.GetLength() + 1;
//	return;
//}
//
//void CAlarmSystem::MakeInteger(char* bDataFromNVGATE, CString& SettingValue)
//{
//	short *a = (short*)(bDataFromNVGATE + m_nCurrent);
//	SettingValue.Format("%d", *a);
//}
//
//void CAlarmSystem::MakeFloat(char* bDataFromNVGATE, CString& SettingValue)
//{
//	float *a = (float*)(bDataFromNVGATE + m_nCurrent);
//	if (*a >= 1){//之前为直接强转为int的代码，会把小数丢失
//		int b = (int)(*a);
//		SettingValue.Format("%d", b);
//	}
//	else if (*a<1){//为了维护原来的代码，此处
//		SettingValue.Format("%f", *a);
//	}
//	return;
//}


//////////////////////////////////////////////////////////////////////
// CStartMonitorThread
//////////////////////////////////////////////////////////////////////


CStartMonitor::CStartMonitor(CTcpClient *pTcpClient, int nPriority) : CThread(nPriority)
{
	m_pTcpClient = pTcpClient;
	m_hHideWnd = NULL;
	m_hSocket = INVALID_SOCKET;
	m_MainHwnd = NULL;

	m_numofSystem = 0;
	m_pIpAddress = NULL;
	m_bBtnStates = new BTNSTATES();
	ci_CountInfo = new COUNTINFO();
	//msgLen = 0;
	//bDataFromNVGate = NULL;
	//m_bGetResponseFinish = FALSE;

	//CString str_StopFlag = _T("");

	//m_nWaitTimeOut = DFT_WAITTIMEOUT;
}


CStartMonitor::~CStartMonitor()
{
	delete m_bBtnStates;
	delete ci_CountInfo;
}

void CStartMonitor::Execute(void)
{

	int iret = 0;
	CString str_Temp = _T("");
	//CString str_Feedback = _T("");
	//CString str_StopFlag = _T("");
	ASSERT(m_hHideWnd != NULL);
	ci_CountInfo->iSource = 4;
	ci_CountInfo->successFlag = 0;
	//等待鼠标的光标
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);
	m_pTcpClient->SetTerminateStatus(FALSE);

	////禁用按钮
	//m_bBtnStates->b_ButtonStates = FALSE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	//加载项目
	//for (int nsys = 1; nsys <= m_numofSystem; nsys++)
	{
		//连接到远程计算机
		str_Temp.Format(_T("Attempting to connect the remote NVGate of System #%d..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		//端口号为3000
		m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
		m_pTcpClient->SetPort(3000);
		if (!m_pTcpClient->Connect(3))
		{
			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
			str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Connecting to the remote NVGate #%d failed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			////启用按钮
			//m_bBtnStates->b_ButtonStates = TRUE;
			//for (int i = 3; i <= 9; i++)
			//{
			//	m_bBtnStates->indexButton = i;
			//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			//	Sleep(1);
			//}

			//断开SOCKET，销毁线程
			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}
		else
		{
			str_Temp.Format(_T("Connecting the remote NVGate of System #%d succeed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);
		}

		SetSocketHandle(m_pTcpClient->m_hSocket);
		ASSERT(m_hSocket != INVALID_SOCKET);

		//Alarm the recorder
		str_Temp.Format(_T("Saving current project in NVGate of System #%d ..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue 7 208 396 0"));
		if (iret == -1)  //SOCKET断开	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}

		iret = m_pTcpClient->SendDataToNVGate(_T("SaveProject"));
		if (iret == -1)  //SOCKET断开	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}

		//Starting the monitor
		str_Temp.Format(_T("Starting monitoring in System #%d"), m_numofSystem + 1);
		ForcePostMessage(WM_GETSOFTWAREMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		iret = m_pTcpClient->SendDataToNVGate(_T("Run"));
		if (iret == -1)  //SOCKET断开	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			////启用按钮
			//m_bBtnStates->b_ButtonStates = TRUE;
			//for (int i = 3; i <= 9; i++)
			//{
			//	m_bBtnStates->indexButton = i;
			//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			//	Sleep(1);
			//}

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}



	}

	//销毁线程
	//恢复鼠标光标
	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
	ci_CountInfo->successFlag = 1;
	ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

	str_Temp.Format(_T("Starting monitoring in System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	////启用按钮
	//m_bBtnStates->b_ButtonStates = TRUE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// 关闭文件发送线程
// 不发送任何网络消息给接收端
void CStartMonitor::CloseStartMonitorThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // 等待线程结束
}

void CStartMonitor::ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	while (!m_bTerminated)
	{
		if (PostMessage(m_hHideWnd, Msg, wParam, lParam))
			return;
		if (GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
			return;
		Sleep(1000);
	}
}

void CStartMonitor::SetHideWndHandle(HWND hHideWnd)
{
	m_hHideWnd = hHideWnd;
}

void CStartMonitor::SetSocketHandle(SOCKET hSocket)
{

	m_hSocket = hSocket;
}

void CStartMonitor::SetIPAddress(CString *ipAddress)
{
	m_pIpAddress = ipAddress;
}

void CStartMonitor::SetNumOfSystem(int numofSystem)
{
	m_numofSystem = numofSystem;
}

void CStartMonitor::SetHwnd(HWND m_Hwnd)
{
	m_MainHwnd = m_Hwnd;
}

//////////////////////////////////////////////////////////////////////
// CStartRecordThread
//////////////////////////////////////////////////////////////////////


CStartRecord::CStartRecord(CTcpClient *pTcpClient, int nPriority) : CThread(nPriority)
{
	m_pTcpClient = pTcpClient;
	m_hHideWnd = NULL;
	m_MainHwnd = NULL;
	m_hSocket = INVALID_SOCKET;

	//m_nCurrent = 0;
	m_numofSystem = 0;
	m_pIpAddress = NULL;
	//msgLen = 0;
	//bDataFromNVGate = NULL;
	//m_bGetResponseFinish = FALSE;
	//CString str_StopFlag = _T("");
	//m_nWaitTimeOut = DFT_WAITTIMEOUT;
	m_bBtnStates = new BTNSTATES();
}


CStartRecord::~CStartRecord()
{
	delete m_bBtnStates;
}

void CStartRecord::Execute(void)
{

	int iret = 0;
	CString str_Temp = _T("");
	CString str_Feedback = _T("");
	CString str_StopFlag = _T("");

	ASSERT(m_hHideWnd != NULL);

	//等待鼠标的光标
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);

	m_pTcpClient->SetTerminateStatus(FALSE);

	//禁用按钮
	m_bBtnStates->b_ButtonStates = FALSE;
	for (int i = 3; i <= 9; i++)
	{
		m_bBtnStates->indexButton = i;
		ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
		Sleep(1);
	}

	//连接到远程计算机
	str_Temp.Format(_T("Attempting to connect the remote NVGate of System #%d..."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//端口号为3000
	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
	m_pTcpClient->SetPort(3000);
	if (!m_pTcpClient->Connect(3))
	{
		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);
		str_Temp.Format(_T("Connecting to the remote NVGate #%d failed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		//启用按钮
		m_bBtnStates->b_ButtonStates = TRUE;
		for (int i = 3; i <= 9; i++)
		{
			m_bBtnStates->indexButton = i;
			ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			Sleep(1);
		}
		//断开SOCKET，销毁线程

		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}
	else
	{
		str_Temp.Format(_T("Connecting the remote NVGate of System #%d succeed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);
	}

	SetSocketHandle(m_pTcpClient->m_hSocket);

	ASSERT(m_hSocket != INVALID_SOCKET);

	//Start recording
	str_Temp.Format(_T("Starting recording in System #%d..."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue 7 208 396 1"));
	if (iret == -1)  //SOCKET断开	
	{
		str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

		//启用按钮
		m_bBtnStates->b_ButtonStates = TRUE;
		for (int i = 3; i <= 9; i++)
		{
			m_bBtnStates->indexButton = i;
			ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			Sleep(1);
		}

		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}

	Sleep(1000);

	//iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue 7 208 396 0"));
	//if (iret == -1)  //SOCKET断开	
	//{
	//	str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
	//	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	//	Sleep(1);

	//	str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
	//	MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

	//	//恢复鼠标光标
	//	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

	//	//启用按钮
	//	m_bBtnStates->b_ButtonStates = TRUE;
	//	for (int i = 3; i <= 9; i++)
	//	{
	//		m_bBtnStates->indexButton = i;
	//		ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//		Sleep(1);
	//	}

	//	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
	//	return;
	//}

	//销毁线程
	//恢复鼠标光标
	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

	str_Temp.Format(_T("Starting the recording in System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//启用按钮
	m_bBtnStates->b_ButtonStates = TRUE;
	for (int i = 3; i <= 9; i++)
	{
		m_bBtnStates->indexButton = i;
		ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
		Sleep(1);
	}

	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// 关闭文件发送线程
// 不发送任何网络消息给接收端
void CStartRecord::CloseStartRecordThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // 等待线程结束
}

void CStartRecord::ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	while (!m_bTerminated)
	{
		if (PostMessage(m_hHideWnd, Msg, wParam, lParam))
			return;
		if (GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
			return;
		Sleep(1000);
	}
}

void CStartRecord::SetHideWndHandle(HWND hHideWnd)
{
	m_hHideWnd = hHideWnd;
}

void CStartRecord::SetSocketHandle(SOCKET hSocket)
{

	m_hSocket = hSocket;
}

void CStartRecord::SetIPAddress(CString *ipAddress)
{
	m_pIpAddress = ipAddress;
}

void CStartRecord::SetNumOfSystem(int numofSystem)
{
	m_numofSystem = numofSystem;
}

void CStartRecord::SetHwnd(HWND m_Hwnd)
{
	m_MainHwnd = m_Hwnd;
}

//////////////////////////////////////////////////////////////////////
// CStopRecordThread
//////////////////////////////////////////////////////////////////////


CStopRecord::CStopRecord(CTcpClient *pTcpClient, int nPriority) : CThread(nPriority)
{
	m_pTcpClient = pTcpClient;
	m_hHideWnd = NULL;
	m_MainHwnd = NULL;
	m_hSocket = INVALID_SOCKET;

	//m_nCurrent = 0;
	m_numofSystem = 0;
	//m_pIpAddress = NULL;
	m_pIpAddress = NULL;
	m_bBtnStates = new BTNSTATES();
	ci_CountInfo = new COUNTINFO();
	//msgLen = 0;
	//bDataFromNVGate = NULL;
	//m_bGetResponseFinish = FALSE;

	//CString str_StopFlag = _T("");

	//m_nWaitTimeOut = DFT_WAITTIMEOUT;
}


CStopRecord::~CStopRecord()
{
	delete m_bBtnStates;
	delete ci_CountInfo;
}

void CStopRecord::Execute(void)
{

	int iret = 0;
	CString str_Temp = _T("");
	ASSERT(m_hHideWnd != NULL);
	ci_CountInfo->iSource = 4;
	ci_CountInfo->successFlag = 0;
	//等待鼠标的光标
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);
	m_pTcpClient->SetTerminateStatus(FALSE);

	//禁用按钮
	//m_bBtnStates->b_ButtonStates = FALSE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}
		
	//for (int nsys = 1; nsys <= m_numofSystem; nsys++)
	{
		//连接到远程计算机
		str_Temp.Format(_T("Attempting to connect the remote NVGate of System #%d..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		//端口号为3000
		m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
		m_pTcpClient->SetPort(3000);
		if (!m_pTcpClient->Connect(3))
		{
			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
			str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);
			str_Temp.Format(_T("Connecting to the remote NVGate #%d failed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);
			////启用按钮
			//m_bBtnStates->b_ButtonStates = TRUE;
			//for (int i = 3; i <= 9; i++)
			//{
			//	m_bBtnStates->indexButton = i;
			//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			//	Sleep(1);
			//}
			//断开SOCKET，销毁线程

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}
		else
		{
			str_Temp.Format(_T("Connecting the remote NVGate of System #%d succeed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);
		}

		SetSocketHandle(m_pTcpClient->m_hSocket);

		ASSERT(m_hSocket != INVALID_SOCKET);


		//Stop and save the recorder
		str_Temp.Format(_T("Stop and saving the measurement in System #%d ..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		iret = m_pTcpClient->SendDataToNVGate(_T("SetSaveOption 3"));
		if (iret == -1)  //SOCKET断开	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			////启用按钮
			//m_bBtnStates->b_ButtonStates = TRUE;
			//for (int i = 3; i <= 9; i++)
			//{
			//	m_bBtnStates->indexButton = i;
			//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			//	Sleep(1);
			//}

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}

		//停止同步信号输出
		iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue 7 208 396 0"));
		if (iret == -1)  //SOCKET断开	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}

		iret = m_pTcpClient->SendDataToNVGate(_T("Stop"));
		if (iret == -1)  //SOCKET断开	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//恢复鼠标光标
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			////启用按钮
			//m_bBtnStates->b_ButtonStates = TRUE;
			//for (int i = 3; i <= 9; i++)
			//{
			//	m_bBtnStates->indexButton = i;
			//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			//	Sleep(1);
			//}

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}

		Sleep(2000);


		////保存数据并指定文件名
		//iret = m_pTcpClient->SendDataToNVGate(_T("SaveResults 0 1"));
		//if (iret == -1)  //SOCKET断开	
		//{
		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		//	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		//	Sleep(1);

		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		//	MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//	//恢复鼠标光标
		//	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//	ForcePostMessage(WM_FINISHCOUNT, 4, (LPARAM)m_pTcpClient);

		//	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		//	return;
		//}


		//iret = m_pTcpClient->SendDataToNVGate("GetAnalyzerState 3");
		//if (iret == -1)  //SOCKET断开	
		//{
		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		//	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		//	Sleep(1);

		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		//	MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//	//恢复鼠标光标
		//	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//	ForcePostMessage(WM_FINISHCOUNT, 4, (LPARAM)m_pTcpClient);
		//	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		//	return;
		//}


		//iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue 7 208 396 0"));
		//if (iret == -1)  //SOCKET断开	
		//{
		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		//	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		//	Sleep(1);

		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		//	MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//	//恢复鼠标光标
		//	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//	ForcePostMessage(WM_FINISHCOUNT, 4, (LPARAM)m_pTcpClient);

		//	////启用按钮
		//	//m_bBtnStates->b_ButtonStates = TRUE;
		//	//for (int i = 3; i <= 9; i++)
		//	//{
		//	//	m_bBtnStates->indexButton = i;
		//	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
		//	//	Sleep(1);
		//	//}

		//	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		//	return;
		//}

		//iret = m_pTcpClient->SendDataToNVGate(_T("SaveProject"));
		//if (iret == -1)  //SOCKET断开	
		//{
		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		//	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		//	Sleep(1);

		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		//	MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//	//恢复鼠标光标
		//	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//	ForcePostMessage(WM_FINISHCOUNT, 4, (LPARAM)m_pTcpClient);

		//	////启用按钮
		//	//m_bBtnStates->b_ButtonStates = TRUE;
		//	//for (int i = 3; i <= 9; i++)
		//	//{
		//	//	m_bBtnStates->indexButton = i;
		//	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
		//	//	Sleep(1);
		//	//}

		//	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		//	return;
		//}
	}

	//销毁线程
	//恢复鼠标光标
	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
	ci_CountInfo->successFlag = 1;
	ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

	str_Temp.Format(_T("Stop and saving the measurment in System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(100);

	////启用按钮
	//m_bBtnStates->b_ButtonStates = TRUE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// 关闭文件发送线程
// 不发送任何网络消息给接收端
void CStopRecord::CloseStopRecordThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // 等待线程结束
}

void CStopRecord::ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	while (!m_bTerminated)
	{
		if (PostMessage(m_hHideWnd, Msg, wParam, lParam))
			return;
		if (GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
			return;
		Sleep(1000);
	}
}

void CStopRecord::SetHideWndHandle(HWND hHideWnd)
{
	m_hHideWnd = hHideWnd;
}

void CStopRecord::SetSocketHandle(SOCKET hSocket)
{

	m_hSocket = hSocket;
}

void CStopRecord::SetIPAddress(CString *ipAddress)
{
	m_pIpAddress = ipAddress;
}

void CStopRecord::SetNumOfSystem(int numofSystem)
{
	m_numofSystem = numofSystem;
}

void CStopRecord::SetHwnd(HWND m_Hwnd)
{
	m_MainHwnd = m_Hwnd;
}



//////////////////////////////////////////////////////////////////////
// CGetAnalyzerStatusThread
//////////////////////////////////////////////////////////////////////


CGetAnalyzerStatus::CGetAnalyzerStatus(CTcpClient *pTcpClient, int nPriority) : CThread(nPriority)
{
	m_pTcpClient = pTcpClient;
	m_hHideWnd = NULL;
	m_hSocket = INVALID_SOCKET;

	//m_nCurrent = 0;
	m_numofSystem = 0;
	m_pIpAddress = NULL;

	m_sStateInfor = new STATEINFO();

}


CGetAnalyzerStatus::~CGetAnalyzerStatus()
{
	delete m_sStateInfor;
}

void CGetAnalyzerStatus::Execute(void)
{

	int iret = 0;
	CString str_Temp = _T("");
	CString str_Feedback = _T("");
	CString str_StopFlag = _T("");

	CString str_LastStatus = _T("");

	CString str_KeyName = _T("");
	str_KeyName.Format(_T("System%d"), m_numofSystem + 1);
	CString iniValue = _T("");
	char strINIInfo[64] = { "" };

	

	ASSERT(m_hHideWnd != NULL);

	//等待鼠标的光标
	m_pTcpClient->SetTerminateStatus(FALSE);

	//加载项目
	//for (int nsys = 1; nsys <= m_numofSystem; nsys++)
	//while (!m_bTerminated)
	{
		m_sStateInfor->indexSystem = m_numofSystem;

	StartPoint:
		while (m_bConnectionFlag && !m_bTerminated)
		{
			//端口号为3000
			m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
			m_pTcpClient->SetPort(3000);

			if (!m_pTcpClient->Connect(3))
			{
				SetConnectionFlag(TRUE);
				SetStartFlag(FALSE);
			}
			else
			{
				SetConnectionFlag(FALSE);
				SetStartFlag(TRUE);
			}

			Sleep(10);
		}

		//ASSERT(m_hSocket != INVALID_SOCKET);

		while (m_bStartFlag && !m_bTerminated)
		{
			iret = m_pTcpClient->SendDataToNVGate("GetAnalyzerState 3");
			if (iret == -1)  //SOCKET断开	
			{
				////ForcePostMessage(WM_GETSOFTWAREMSG, 6041, (LPARAM)m_pTcpClient);
				SetConnectionFlag(TRUE);
				SetStartFlag(FALSE);

				str_Temp.Format(_T("Remote NVGate #%d is closed!"), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);
				
				m_sStateInfor->stateString = _T("Unknown");
				m_sStateInfor->indexStates = 1;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
				Sleep(1);
				ForcePostMessage(WM_EXEGETTINGSTATUS, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
				Sleep(1);

				m_sStateInfor->stateString = _T("0.0 s");
				m_sStateInfor->indexStates = 2;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
				Sleep(1);

				m_sStateInfor->stateString = _T("0 °C");
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);

				goto StartPoint;
				return;
			}
			else if (iret == 1)   //反馈错误
			{
				m_sStateInfor->stateString = _T("Unknown");
				m_sStateInfor->indexStates = 1;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
				Sleep(1);
				ForcePostMessage(WM_EXEGETTINGSTATUS, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
			}
			else
			{
				m_pTcpClient->m_nCurrent = 10;
				m_pTcpClient->MakeString(m_pTcpClient->bDataFromNVGate, str_Feedback);
				m_sStateInfor->stateString = str_Feedback;
				m_sStateInfor->indexStates = 1;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
				Sleep(1);
				ForcePostMessage(WM_EXEGETTINGSTATUS, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
			}

			iret = m_pTcpClient->SendDataToNVGate("GetAnalyzerState 3 Count");
			if (iret == -1)  //SOCKET断开	
			{
				SetConnectionFlag(TRUE);
				SetStartFlag(FALSE);

				str_Temp.Format(_T("Remote NVGate #%d is closed!"), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);
				
				m_sStateInfor->stateString = _T("Unknown");
				m_sStateInfor->indexStates = 1;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
				Sleep(1);

				m_sStateInfor->stateString = _T("0.0 s");
				m_sStateInfor->indexStates = 2;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
				Sleep(1);

				m_sStateInfor->stateString = _T("0 °C");
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);

				goto StartPoint;
				return;
			}
			else if (iret == 1)   //反馈错误
			{
				m_sStateInfor->stateString = _T("0.0 s");
				m_sStateInfor->indexStates = 2;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
			}
			else
			{
				m_pTcpClient->m_nCurrent = 10;
				m_pTcpClient->MakeFloatA(m_pTcpClient->bDataFromNVGate, str_Feedback);
				m_sStateInfor->stateString.Format(_T("%s s"),str_Feedback);
				m_sStateInfor->indexStates = 2;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
			}

			iret = m_pTcpClient->SendDataToNVGate("GetFanInfo");
			if (iret == -1)  //SOCKET断开	
			{
				SetConnectionFlag(TRUE);
				SetStartFlag(FALSE);

				str_Temp.Format(_T("Remote NVGate #%d is closed!"), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);

				m_sStateInfor->stateString = _T("Unknown");
				m_sStateInfor->indexStates = 1;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
				Sleep(1);

				m_sStateInfor->stateString = _T("0.0 s");
				m_sStateInfor->indexStates = 2;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
				Sleep(1);

				m_sStateInfor->stateString = _T("0 °C");
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);

				goto StartPoint;
				return;
			}
			else if (iret == 1)   //反馈错误
			{
				m_sStateInfor->stateString = _T("0 °C");
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
			}
			else
			{
				m_pTcpClient->m_nCurrent = 18;
				m_pTcpClient->MakeLongInteger(m_pTcpClient->bDataFromNVGate, str_Feedback);
				m_sStateInfor->stateString.Format(_T("%s °C"), str_Feedback);
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
			}

			Sleep(100);

		}

	}
	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// 关闭文件发送线程
// 不发送任何网络消息给接收端
void CGetAnalyzerStatus::CloseGetAnalyzerStatusThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // 等待线程结束
}

void CGetAnalyzerStatus::ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	while (!m_bTerminated)
	{
		if (PostMessage(m_hHideWnd, Msg, wParam, lParam))
			return;
		if (GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
			return;
		Sleep(1000);
	}
}

void CGetAnalyzerStatus::SetHideWndHandle(HWND hHideWnd)
{
	m_hHideWnd = hHideWnd;
}

void CGetAnalyzerStatus::SetSocketHandle(SOCKET hSocket)
{

	m_hSocket = hSocket;
}

void CGetAnalyzerStatus::SetIPAddress(CString *ipAddress)
{
	m_pIpAddress = ipAddress;
}

void CGetAnalyzerStatus::SetNumOfSystem(int numofSystem)
{
	m_numofSystem = numofSystem;
}

void CGetAnalyzerStatus::SetHwnd(HWND m_Hwnd)
{
	m_MainHwnd = m_Hwnd;
}

void CGetAnalyzerStatus::SetStartFlag(BOOL startFlag)
{
	m_bStartFlag = startFlag;
}

void CGetAnalyzerStatus::SetConnectionFlag(BOOL connectionFlag)
{
	m_bConnectionFlag = connectionFlag;
}



//////////////////////////////////////////////////////////////////////
// CRunAutoZeroThread
//////////////////////////////////////////////////////////////////////


CRunAutoZero::CRunAutoZero(CTcpClient *pTcpClient, int nPriority) : CThread(nPriority)
{
	m_pTcpClient = pTcpClient;
	m_hHideWnd = NULL;
	m_MainHwnd = NULL;
	m_hSocket = INVALID_SOCKET;

	m_numofSystem = 0;
	m_pIpAddress = NULL;
	m_bBtnStates = new BTNSTATES();
	ci_CountInfo = new COUNTINFO();

}


CRunAutoZero::~CRunAutoZero()
{
	delete m_bBtnStates;
	delete ci_CountInfo;
}

void CRunAutoZero::Execute(void)
{

	int iret = 0;
	CString str_Temp = _T("");
	CString str_Feedback = _T("");
	CString str_StopFlag = _T("");

	ASSERT(m_hHideWnd != NULL);
	ci_CountInfo->iSource = 4;
	ci_CountInfo->successFlag = 0;

	//等待鼠标的光标
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);

	m_pTcpClient->SetTerminateStatus(FALSE);

	////禁用按钮
	//m_bBtnStates->b_ButtonStates = FALSE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	//连接到远程计算机
	str_Temp.Format(_T("Attempting to connect the remote NVGate of System #%d..."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//端口号为3000
	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
	m_pTcpClient->SetPort(3000);
	if (!m_pTcpClient->Connect(3))
	{
		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

		str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Connecting to the remote NVGate #%d failed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		////启用按钮
		//m_bBtnStates->b_ButtonStates = TRUE;
		//for (int i = 3; i <= 9; i++)
		//{
		//	m_bBtnStates->indexButton = i;
		//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
		//	Sleep(1);
		//}
		//断开SOCKET，销毁线程

		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}
	else
	{
		str_Temp.Format(_T("Connecting the remote NVGate of System #%d succeed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);
	}

	SetSocketHandle(m_pTcpClient->m_hSocket);

	ASSERT(m_hSocket != INVALID_SOCKET);

	//Start run Auto-Zero
	str_Temp.Format(_T("Starting runing auto-zero in System #%d..."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);


	iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue 1 338 77 1"));
	if (iret == -1)  //SOCKET断开	
	{
		str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}

	iret = m_pTcpClient->SendDataToNVGate(_T("SetSilent 1"));
	if (iret == -1)  //SOCKET断开	
	{
		str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}

	//保存当前的WORKBOOK
	str_Temp.Format(_T("SaveWorkbookModel SuperModel_%d"), m_numofSystem + 1);
	iret = m_pTcpClient->SendDataToNVGate(str_Temp);
	if (iret == -1)  //SOCKET断开	
	{
		str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//恢复鼠标光标
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}


	//销毁线程
	//恢复鼠标光标
	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
	ci_CountInfo->successFlag = 1;
	ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

	str_Temp.Format(_T("Starting the recording in System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	////启用按钮
	//m_bBtnStates->b_ButtonStates = TRUE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// 关闭文件发送线程
// 不发送任何网络消息给接收端
void CRunAutoZero::CloseRunAutoZeroThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // 等待线程结束
}

void CRunAutoZero::ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	while (!m_bTerminated)
	{
		if (PostMessage(m_hHideWnd, Msg, wParam, lParam))
			return;
		if (GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
			return;
		Sleep(1000);
	}
}

void CRunAutoZero::SetHideWndHandle(HWND hHideWnd)
{
	m_hHideWnd = hHideWnd;
}

void CRunAutoZero::SetSocketHandle(SOCKET hSocket)
{

	m_hSocket = hSocket;
}

void CRunAutoZero::SetIPAddress(CString *ipAddress)
{
	m_pIpAddress = ipAddress;
}

void CRunAutoZero::SetNumOfSystem(int numofSystem)
{
	m_numofSystem = numofSystem;
}

void CRunAutoZero::SetHwnd(HWND m_Hwnd)
{
	m_MainHwnd = m_Hwnd;
}


//////////////////////////////////////////////////////////////////////
// CCloseNVGateThread
//////////////////////////////////////////////////////////////////////

CCloseNVGate::CCloseNVGate(CTcpClient *pTcpClient, int nPriority) : CThread(nPriority)
{
	m_pTcpClient = pTcpClient;
	m_hHideWnd = NULL;
	m_hSocket = INVALID_SOCKET;

	m_MainHwnd = NULL;

	m_nWaitTimeOut = DFT_WAITTIMEOUT;


}


CCloseNVGate::~CCloseNVGate()
{

}

void CCloseNVGate::Execute(void)
{
	int iret = 0;
	CString str_Temp = _T("");
	BOOL b_Temp = TRUE;

	CString str_KeyName = _T("");
	str_KeyName.Format(_T("System%d"), m_numofSystem + 1);

	ASSERT(m_hHideWnd != NULL);

	//连接到远程计算机
	str_Temp.Format(_T("Attempting to connect the controller PC of System #%d..."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//端口号为8000
	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
	m_pTcpClient->SetPort(8000);
	if (!m_pTcpClient->Connect(0))
	{
		//断开SOCKET，销毁线程
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}

	SetSocketHandle(m_pTcpClient->m_hSocket);
	ASSERT(m_hSocket != INVALID_SOCKET);

	//远程关闭NVGate
	SendRequestCloseNVGateNetMsg(m_numofSystem);

	//发送成功后，断开SOCKET，销毁线程
	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// 关闭文件发送线程
// 不发送任何网络消息给接收端
void CCloseNVGate::CloseNVGateThreadMute(void)
{
	WaitForSingleObject(m_hThread, INFINITE); // 等待线程结束
}

void CCloseNVGate::ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	while (!m_bTerminated)
	{
		if (PostMessage(m_hHideWnd, Msg, wParam, lParam))
			return;
		if (GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
			return;
		Sleep(1000);
	}
}

void CCloseNVGate::SetHideWndHandle(HWND hHideWnd)
{
	m_hHideWnd = hHideWnd;
}

void CCloseNVGate::SetSocketHandle(SOCKET hSocket)
{
	m_hSocket = hSocket;
}



void CCloseNVGate::SetIPAddress(CString *ipAddress)
{
	m_pIpAddress = ipAddress;
}

void CCloseNVGate::SetNumOfSystem(int numofSystem)
{
	m_numofSystem = numofSystem;
}

void CCloseNVGate::SetHwnd(HWND m_Hwnd)
{
	m_MainHwnd = m_Hwnd;
}


// 发送请求发送文件网络消息
BOOL CCloseNVGate::SendRequestCloseNVGateNetMsg(int nSys)
{

	char Buf[19 + MAX_PATH];

	strncpy(&(Buf[6]), "-48", 3);

	return m_pTcpClient->SendNetMsg(Buf, 9, 0);
}


//////////////////////////////////////////////////////////////////////
// CGetFanStatusThread
//////////////////////////////////////////////////////////////////////


CFanStatus::CFanStatus(CTcpClient *pTcpClient, int nPriority) : CThread(nPriority)
{
	m_pTcpClient = pTcpClient;
	m_hHideWnd = NULL;
	m_hSocket = INVALID_SOCKET;

	//m_nCurrent = 0;
	m_numofSystem = 0;
	m_pIpAddress = NULL;

	m_sStateInfor = new STATEINFO();

}


CFanStatus::~CFanStatus()
{
	delete m_sStateInfor;
}

void CFanStatus::Execute(void)
{

	int iret = 0;
	CString str_Temp = _T("");
	CString str_Feedback = _T("");
	CString str_StopFlag = _T("");

	CString str_LastStatus = _T("");

	ASSERT(m_hHideWnd != NULL);

	//等待鼠标的光标
	m_pTcpClient->SetTerminateStatus(FALSE);

	//加载项目
	//for (int nsys = 1; nsys <= m_numofSystem; nsys++)
	//while (!m_bTerminated)
	{
		m_sStateInfor->indexSystem = m_numofSystem;

	StartPoint:
		while (m_bConnectionFlag && !m_bTerminated)
		{
			//端口号为3000
			m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
			m_pTcpClient->SetPort(3000);
			if (!m_pTcpClient->Connect(3))
			{
				SetConnectionFlag(TRUE);
				SetStartFlag(FALSE);
			}
			else
			{
				SetConnectionFlag(FALSE);
				SetStartFlag(TRUE);
			}
			//Sleep(1000);					
		}

		//ASSERT(m_hSocket != INVALID_SOCKET);

		while (m_bStartFlag && !m_bTerminated)
		{
			iret = m_pTcpClient->SendDataToNVGate("GetFanInfo");
			if (iret == -1)  //SOCKET断开	
			{
				SetConnectionFlag(TRUE);
				SetStartFlag(FALSE);

				str_Temp.Format(_T("Remote NVGate #%d is closed!"), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);

				str_Temp.Format(_T("Remote NVGate #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
				MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);


				//m_sStateInfor->stateString = _T("Unknown");
				//m_sStateInfor->indexStates = 1;
				//ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
				//Sleep(1);

				//m_sStateInfor->stateString = _T("0.0 s");
				//m_sStateInfor->indexStates = 2;
				//ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
				//Sleep(1);

				m_sStateInfor->stateString = _T("0 °C");
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);

				goto StartPoint;
				return;
			}
			else if (iret == 1)   //反馈错误
			{
				m_sStateInfor->stateString = _T("0 °C");
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
			}
			else
			{
				m_pTcpClient->m_nCurrent = 18;
				m_pTcpClient->MakeLongInteger(m_pTcpClient->bDataFromNVGate, str_Feedback);
				m_sStateInfor->stateString.Format(_T("%s °C"), str_Feedback);
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
			}

			Sleep(50000);

		}

	}
	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// 关闭文件发送线程
// 不发送任何网络消息给接收端
void CFanStatus::CloseGetFanStatusThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // 等待线程结束
}

void CFanStatus::ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	while (!m_bTerminated)
	{
		if (PostMessage(m_hHideWnd, Msg, wParam, lParam))
			return;
		if (GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
			return;
		Sleep(1000);
	}
}

void CFanStatus::SetHideWndHandle(HWND hHideWnd)
{
	m_hHideWnd = hHideWnd;
}

void CFanStatus::SetSocketHandle(SOCKET hSocket)
{

	m_hSocket = hSocket;
}

void CFanStatus::SetIPAddress(CString *ipAddress)
{
	m_pIpAddress = ipAddress;
}

void CFanStatus::SetNumOfSystem(int numofSystem)
{
	m_numofSystem = numofSystem;
}

void CFanStatus::SetHwnd(HWND m_Hwnd)
{
	m_MainHwnd = m_Hwnd;
}

void CFanStatus::SetStartFlag(BOOL startFlag)
{
	m_bStartFlag = startFlag;
}

void CFanStatus::SetConnectionFlag(BOOL connectionFlag)
{
	m_bConnectionFlag = connectionFlag;
}

