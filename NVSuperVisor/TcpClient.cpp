// TcpClient
// ���Է���������Ϣ�����Է����ļ���֧�ֶϵ�����
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


	// �¼�ָ��
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

	StartupSocket(); // ��ʼ��windows socket
	RegisterClientSocketHideWndClass(); // �������ش�����
}

CTcpClient::~CTcpClient(void)
{
	m_nCurrent = 0;
	msgLen = 0;
	bDataFromNVGate = NULL;
	m_bGetResponseFinish = FALSE;
	m_bTerminateStatus = FALSE;


	//delete m_sStateInfor;

	Disconnect(); // �Ͽ�socket����
	CleanupSocket();
	UnregisterClass(STR_CLIENTSOCKETHIDEWNDCLASSNAME, AfxGetInstanceHandle());
	DiscardThread();  //�����ļ����͵��߳�
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


//  ��ʼ��windows socket
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

// ���windows socket
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

	// Э����
	pSockAddr->sin_family = PF_INET;
    
	// �󶨵�ַ
	if(strcmp(szBindIpAddr, "") == 0)
		AIp = INADDR_NONE;
	else
		AIp = inet_addr(szBindIpAddr); // ת�����ʮ����ip��ַ

    if(AIp == INADDR_NONE)
		pSockAddr->sin_addr.S_un.S_addr = INADDR_ANY; // �󶨵���������ӿ�
    else
      pSockAddr->sin_addr.S_un.S_addr = AIp;

    pSockAddr->sin_port = htons(wPort); // �󶨶˿�
}

// ���ش�����Ϣ������
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
	case WM_FINISHCOUNT:   //��ȡ���߳��������
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
	case WM_SOCKETCLOSE:// socket���ر�
		pTcpClient->HandleSocketCloseMsg();
		break;// socket���ر�
	case WM_ONLYCLOSESOCKET:// �����رշ����ļ�socket
		pTcpClient->HandleSocketOnlyCloseMsg();
		break;// socket���ر�
	case WM_DISCARDTHREAD:  //�ر��߳�
		pTcpClient->DiscardThread();
		break;
	case WM_SOCKETSENDERR: // socket���ͳ���
		pTcpClient->HandleSocketSendErrMsg();
		break;
	case WM_SOCKETRECVERR: // socket���ճ���
		pTcpClient->HandleSocketRecvErrMsg();
		break;
	case WM_RECVERACCEPTFILE: // ���շ������ļ�
		pTcpClient->HandleRecverAcceptFileMsg((DWORD)wParam);
		break;
	case WM_RECVERREFUSEFILE: // ���շ��ܾ��ļ�
		pTcpClient->HandleRecverRefuseFileMsg();
		break;
	case WM_RECVERSUCC: // ���շ������ļ��ɹ�
		pTcpClient->HandleRecverSuccMsg();
		break;
	case WM_RECVERFAIL: // ���շ�ʧ��
		pTcpClient->HandleRecverFailMsg();
		break;
	case WM_RECVERCANCEL: // ���շ�ȡ��
		pTcpClient->HandleRecverCancelMsg();
		break;
	case WM_SENDERFAIL: // ���շ�ʧ��
		pTcpClient->HandleSenderFailMsg();
		break;
	case WM_SENDFILEPROGRESS: // �ļ����ͽ���
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

// ������������
BOOL CTcpClient::AllocateWindow(void)
{
	if(m_hHideWnd == NULL)
	{
		m_hHideWnd = CreateWindow(STR_CLIENTSOCKETHIDEWNDCLASSNAME, NULL, WS_POPUP,  
			CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, AfxGetInstanceHandle(), this);
	}

	return (m_hHideWnd != NULL);
}

// �ͷ���������
void CTcpClient::DeallocateWindow(void)
{
	if(m_hHideWnd != NULL)
	{
		DestroyWindow(m_hHideWnd);
		m_hHideWnd = NULL;
	}
}

// ���ӷ���ˣ�SOURCE = 1 FROM NVGATE
BOOL CTcpClient::Connect(int source)
{
	u_long			ulTmp;
	SOCKADDR_IN		SockAddr;
	
	Disconnect(); // �Ͽ�����

	//if(!AllocateWindow())
	//	return FALSE;
	
	// ����socket
	m_hSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
	if(m_hSocket == INVALID_SOCKET)
		goto ErrEntry;
	
	// ����socketΪ������ʽ
	ulTmp = 0;
	if(ioctlsocket(m_hSocket, FIONBIO, &ulTmp) != 0)
		goto ErrEntry;

	// ����socket���ͳ�ʱ
	if(setsockopt(m_hSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&m_nSendTimeOut, 
		sizeof(m_nSendTimeOut)) != 0)
		goto ErrEntry;

	// //����socket���ܳ�ʱ
	//if (source != 0)
	//{
	//	if (setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&m_nSendTimeOut,
	//			sizeof(m_nSendTimeOut)) != 0)
	//			goto ErrEntry;
	//}
		
	// ���ӷ����
	InitSockAddr(&SockAddr, m_szServerIpAddr, m_wPort);
	if(connect(m_hSocket, (sockaddr *)&SockAddr, sizeof(SOCKADDR_IN)) != 0)
		goto ErrEntry;

	//��ʾ������Ϣ
	m_OnSocketConnect(m_pNotifyObj, m_hSocket);

	SetTerminateStatus(FALSE);

	// ���������߳�
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

// �Ͽ�����
void CTcpClient::Disconnect(void)
{

	// �ر�socket����
	if(m_hSocket != INVALID_SOCKET)
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}
	
	// ���ٽ����߳�
	if(m_pClientSocketRecvThread != NULL)
	{
		m_pClientSocketRecvThread->Terminate();
		WaitForSingleObject(m_pClientSocketRecvThread->GetThreadHandle(), INFINITE);
		delete m_pClientSocketRecvThread;
		m_pClientSocketRecvThread = NULL;
	}

}

//������Ϣ��ȡ���ļ������߳�
void CTcpClient::DiscardThread(void)
{

	HandleSetCursor(0);

	// ���������Workbook�߳�
	if (m_pGenerateModelThread != NULL)
	{
		m_pGenerateModelThread->CloseGenerateWorkbookThreadMute();
		delete m_pGenerateModelThread;
		m_pGenerateModelThread = NULL;
	}
	
	// ����ļ������߳�
	if (m_pSendFileThread != NULL)
	{
		m_pSendFileThread->CloseSendFileThreadMute();
		delete m_pSendFileThread;
		m_pSendFileThread = NULL;
	}

	// ����ļ������߳�
	if (m_pOperateRemoteNVGate != NULL)
	{
		m_pOperateRemoteNVGate->CloseOperateNVGateThreadMute();
		delete m_pOperateRemoteNVGate;
		m_pOperateRemoteNVGate = NULL;
	}

	// ����ļ������߳�
	if (m_pOpenRemoteNVGate != NULL)
	{
		m_pOpenRemoteNVGate->CloseOpenNVGateThreadMute();
		delete m_pOpenRemoteNVGate;
		m_pOpenRemoteNVGate = NULL;
	}

	// ���Alarm Recorder�߳�
	if (m_pAlarmSystem != NULL)
	{
		m_pAlarmSystem->CloseAlarmSystemThreadMute();
		delete m_pAlarmSystem;
		m_pAlarmSystem = NULL;
	}

	// ���Start Monitor�߳�
	if (m_pStartMonitor != NULL)
	{
		m_pStartMonitor->CloseStartMonitorThreadMute();
		delete m_pStartMonitor;
		m_pStartMonitor = NULL;
	}

	// ���Start Monitor�߳�
	if (m_pStartRecord != NULL)
	{
		m_pStartRecord->CloseStartRecordThreadMute();
		delete m_pStartRecord;
		m_pStartRecord = NULL;
	}

	// ���Start Monitor�߳�
	if (m_pStopRecord != NULL)
	{
		m_pStopRecord->CloseStopRecordThreadMute();
		delete m_pStopRecord;
		m_pStopRecord = NULL;
	}

	// ���Start Monitor�߳�
	if (m_pGetAnalyzerStatus != NULL)
	{
		m_pGetAnalyzerStatus->CloseGetAnalyzerStatusThreadMute();
		delete m_pGetAnalyzerStatus;
		m_pGetAnalyzerStatus = NULL;
	}

	// ���Start Monitor�߳�
	if (m_pRunAutoZero != NULL)
	{
		m_pRunAutoZero->CloseRunAutoZeroThreadMute();
		delete m_pRunAutoZero;
		m_pRunAutoZero = NULL;
	}

	// ���Start Monitor�߳�
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

	DeallocateWindow(); // ���ٴ���, ������Ӧ������Ϣ
}


// ���ص�ǰsocket����״̬
BOOL CTcpClient::IsConnect(void)
{
	return (m_hSocket != INVALID_SOCKET);
}

// �ж��Ƿ��ڷ���
BOOL CTcpClient::IsSending(void)
{
	return (m_pSendFileThread != NULL);
}

BOOL CTcpClient::SendFile(CString *ipAddress, int numofSystem, CString cfgName, int nPlace, HWND mHwnd)
{
	
	if(IsSending())
		return FALSE; // ���ڷ����ļ�

	//�������ش���
	if (!AllocateWindow())
		return FALSE;

	// �����ļ�������������	
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
	m_pSendFileThread->Resume(); // ��ʼ����
	
	return TRUE;
}

BOOL CTcpClient::GetGenerateWorkbook(int *numofChannel, CString *ipAddress, int numofSystem, int totalChannels, CString configName, HWND mHwnd)
{

	//�������ش���
	if (!AllocateWindow())
		return FALSE;

	////���ӵ�����NVGate	
	//SetServerIpAddr((char *)(LPCTSTR)_T("127.0.0.1"));
	//SetPort(3000);
	//if (!Connect(3)) 
	//	return FALSE;
	//

	// ����������Model������
	m_pGenerateModelThread = new CGenerateModel(this, m_nSendFileThreadPriority);
	m_pGenerateModelThread->SetHwnd(mHwnd);
	m_pGenerateModelThread->SetHideWndHandle(m_hHideWnd);
	m_pGenerateModelThread->SetSocketHandle(m_hSocket);
	m_pGenerateModelThread->SetConfigName(configName);
	m_pGenerateModelThread->PrepareGetLocalInformation(numofChannel, ipAddress, numofSystem, totalChannels);
	m_totalChannels = totalChannels;
	m_pGenerateModelThread->Resume(); // ��ʼ����

	return TRUE;
}

BOOL CTcpClient::OperateRemoteNVGate(CString *ipAddress, int numofSystem, CString workbookName, HWND mHwnd)
{
	//�������ش���
	if (!AllocateWindow())
		return FALSE;

	// ����������Model������
	m_pOperateRemoteNVGate = new COperateRemoteNVGate(this, m_nSendFileThreadPriority);
	m_pOperateRemoteNVGate->SetHideWndHandle(m_hHideWnd);
	m_pOperateRemoteNVGate->SetHwnd(mHwnd);
	m_pOperateRemoteNVGate->SetWorkbookName(workbookName);
	//m_pOperateRemoteNVGate->SetSocketHandle(m_hSocket);
	m_pOperateRemoteNVGate->SetIPAddress(ipAddress);
	m_pOperateRemoteNVGate->SetNumOfSystem (numofSystem);
	m_pOperateRemoteNVGate->Resume(); // ��ʼ����

	return TRUE;
}

BOOL CTcpClient::OpenRemoteNVGate(CString *ipAddress, int numofSystem, HWND mHwnd, CString *HardIpAddr, CString *HardType)
{
	//�������ش���
	if (!AllocateWindow())
		return FALSE;
	
	// ����������Model������
	m_pOpenRemoteNVGate = new COpenRemoteNVGate(this, m_nSendFileThreadPriority);
	m_pOpenRemoteNVGate->SetHideWndHandle(m_hHideWnd);
	m_pOpenRemoteNVGate->SetHwnd(mHwnd);
	m_pOpenRemoteNVGate->SetHardIpAddr(HardIpAddr);
	m_pOpenRemoteNVGate->SetHardType(HardType);
	//m_pOperateRemoteNVGate->SetSocketHandle(m_hSocket);
	m_pOpenRemoteNVGate->SetIPAddress(ipAddress);
	m_pOpenRemoteNVGate->SetNumOfSystem(numofSystem);
	m_pOpenRemoteNVGate->Resume(); // ��ʼ����

	return TRUE;
}

BOOL CTcpClient::AlarmRecorder(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//�������ش���
	if (!AllocateWindow())
		return FALSE;

	// ����������Model������
	m_pAlarmSystem = new CAlarmSystem(this, m_nSendFileThreadPriority);
	m_pAlarmSystem->SetHideWndHandle(m_hHideWnd);
	m_pAlarmSystem->SetHwnd(mHwnd);
	//m_pOperateRemoteNVGate->SetSocketHandle(m_hSocket);
	m_pAlarmSystem->SetIPAddress(ipAddress);
	m_pAlarmSystem->SetNumOfSystem(numofSystem);
	m_pAlarmSystem->Resume(); // ��ʼ����

	return TRUE;
}

BOOL CTcpClient::StartMonitor(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//�������ش���
	if (!AllocateWindow())
		return FALSE;

	// ����������Model������
	m_pStartMonitor = new CStartMonitor(this, m_nSendFileThreadPriority);
	m_pStartMonitor->SetHideWndHandle(m_hHideWnd);
	m_pStartMonitor->SetHwnd(mHwnd);
	//m_pOperateRemoteNVGate->SetSocketHandle(m_hSocket);
	m_pStartMonitor->SetIPAddress(ipAddress);
	m_pStartMonitor->SetNumOfSystem(numofSystem);
	m_pStartMonitor->Resume(); // ��ʼ����

	return TRUE;
}

BOOL CTcpClient::StartRecord(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//�������ش���
	if (!AllocateWindow())
		return FALSE;

	// ����������Model������
	m_pStartRecord = new CStartRecord(this, m_nSendFileThreadPriority);
	m_pStartRecord->SetHideWndHandle(m_hHideWnd);
	m_pStartRecord->SetHwnd(mHwnd);
	//m_pOperateRemoteNVGate->SetSocketHandle(m_hSocket);
	m_pStartRecord->SetIPAddress(ipAddress);
	m_pStartRecord->SetNumOfSystem(numofSystem);
	m_pStartRecord->Resume(); // ��ʼ����

	return TRUE;
}

BOOL CTcpClient::StopRecord(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//�������ش���
	if (!AllocateWindow())
		return FALSE;

	// ����������Model������
	m_pStopRecord = new CStopRecord(this, m_nSendFileThreadPriority);
	m_pStopRecord->SetHideWndHandle(m_hHideWnd);
	m_pStopRecord->SetHwnd(mHwnd);
	//m_pOperateRemoteNVGate->SetSocketHandle(m_hSocket);
	m_pStopRecord->SetIPAddress(ipAddress);
	//m_pStopRecord-
	m_pStopRecord->SetNumOfSystem(numofSystem);
	m_pStopRecord->Resume(); // ��ʼ����

	return TRUE;
}

BOOL CTcpClient::GetAnalyzerStatus(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//�������ش���
	if (!AllocateWindow())
		return FALSE;
	
	// ����������Model������
	m_pGetAnalyzerStatus = new CGetAnalyzerStatus(this, m_nSendFileThreadPriority);
	m_pGetAnalyzerStatus->SetHideWndHandle(m_hHideWnd);
	m_pGetAnalyzerStatus->SetHwnd(mHwnd);
	//m_pGetAnalyzerStatus->SetSocketHandle(m_hSocket);
	m_pGetAnalyzerStatus->SetConnectionFlag(TRUE);
	m_pGetAnalyzerStatus->SetStartFlag(TRUE);
	m_pGetAnalyzerStatus->SetIPAddress(ipAddress);
	//m_pStopRecord-
	m_pGetAnalyzerStatus->SetNumOfSystem(numofSystem);
	m_pGetAnalyzerStatus->Resume(); // ��ʼ����

	return TRUE;
}

BOOL CTcpClient::GetFanStatus(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//�������ش���
	if (!AllocateWindow())
		return FALSE;

	// ����������Model������
	m_pGetFanStatus = new CFanStatus(this, m_nSendFileThreadPriority);
	m_pGetFanStatus->SetHideWndHandle(m_hHideWnd);
	m_pGetFanStatus->SetHwnd(mHwnd);
	//m_pGetAnalyzerStatus->SetSocketHandle(m_hSocket);
	m_pGetFanStatus->SetConnectionFlag(TRUE);
	m_pGetFanStatus->SetStartFlag(TRUE);
	m_pGetFanStatus->SetIPAddress(ipAddress);
	//m_pStopRecord-
	m_pGetFanStatus->SetNumOfSystem(numofSystem);
	m_pGetFanStatus->Resume(); // ��ʼ����

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
	//�������ش���
	if (!AllocateWindow())
		return FALSE;

	// ����������Model������
	m_pRunAutoZero = new CRunAutoZero(this, m_nSendFileThreadPriority);
	m_pRunAutoZero->SetHideWndHandle(m_hHideWnd);
	m_pRunAutoZero->SetHwnd(mHwnd);
	m_pRunAutoZero->SetIPAddress(ipAddress);
	//m_pStopRecord-
	m_pRunAutoZero->SetNumOfSystem(numofSystem);
	m_pRunAutoZero->Resume(); // ��ʼ����

	return TRUE;
}

BOOL CTcpClient::CloseNVGate(CString *ipAddress, int numofSystem, HWND mHwnd)
{
	//�������ش���
	if (!AllocateWindow())
		return FALSE;

	// ����������Model������
	m_pCloseNVGate = new CCloseNVGate(this, m_nSendFileThreadPriority);
	m_pCloseNVGate->SetHideWndHandle(m_hHideWnd);
	m_pCloseNVGate->SetHwnd(mHwnd);
	m_pCloseNVGate->SetIPAddress(ipAddress);
	//m_pStopRecord-
	m_pCloseNVGate->SetNumOfSystem(numofSystem);
	m_pCloseNVGate->Resume(); // ��ʼ����

	return TRUE;
}

//int CTcpClient::SendDataToNVGateA(CString str_Command)
//{
//	return m_pGenerateModelThread->SendDataToNVGate(str_Command);	
//}

// ȡ�������ļ�
void CTcpClient::CancelSendFile(void)
{
	if(IsSending())
	{
		m_pSendFileThread->CloseSendFileThreadCancel();
		delete m_pSendFileThread;
		m_pSendFileThread = NULL;
	}
}

//���ù��
void CTcpClient::HandleSetCursor(int cursorType)
{
	if (m_OnSetCursorType != NULL)
	{
		m_OnSetCursorType(m_pNotifyObj, cursorType);
	}
}

//���ð�ť����
void CTcpClient::HandleSetButtonStatus(int buttonID, BOOL buttonStatus)
{
	if (m_OnSetButtonStatus != NULL)
	{
		m_OnSetButtonStatus(m_pNotifyObj, buttonID, buttonStatus);
	}
}

// ����������Ϣ
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

// ����socket���ͳ�������Ϣ
// socket���ͳ����ǿ�ƹر�socket����
void CTcpClient::HandleSocketSendErrMsg(void)
{
	if(IsSending() && m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_SOCKETSENDERR);

	if(IsConnect() && m_OnSocketClose != NULL)
		m_OnSocketClose(m_pNotifyObj, m_hSocket, SCR_SOCKETSENDERR);

	Disconnect();
	DiscardThread();
}

//// ����socket���ճ�������Ϣ
void CTcpClient::HandleSocketRecvErrMsg(void)
{
	if(IsSending() && m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_SOCKETRECVERR);

	if(IsConnect () && m_OnSocketClose != NULL)
		m_OnSocketClose(m_pNotifyObj, m_hSocket, SCR_SOCKETRECVERR);

	Disconnect();
	DiscardThread();
}

// ����socket�رմ�����Ϣ
void CTcpClient::HandleSocketCloseMsg()
{
	
	//if(m_OnSendFileFail != NULL)
	//	m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_SOCKETCLOSE);
		
	//m_bTerminateStatus = TRUE;
	
	//if(m_OnSocketClose != NULL)
	//	m_OnSocketClose(m_pNotifyObj, m_hSocket, SCR_PEERCLOSE);

	SetTerminateStatus(TRUE);

	Disconnect(); // �Ͽ�socket���ӣ��Ͽ����������߳�
	DiscardThread();//�رշ����ļ��߳�
}

// ����socket�رմ�����Ϣ
void CTcpClient::HandleSocketOnlyCloseMsg(void)
{

	if (IsConnect() && m_OnSocketClose != NULL)
		m_OnSocketClose(m_pNotifyObj, m_hSocket, SCR_PEERCLOSE);

	Disconnect(); // �Ͽ�socket���ӣ��Ͽ����������߳�
}


// ����������������Ϣ
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


// ������ն˽����ļ�������Ϣ
void CTcpClient::HandleRecverAcceptFileMsg(DWORD dwRecvedBytes)
{
	if(!IsSending())
		return;
	
	// ���������ļ��߳�
	m_pSendFileThread->TrigAcceptFile(dwRecvedBytes);
}

// ������ն˾ܾ��ļ�������Ϣ
void CTcpClient::HandleRecverRefuseFileMsg(void)
{
	if(!IsSending())
		return;

	m_pSendFileThread->CloseSendFileThreadMute(); // �ر��ļ������߳�
	
	if(m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_RECVERREFUSEFILE);

	delete m_pSendFileThread;
	m_pSendFileThread = NULL;
}

// ������շ������ļ��ɹ�������Ϣ
void CTcpClient::HandleRecverSuccMsg(void)
{
	if(!IsSending())
		return;
	
	// ���յ��ô�����Ϣʱ�ļ������Ѿ�������
	if(!m_pSendFileThread->IsFileAccepted())
		return;

	m_pSendFileThread->TrigRecvFileSucc();

	//WaitForSingleObject(m_pSendFileThread->GetThreadHandle(), INFINITE); // �ȴ��߳̽���

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


// ������շ�ʧ�ܴ�����Ϣ
void CTcpClient::HandleRecverFailMsg(void)
{
	if(!IsSending())
		return;

	m_pSendFileThread->CloseSendFileThreadMute(); // �ر��ļ������߳�

	if(m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_RECVERFAIL);

	delete m_pSendFileThread;
	m_pSendFileThread = NULL;
}

// ������շ�ȡ���ļ����մ�����Ϣ
void CTcpClient::HandleRecverCancelMsg(void)
{
	if(!IsSending())
		return;

	m_pSendFileThread->CloseSendFileThreadMute(); // �ر��ļ������߳�

	if(m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_RECVERCANCEL);

	delete m_pSendFileThread;
	m_pSendFileThread = NULL;
}

// �����ļ����ͽ��ȴ�����Ϣ
void CTcpClient::HandleSendFileProgressMsg(void)
{
	UWORD dwSentBytes; // �ѷ����ֽ���
	UWORD dwFileSize; // �ļ���С

	if(!IsSending())
		return;

	if(m_OnSendFileProgress != NULL)
	{
		dwSentBytes = m_pSendFileThread->GetSentBytes();
		dwFileSize = m_pSendFileThread->GetFileSize();
		m_OnSendFileProgress(m_pNotifyObj, dwSentBytes, dwFileSize);
	}
}

// �����ļ�����ʧ�ܴ�����Ϣ 
void CTcpClient::HandleSenderFailMsg(void)
{
	if(!IsSending())
		return;

	m_pSendFileThread->CloseSendFileThreadMute(); // �ر��ļ������߳�

	if(m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_SENDERFAIL);

	delete m_pSendFileThread;
	m_pSendFileThread = NULL;
}

// ����ȴ���ʱ������Ϣ
void CTcpClient::HandleWaitTimeOutMsg(void)
{
	if(!IsSending())
		return;

	m_pSendFileThread->CloseSendFileThreadMute(); // �ر��ļ������߳�

	if(m_OnSendFileFail != NULL)
		m_OnSendFileFail(m_pNotifyObj, m_pSendFileThread->GetPathName(), SFFR_WAITTIMEOUT);

	delete m_pSendFileThread;
	m_pSendFileThread = NULL;
}

// ����ȴ���ʱ������Ϣ
void CTcpClient::HandleOpenNVGateSuccMsg(void)
{
	m_pOpenRemoteNVGate->OpenRemoteNVGateSucc();
}

// ����ȴ���ʱ������Ϣ
void CTcpClient::HandleOpenNVGateFailMsg(void)
{
	m_pOpenRemoteNVGate->OpenRemoteNVGateFail();
}

// ��������ر���Ϣ
void CTcpClient::HandleOpenNVGateMuteMsg(void)
{
	m_pOpenRemoteNVGate->OpenRemoteNVGateMute();
}

//���������״̬
void CTcpClient::HandleSetAnalyzerStatus(STATEINFO sStateInfo)
{
	if (m_OnSetAnalyzerStatus != NULL)
		m_OnSetAnalyzerStatus(m_pNotifyObj,sStateInfo.indexSystem,sStateInfo.stateString,sStateInfo.indexStates);
}

//���������״̬
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

//���������״̬
void CTcpClient::HandleSetFinishCount(int iSource, int successFlag)
{
	if (m_OnSetFinishCount != NULL)
		m_OnSetFinishCount(m_pNotifyObj, iSource, successFlag);
}

//�������ݸ�NVGate�����յ�������Ϣ
//���� -1��SOCKET�Ͽ���1�����ִ�����룻0����ȷִ��
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
		return -1;  //SOCKET�Ͽ�
	}

	while (!m_bGetResponseFinish)
	{
		if (!m_bTerminateStatus)
		{
			Sleep(1);
		}
		else
		{
			return -1;  //SOCKET�Ͽ�
		}

	}

	if (*bDataFromNVGate == NVSUC_TAG)
	{
		/*bData = *bDataFromNVGate;
		msgLength = msgLen;*/
		return 0;   //��ȷִ��
	}

	else return 1;    //���ִ���
}

//�����յ��ķ�����Ϣ
void CTcpClient::GetResString(char *Msg, int nMsgLen)
{
	bDataFromNVGate = Msg;
	msgLen = nMsgLen;
	SetFinishStatus(TRUE);
}

//��ȡ�ַ���
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
	//if (*a >= 1){//֮ǰΪֱ��ǿתΪint�Ĵ��룬���С����ʧ
	//	int b = (int)(*a);
	//	SettingValue.Format("%d", b);
	//}
	//else if (*a<1){//Ϊ��ά��ԭ���Ĵ��룬�˴�
	//	SettingValue.Format("%f", *a);
	//}

	SettingValue.Format("%.1f", *a);

	return;
}

void CTcpClient::MakeFloat(char* bDataFromNVGATE, CString& SettingValue)
{
	float *a = (float*)(bDataFromNVGATE + m_nCurrent);
	//if (*a >= 1){//֮ǰΪֱ��ǿתΪint�Ĵ��룬���С����ʧ
	//	int b = (int)(*a);
	//	SettingValue.Format("%d", b);
	//}
	//else if (*a<1){//Ϊ��ά��ԭ���Ĵ��룬�˴�
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

	//��ȡ��ǰ��Ӧ�ó���·������ȷ��INI�ļ��������ļ���
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

// �߳�ִ�к���
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

	//��ȡWORKBOOK FOLDER
	char str_Content[64] = { "0" };
	GetPrivateProfileString(_T("Parameters"), _T("WorkbookFolder"), _T(""), str_Content, 64, iniFileName);
	if (_T(str_Content) == "")
	{
		MessageBox(m_MainHwnd, _T("Getting workbook folder failed, please check Custom.ini file."), _T("NVSuperVisor"), MB_OK | MB_ICONERROR);
		//�Ͽ�SOCKET�������߳�
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}

	m_sWorkbookFolder = _T(str_Content);

	////�����Ϊ�ȴ�
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);

	////���ð�ť
	//m_bBtnStates->b_ButtonStates = FALSE;
	//for (int i = 1; i <= 3; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	//m_bBtnStates->indexButton = 1;
	//ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);

	//����Workbook�ļ�
	str_Temp.Format(_T("Transferring the workbook files to system #%d..."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	// �ȴ����շ������ļ�
	HANDLE hWaitAcceptFileEvents[3];
	hWaitAcceptFileEvents[0] = m_hCloseThreadMute;
	hWaitAcceptFileEvents[1] = m_hCloseThreadCancel;
	hWaitAcceptFileEvents[2] = m_hAcceptFile;

	// �ȴ����շ������ļ��ɹ�
	HANDLE hWaitRecvFileSuccEvents[3];
	hWaitRecvFileSuccEvents[0] = m_hCloseThreadMute;
	hWaitRecvFileSuccEvents[1] = m_hCloseThreadCancel;
	hWaitRecvFileSuccEvents[2] = m_hRecvFileSucc;

	//for (int nsys = 1; nsys <= m_numofSystem ; nsys++)
	{
		//���ӵ�Զ�̼����
		if (m_nSendPlace==0)
			str_Temp.Format(_T("Attempting to connect the SmartRoute PC of System #%d..."), m_numofSystem + 1);
		else
			str_Temp.Format(_T("Attempting to connect the Monitor PC #%d..."), m_numofSystem + 1);

		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);
		
		//�˿ں�Ϊ8000
		m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
		m_pTcpClient->SetPort(8000);
		if (!m_pTcpClient->Connect(0))
		{
			//�ָ������
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

			//�Ͽ�SOCKET�������߳�
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

		//���������ļ�
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
		

		//��·��������
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

			//�Ͽ�SOCKET�������߳�
			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}
					
		do
		{
			//������Ŀ¼��Ҳ����.��..
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


					//�Ͽ�SOCKET�������߳�
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
				}

				// �������ļ�
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


					//�Ͽ�SOCKET�������߳�
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
				}

				// �ȴ����շ������ļ�, һ�����շ������ļ�����ʼ�����ļ�����
				switch (WaitForMultipleObjects(3, hWaitAcceptFileEvents, FALSE, m_nWaitTimeOut))
				{
				case WAIT_OBJECT_0: // �ر��߳�
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


					//�Ͽ�SOCKET�������߳�
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;

					break;
				case WAIT_OBJECT_0 + 1: // ���Ͷ�ȡ�������̹߳ر�
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


					//�Ͽ�SOCKET�������߳�
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;

					break;
				case WAIT_OBJECT_0 + 2: // ���ն˽����ļ�
					Sleep(1000);
					break;
				case WAIT_TIMEOUT: // �ȴ���ʱ
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


					//�Ͽ�SOCKET�������߳�
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;

					break;
				default: // ����
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


					//�Ͽ�SOCKET�������߳�
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
					break;
				}

				////���ð�ť
				//m_bBtnStates->b_ButtonStates = FALSE;
				//for (int i = 1; i <= 3; i++)
				//{
				//	m_bBtnStates->indexButton = i;
				//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
				//	Sleep(1);
				//}

				// �����ļ�����
				SendFileData();

				// �ȴ����շ������ļ��ɹ�
				switch(WaitForMultipleObjects(3, hWaitRecvFileSuccEvents, FALSE, m_nWaitTimeOut))
				{
				case WAIT_OBJECT_0: // �ر��߳�
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


					//�Ͽ�SOCKET�������߳�
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
					break;
				case WAIT_OBJECT_0 + 1: // ���Ͷ�ȡ�������̹߳ر�
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


					//�Ͽ�SOCKET�������߳�
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
					break;
				case WAIT_OBJECT_0 + 2: // ���ն˽����ļ��ɹ�
					ResetSendFile();
					Sleep(1000);
					break;
				case WAIT_TIMEOUT: // �ȴ���ʱ
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


					//�Ͽ�SOCKET�������߳�
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
					break;
				default: // ����
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


					//�Ͽ�SOCKET�������߳�
					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
					break;
				}
			}
		} while (_findnext(handle, &findData) == 0);    // ����Ŀ¼�е���һ���ļ�
	}



	//���ͳɹ�
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
	//	//ɾ������Model
	//	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)_T("127.0.0.1"));
	//	m_pTcpClient->SetPort(3000);
	//	if (!m_pTcpClient->Connect(3))
	//	{
	//		//�ָ������
	//		//ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
	//		
	//		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

	//		str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
	//		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	//		Sleep(1);

	//		str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed, please check the network connection."), m_numofSystem + 1);
	//		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_OK | MB_ICONERROR);

	//		//�Ͽ�SOCKET�������߳�
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
	//	if (iret == -1)  //SOCKET�Ͽ�	
	//	{
	//		//�ָ������
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
	
	//�Ͽ����ӣ������߳�
	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);

}

// ׼�������ļ�
BOOL CSendFileThread::PrepareSendFile(char *szPathName)
{
	// �򿪴����͵��ļ�
	if (!m_File.Open(szPathName, CFile::modeRead | CFile::shareExclusive))
		return FALSE;
	
	// ��ȡ�ļ���С
	try
	{
		m_dwFileSize = m_File.GetLength();
	}
	catch(...)
	{
		m_File.Abort();
		return FALSE;
	}

	strcpy(m_szPathName, szPathName); // �����ļ���
	m_dwSentBytes = 0; // ��ʼ���ѷ����ֽ���

	return TRUE;
}

// ��λ�ļ������߳�
void CSendFileThread::ResetSendFile(void)
{
	// �ر��ļ�
	if(m_File.m_hFile != CFile::hFileNull)
	{
		m_File.Close();
	}
}

// ����ļ��Ƿ��Ѿ������ն˽���
BOOL CSendFileThread::IsFileAccepted(void)
{
	return (WaitForSingleObject(m_hAcceptFile, 0) == WAIT_OBJECT_0);
}

// �����ļ�����
void CSendFileThread::SendFileData(void)
{
	UWORD dwTickCount;
	char Buf[MAX_NETMSGPACKAGESIZE + 6]; // ���ͻ�����
	int nBytes;

	// ��Ϣͷ
	strncpy(&(Buf[6]), "-19", 3);

	// ���ͽ���
	ForcePostMessage(WM_SENDFILEPROGRESS, 0, (LPARAM)m_pTcpClient);

	while(!m_bTerminated && m_dwSentBytes < m_dwFileSize)
	{
		// ��ȡ�ļ����ݰ�
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

		// ����
		if(!m_pTcpClient->SendNetMsg(Buf, 3 + nBytes,0))
		{
			ResetSendFile();
			ForcePostMessage(WM_SOCKETSENDERR, 0, (LPARAM)m_pTcpClient);
			return;
		}

		m_dwSentBytes += nBytes;

		// ���ͽ���
		dwTickCount = GetTickCount();
		if(dwTickCount < m_dwLastProgressTick) m_dwLastProgressTick = 0;
		if(dwTickCount - m_dwLastProgressTick > m_dwProgressTimeInterval || m_dwSentBytes >= m_dwFileSize)
		{
			ForcePostMessage(WM_SENDFILEPROGRESS, 0, (LPARAM)m_pTcpClient); // �ļ����ս���
			m_dwLastProgressTick = dwTickCount;
		}
		else
			Sleep(0); // ǿ��windows�����߳�
	}
}

// �ر��ļ������߳�
// �������κ�������Ϣ�����ն�
void CSendFileThread::CloseSendFileThreadMute(void)
{
	Terminate();
	SetEvent(m_hCloseThreadMute); // �رշ����ļ��߳�
	WaitForSingleObject(m_hThread, INFINITE); // �ȴ��߳̽���
}

// �ر��ļ������߳�
// ����ȡ�������ļ�������Ϣ�����ն�
void CSendFileThread::CloseSendFileThreadCancel(void)
{
	Terminate();
	SetEvent(m_hCloseThreadCancel); // �رշ����ļ��߳�
	WaitForSingleObject(m_hThread, INFINITE); // �ȴ��߳̽���
}

// �����������ļ�������Ϣ
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

// ����ȡ�������ļ�������Ϣ
BOOL CSendFileThread::SendSenderCancelNetMsg(void)
{
	char Msg[9];

	strncpy(&(Msg[6]), "-21", 3);
	return m_pTcpClient->SendNetMsg(Msg, 3,0);
}

// ���ͷ����ļ�ʧ��������Ϣ
BOOL CSendFileThread::SendSenderFailNetMsg(void)
{
	char Msg[9];

	strncpy(&(Msg[6]), "-23", 3);
	return m_pTcpClient->SendNetMsg(Msg, 3,0);
}

// ���ն˽����ļ�
void CSendFileThread::TrigAcceptFile(DWORD dwRecvedBytes)
{
	// ������Ϣ�ĺϷ���
	if(WaitForSingleObject(m_hAcceptFile, 0) == WAIT_OBJECT_0)
		return;

	m_dwSentBytes = dwRecvedBytes;
	SetEvent(m_hAcceptFile);
}

// ���ն˽����ļ��ɹ�
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

// �߳�ִ����
void CClientSocketRecvThread::Execute(void)
{
	char Buf[SOCKET_RECVBUFSIZE]; // ���ջ�����
	int nErr;

	while(!m_bTerminated)
	{
		nErr = recv(m_hSocket, Buf, SOCKET_RECVBUFSIZE, 0);
		if(nErr == SOCKET_ERROR) // �д�����
		{
			nErr = WSAGetLastError();
			if(nErr == WSAECONNRESET) // TODO: ���æ������������ֵ������socket��λ
			{
				// socket���ӱ�peer�˹رգ����·��λ�ȴ��󣬳��ִ����͵Ĵ���ʱsocket���뱻�رա�
				//ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
				m_pTcpClient->SetTerminateStatus(TRUE);
				break;
			}
			else
			{				
				// ������socket���󣬽������ݳ���
				//ForcePostMessage(WM_SOCKETRECVERR, 0, (LPARAM)m_pTcpClient);
				m_pTcpClient->SetTerminateStatus(TRUE);
				return;
			}
		}
		else if(nErr == 0) // socket�������ر�
		{
			//ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			m_pTcpClient->SetTerminateStatus(TRUE);
			break;
		}
		else // ���յ�����
		{
			DoRecv(Buf, nErr); // ������յ�������
		}
	}; // while

}

// ��������Ϣ��װ����,Ȼ�������Ӧ�Ĵ�����
void CClientSocketRecvThread::DoRecv(char *Buf, int nBytes)
{
	int i;
	char szMsgLen[11];
	int nMsgLen; // ��Ϣ����
	int nTotalLen;

	// �������ݵ����ջ�����
	if (m_nRecvBufSize + nBytes > NETMSG_RECVBUFSIZE)
		m_nRecvBufSize = 0; // ��ս��ջ������е�����
	memcpy(&(m_RecvBuf[m_nRecvBufSize]), Buf, nBytes);
	m_nRecvBufSize += nBytes;


	if (m_source == 0)   // From File transfering
	{
		while (!m_bTerminated)
		{
			if (m_nRecvBufSize == 0) return; // ��������������,�˳�

			// �������е�һ���ֽڱ�������Ϣ��־
			if (m_RecvBuf[0] != MSG_TAG) // ��һ���ֽڲ�����Ϣ��־
			{
				for (i = 0; i < m_nRecvBufSize; i++)
				if (m_RecvBuf[i] == MSG_TAG) break;
				m_nRecvBufSize -= i;
				memcpy(m_RecvBuf, &(m_RecvBuf[i]), m_nRecvBufSize);
			}

			// ��ȡ��Ϣ�����ֽ���
			if (m_nRecvBufSize < 6) return; // ��Ϣ����δ��������,�˳�
			strncpy(szMsgLen, &(m_RecvBuf[1]), 5);
			szMsgLen[5] = 0;
			nMsgLen = atoi(szMsgLen);
			nTotalLen = 1 + 5 + nMsgLen;

			// �ж���Ϣ�Ƿ���������
			if (nTotalLen > m_nRecvBufSize) return; // ��Ϣû�н�������
			HandleRecvedNetMsg(&(m_RecvBuf[6]), nMsgLen); // �����������Ϣ
			m_nRecvBufSize -= nTotalLen;
			memcpy(m_RecvBuf, &(m_RecvBuf[nTotalLen]), m_nRecvBufSize); // ����Ϣ��������ȥ������Ϣ
		};
	}
	else
	{
		while(!m_bTerminated)
		{
			if(m_nRecvBufSize == 0) return; // ��������������,�˳�

			// �������е�һ���ֽڱ�������Ϣ��־
			if (m_RecvBuf[0] != NVSUC_TAG) // ��һ���ֽڲ��ǽ��ܳɹ��ı�־
			{
				// ��ȡ��Ϣ�����ֽ���
				if (m_nRecvBufSize < 10) return; // ��Ϣ����δ��������,�˳�
				nMsgLen = 10;
				nTotalLen = 10;
				HandleRecvedNetMsg(&(m_RecvBuf[0]), nMsgLen); // �����������Ϣ
				m_nRecvBufSize -= nTotalLen;
				memcpy(m_RecvBuf, &(m_RecvBuf[nTotalLen]), m_nRecvBufSize); // ����Ϣ��������ȥ������Ϣ
			}
			else
			{
				// ��ȡ��Ϣ�����ֽ���
				if (m_nRecvBufSize < 10) return; // ��Ϣ����δ��������,�˳�

				strncpy(szMsgLen, &(m_RecvBuf[1]), 9);
				szMsgLen[10] = 0;
				nMsgLen = atoi(szMsgLen);
				nTotalLen = 10 + nMsgLen;

				// �ж���Ϣ�Ƿ���������
				if (nTotalLen > m_nRecvBufSize) return; // ��Ϣû�н�������
				HandleRecvedNetMsg(&(m_RecvBuf[0]), nTotalLen); // �����������Ϣ
				m_nRecvBufSize -= nTotalLen;
				memcpy(m_RecvBuf, &(m_RecvBuf[nTotalLen]), m_nRecvBufSize); // ����Ϣ��������ȥ������Ϣ
			}
		
		};
	}

}

// ������յ���������Ϣ
void CClientSocketRecvThread::HandleRecvedNetMsg(char *Msg, int nMsgLen)
{
	char szCmd[4];
	int nCmd;

	strncpy(szCmd, Msg, 3);
	szCmd[3] = 0;
	nCmd = atoi(szCmd);

	switch(nCmd)
	{
	case -17: // ���ն˽����ļ�
		{
			DWORD dwSentBytes;
			char szSentBytes[11];

			strncpy(szSentBytes, &(Msg[3]), 10);
			szSentBytes[10] = 0;
			dwSentBytes = atoi(szSentBytes);
			ForcePostMessage(WM_RECVERACCEPTFILE, dwSentBytes, (LPARAM)m_pTcpClient);
		}
		break;
	case -18: // ���ն˾ܾ������ļ�
		ForcePostMessage(WM_RECVERREFUSEFILE, 0, (LPARAM)m_pTcpClient);
		break;
	case -20: // ���ն˽����ļ��ɹ�
		ForcePostMessage(WM_RECVERSUCC, 0, (LPARAM)m_pTcpClient);
		break;
	case -22: // ���ն�ȡ�������ļ�
		ForcePostMessage(WM_RECVERCANCEL, 0, (LPARAM)m_pTcpClient);
		break;
	case -24: // ���ն˽����ļ�����
		ForcePostMessage(WM_RECVERFAIL, 0, (LPARAM)m_pTcpClient);
		break;
	case -25: // ��NVGATE�ɹ�
		ForcePostMessage(WM_OPENNVGATESUCC, 0, (LPARAM)m_pTcpClient);
		break;
	case -27: // ��NVGATE�ɹ�
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

	//��ȡ��ǰ��Ӧ�ó���·������ȷ��INI�ļ��������ļ���
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

	//�����Ϊ�ȴ�
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);
	m_pTcpClient->SetTerminateStatus(FALSE);

	//���ð�ť
	m_bBtnStates->b_ButtonStates = FALSE;
	for (int i = 1; i <= 4; i++)
	{
		m_bBtnStates->indexButton = i;
		ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
		Sleep(1);
	}

	//���ӵ�����NVGate
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Attempting to connect local NVGate..."), (LPARAM)m_pTcpClient);
	Sleep(1);

	//�˿ں�Ϊ3000
	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)_T("127.0.0.1"));
	m_pTcpClient->SetPort(3000);
	if (!m_pTcpClient->Connect(3))
	{
		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Connecting local NVGate failed."), (LPARAM)m_pTcpClient);
		Sleep(1);

		MessageBox(m_MainHwnd, _T("Connecting to the local NVGate failed, please open the remote NVGate or check the network connection."), 
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//���ð�ť
		m_bBtnStates->b_ButtonStates = TRUE;
		for (int i = 1; i <= 4; i++)
		{
			m_bBtnStates->indexButton = i;
			ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			Sleep(1);
		}
		//�Ͽ�SOCKET�������߳�
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

	//��ȡ��ǰ��Ŀ����
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting current project name of local NVGate..."), (LPARAM)m_pTcpClient);
	iret = m_pTcpClient->SendDataToNVGate(_T("GetCurrentProjectName"));
	if (iret == -1)  //SOCKET�Ͽ�	
	{
		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Local NVGate is closed."), (LPARAM)m_pTcpClient);
		Sleep(1);

		MessageBox(m_MainHwnd, _T("Local NVGate is closed, please open the remote NVGate or check the network connection."), 
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		
		//���ð�ť
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
		
		//���ð�ť
		m_bBtnStates->b_ButtonStates = TRUE;
		for (int i = 1; i <= 4; i++)
		{
			m_bBtnStates->indexButton = i;
			ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			Sleep(1);
		}
		//�Ͽ����ӣ������̣߳��ر����ش���
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
	if (iret == -1)  //SOCKET�Ͽ�	
	{
		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Local NVGate is closed."), (LPARAM)m_pTcpClient);
		Sleep(1);

		MessageBox(m_MainHwnd, _T("Local NVGate is closed, please open the remote NVGate or check the network connection."),
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		
		//���ð�ť
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
	if (iret == -1)  //SOCKET�Ͽ�	
	{
		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Local NVGate is closed."), (LPARAM)m_pTcpClient);
		Sleep(1);
		MessageBox(m_MainHwnd, _T("Local NVGate is closed, please open the remote NVGate or check the network connection."),
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		
		//���ð�ť
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

	//��ȡ����NVGate����Ϣ

	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting the configuration of local NVGate..."), (LPARAM)m_pTcpClient);
	Sleep(1);
	
	iret = GetLocalNVGateData();
	if (iret == -1)  //SOCKET�Ͽ�	
	{
		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Local NVGate is closed."), (LPARAM)m_pTcpClient);
		Sleep(1);

		MessageBox(m_MainHwnd, _T("Local NVGate is closed, please open the remote NVGate or check the network connection."),
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		
		//���ð�ť
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
	if (iret == -1)  //SOCKET�Ͽ�	
	{
		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Local NVGate is closed."), (LPARAM)m_pTcpClient);
		Sleep(1);
		MessageBox(m_MainHwnd, _T("Local NVGate is closed, please open the remote NVGate or check the network connection."),
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		
		//���ð�ť
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
	
	//�Ͽ����ӣ������̣߳��ر����ش���
	//���ð�ť
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

// �ر�������Workbook�����߳�
void CGenerateModel::CloseGenerateWorkbookThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // �ȴ��߳̽���
}

//��ȡ����NVGate��WORKBOOK������Ϣ
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

	//��ȡInput����Ϣ��������Label��Input Type, Transducer, Physical Qty
	//Sensitivity, Range Peak, Polarity, Offset, Coupling, Sampling Rate
	//Gauge Type, Bridge Resistance, Bridge Gain, Bridge Offset, Input Filter
	//Auto Range, AutoZero

	//ǰ��32��ͨ���Ĳ����ͺ���ͨ����ͬ���ֿ���ȡ
	CString tempStr = _T("");

	int numberofTO = 0;
	BOOL bFirstFlag = TRUE;//����Tracked Order�������Ե�һ�ζ�����0

	CString isActive = _T("0");   //����ͨ���Ƿ񼤻�

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

	//����ͨ����Ϣ��ȡ
	for (int i = 1; i <= m_totalChannel; i++)
	{
		ForcePostMessage(WM_STARTGETTINGINPUT, i, (LPARAM)m_pTcpClient);

		tempStr.Format(_T("frontEnd input%d active"), i);
		iret = GetSettingValue(tempStr, isActive);
		//SOCKET�Ͽ������߷��ش������
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
			//����ͨ������
			::WritePrivateProfileStringA(sectionName, _T("Active"), "ON", iniFileName);
			////ת��ͨ������
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

			//��ȡEXPANDXPOD����Ϣ
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

	//Recorder �ж�ת���Ƿ����ӽ������ܹ�228��ͨ��
	//ǰ��38��ͨ���Ĳ�����һ��
	//����ж���2��ת�٣��Ժ������

	for (int i = 1; i <= DFT_MAXRECCHANNELCOUNT; i++)
	{
		ForcePostMessage(WM_STARTGETTINGREC, i, (LPARAM)m_pTcpClient);

		//��ȡ��¼��ͨ����Active
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

		//ͨ������������ȨȨ�ޣ��˳�REC��ѭ��
		if (iret == 1)
		{
			break;
		}

		if (str_Setting == _T("1"))
		{
			//��ȡREC�е�source���õ���Ӧ��ͨ����
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
		} //��ǰRecorder Track�Ѿ�Active		
	}

	//��ȡFFT�Ĳ������K����ص���Ϣд�뵽��Ӧ��INPUT��
	//FFT��ͨ����Ϊ32ͨ��
	for (int x = 1; x <= 4; x++)   //4��FFTģ��
	{
		for (int i = 1; i <= DFT_MAXINPUTCHANNELCOUNT; i++)
		{
			//tempStr.Format(_T("Getting the information of FFT%d channels ---- %d / %d"), x, i, DFT_MAXINPUTCHANNELCOUNT);
			//SendSoftwareMessage(tempStr);

			ForcePostMessage(WM_STARTGETTINGFFT, i, (LPARAM)m_pTcpClient);
			//m_pTcpClient->HandleGetSoftwareMsg(tempStr);

			//��ȡFFT Channel�Ƿ񼤻���û�м���Ͳ���ȥ��ȡ��
			tempStr.Format(_T("fft%d channel%d active"), x, i);
			iret = GetSettingValue(tempStr, str_Setting);

			//Socket�Ͽ�������ͨѶ��������
			if (iret == -1)
			{
				delete startIndex;
				delete indexChannel;
				startIndex = NULL;
				indexChannel = NULL;
				return -1;
			}

			//���ش�����룬Ӧ����ͨ����Ȩ�������˳�����ѭ��
			if (iret == 1)
			{
				break; //�˳���ǰFFT��ͨ��ѭ��
			}

			if (str_Setting == _T("1"))
			{
				//��ȡFFT CHANNLE ��Source
				tempStr.Format("fft%d channel%d 100", x, i);
				iret = GetSettingValue(tempStr, str_Setting);

				//Socket�Ͽ�������ͨѶ��������
				if (iret == -1)
				{
					delete startIndex;
					delete indexChannel;
					startIndex = NULL;
					indexChannel = NULL;
					return -1;
				}

				//SOURCE��ȡ����
				if (str_Setting != _T("none") && str_Setting != _T(""))
				{
					*startIndex = 0;
					*indexChannel = 0;

					//������ͨ���ţ����õ���ϵͳ����ϵͳ�е�ͨ����
					GetIndex(GetCHIndexFromString(str_Setting), *startIndex, *indexChannel, 0, m_numofSystem, m_pNumofChannel);
					sectionName.Format(_T("SYS%d_Input%d"), *startIndex, *indexChannel);
					FFTChIndex[x][i] = sectionName;

					//FFT��־ΪON����Ϊ����״̬
					keyName.Format(_T("FFT%d"), x);
					::WritePrivateProfileStringA(sectionName, keyName, "ON", iniFileName);

					//��ȡ��ͨ����FFT��Input Filter�Ĳ���
					tempStr.Format("fft%d channel%d inputFilter", x, i);
					iret = GetSettingValue(tempStr, str_Setting);
					//Socket�Ͽ�������ͨѶ��������
					if (iret == -1)
					{
						delete startIndex;
						delete indexChannel;
						startIndex = NULL;
						indexChannel = NULL;
						return -1;
					}
					//��ȡ������д��
					keyName.Format(_T("FFT%d.InputFilter"), x);
					::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

					//��ȡ��ͨ����FFT��Weighting Window�Ĳ���
					tempStr.Format("fft%d channel%d 082", x, i);
					iret = GetSettingValue(tempStr, str_Setting);
					//Socket�Ͽ�������ͨѶ��������
					if (iret == -1)
					{
						delete startIndex;
						delete indexChannel;
						startIndex = NULL;
						indexChannel = NULL;
						return -1;
					}
					//��ȡ������д��
					keyName.Format(_T("FFT%d.Weighting"), x);
					::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

					//��ȡ��ӵĽ״���Ϣ���ܹ�8���״�
					CString CBTParaName[3] = { _T("Active"), _T("Order"), _T("Band") };

					//��ȡ��һ�е���Ϣ�������һ��ΪON�������ÿ�ж��ܵõ���Ϣ�������Ƿ񼤻�
					tempStr.Format("fft%d channel%d 215 0 0", x, i);
					iret = GetSettingValue(tempStr, str_Setting);
					//Socket�Ͽ�������ͨѶ��������
					if (iret == -1)
					{
						delete startIndex;
						delete indexChannel;
						startIndex = NULL;
						indexChannel = NULL;
						return -1;
					}
					//��ȡ����֤��û����ӽ״Σ������˳�����ѭ�������뵽��һ��ͨ����
					if (iret == 1)
					{
						keyName.Format(_T("FFT%d.TO.States"), x);
						::WritePrivateProfileStringA(sectionName, keyName, _T("OFF"), iniFileName);
						keyName.Format(_T("FFT%d.TO.Number"), x);
						::WritePrivateProfileStringA(sectionName, keyName, _T("0"), iniFileName);
						continue;  //������һ��ѭ����
					}

					//��ȡ������������к��������ݶ����Զ�ȡ��
					if (iret == 0)
					{
						//����ڵ�һ�е���������д��
						{
							keyName.Format(_T("FFT%d.TO.States"), x);
							::WritePrivateProfileStringA(sectionName, keyName, _T("ON"), iniFileName);

							keyName.Format(_T("FFT%d.TO.Number"), x);
							::WritePrivateProfileStringA(sectionName, keyName, _T("0"), iniFileName);

							//�ѵ�һ�е�����д��
							keyName.Format(_T("FFT%d.TO%d.%s"), x, 0, CBTParaName[0]);
							::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

							for (int k = 1; k < 3; k++)
							{
								tempStr.Format("fft%d channel%d 215 %d %d", x, i, 0, k);
								iret = GetSettingValue(tempStr, str_Setting);
								if (iret == -1)  //SOCKET�Ͽ�
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

						//����ѭ������ȡ��Ŀ
						bFirstFlag = TRUE;
						for (int j = 7; j > 0; j--)
						{
							tempStr.Format("fft%d channel%d 215 %d 0", x, i, j);
							iret = GetSettingValue(tempStr, str_Setting);

							//Socket�Ͽ�
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

										//д������Ĳ���
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
									//д����������
									keyName.Format(_T("FFT%d.TO%d.%s"), x, j, CBTParaName[0]);
									::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

									//д������Ĳ���
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

			}//��ǰFFT��Channel�Ѿ�����

		}
	}

	//��ȡTDA�Ĳ���
	// TDA��32��ͨ��

	for (int i = 1; i <= DFT_MAXINPUTCHANNELCOUNT; i++)
	{

		ForcePostMessage(WM_STARTGETTINGTDA, i, (LPARAM)m_pTcpClient);

		//��ȡFFT Channel�Ƿ񼤻���û�м���Ͳ���ȥ��ȡ��
		tempStr.Format(_T("tda channel%d active"), i);
		iret = GetSettingValue(tempStr, str_Setting);

		//Socket�Ͽ�������ͨѶ��������
		if (iret == -1)
		{
			delete startIndex;
			delete indexChannel;
			startIndex = NULL;
			indexChannel = NULL;
			return -1;
		}

		//���ش�����룬Ӧ����ͨ����Ȩ�������˳�����ѭ��
		if (iret == 1)
		{
			break; //�˳���ǰFFT��ͨ��ѭ��
		}

		if (str_Setting == _T("1"))
		{
			tempStr.Format("tda channel%d 100", i);
			iret = GetSettingValue(tempStr, str_Setting);

			if (iret == -1)    //Socket�Ͽ�
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

				//������ͨ���ţ����õ���ϵͳ����ϵͳ�е�ͨ����
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

	//��ȡORD�Ĳ������K����ص���Ϣд�뵽��Ӧ��INPUT��
	//ORD��ͨ����Ϊ32ͨ��

	//int moduleIDORD[3] = { 0, 14, 17 };
	for (int x = 1; x <= 2; x++)   //2��ORDģ��
	{
		for (int i = 1; i <= DFT_MAXINPUTCHANNELCOUNT; i++)
		{
			ForcePostMessage(WM_STARTGETTINGORD, i, (LPARAM)m_pTcpClient);
			//��ȡORD Channel�Ƿ񼤻���û�м���Ͳ���ȥ��ȡ��
			tempStr.Format(_T("soa%d channel%d active"), x, i);
			iret = GetSettingValue(tempStr, str_Setting);

			//Socket�Ͽ�������ͨѶ��������
			if (iret == -1)
			{
				delete startIndex;
				delete indexChannel;
				startIndex = NULL;
				indexChannel = NULL;
				return -1;
			}

			//���ش�����룬Ӧ����ͨ����Ȩ�������˳�����ѭ��
			if (iret == 1)
			{
				break; //�˳���ǰFFT��ͨ��ѭ��
			}

			if (str_Setting == _T("1"))
			{
				//��ȡORDER��Channel������Դ
				tempStr.Format("soa%d channel%d 100", x, i);
				iret = GetSettingValue(tempStr, str_Setting);

				//SOCKET�����˳�
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

					//������ͨ���ţ����õ���ϵͳ����ϵͳ�е�ͨ����
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


					//��ȡ��ӵĽ״���Ϣ���ܹ�8���״�
					CString CBTParaName[3] = { _T("Active"), _T("Order") };
					//��ȡ��һ�е���Ϣ�������һ��ΪON�������ÿ�ж��ܵõ���Ϣ�������Ƿ񼤻�
					tempStr.Format("soa%d channel%d 215 0 0", x, i);
					iret = GetSettingValue(tempStr, str_Setting);
					//Socket�Ͽ�������ͨѶ��������
					if (iret == -1)
					{
						delete startIndex;
						delete indexChannel;
						startIndex = NULL;
						indexChannel = NULL;
						return -1;
					}
					//��ȡ����֤��û����ӽ״Σ������˳�����ѭ�������뵽��һ��ͨ����
					if (iret == 1)
					{
						keyName.Format(_T("ORD%d.TO.States"), x);
						::WritePrivateProfileStringA(sectionName, keyName, _T("OFF"), iniFileName);
						keyName.Format(_T("ORD%d.TO.Number"), x);
						::WritePrivateProfileStringA(sectionName, keyName, _T("0"), iniFileName);
						continue;  //������һ��ѭ����
					}

					//��ȡ������������к��������ݶ����Զ�ȡ��
					if (iret == 0)
					{
						//����ڵ�һ�е���������д��
						{
							keyName.Format(_T("ORD%d.TO.States"), x);
							::WritePrivateProfileStringA(sectionName, keyName, _T("ON"), iniFileName);

							keyName.Format(_T("ORD%d.TO.Number"), x);
							::WritePrivateProfileStringA(sectionName, keyName, _T("0"), iniFileName);

							//�ѵ�һ�е�����д��
							keyName.Format(_T("ORD%d.TO%d.%s"), x, 0, CBTParaName[0]);
							::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

							for (int k = 1; k < 2; k++)
							{
								tempStr.Format("soa%d channel%d 215 %d %d", x, i, 0, k);
								iret = GetSettingValue(tempStr, str_Setting);
								if (iret == -1)  //SOCKET�Ͽ�
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

						//����ѭ������ȡ��Ŀ
						bFirstFlag = TRUE;
						for (int j = 7; j > 0; j--)
						{
							tempStr.Format("soa%d channel%d 215 %d 0", x, i, j);
							iret = GetSettingValue(tempStr, str_Setting);

							//Socket�Ͽ�
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

										//д������Ĳ���
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
									//д����������
									keyName.Format(_T("ORD%d.TO%d.%s"), x, j, CBTParaName[0]);
									::WritePrivateProfileStringA(sectionName, keyName, str_Setting, iniFileName);

									//д������Ĳ���
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

	//��ȡwaterfall�Ĳ���
	//Waterfall��96��ͨ��
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

		//������Ȩ
		if (iret == 1)
		{
			break;
		}

		if (str_Setting != _T("None"))
		{
			//�ҳ�����λ��
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
					//Ŀǰ����������� Ext��Frac
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

//������ϵͳ��Workbook
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

		//�����ӳ�д�룬�����в�����������д��
		iret = m_pTcpClient->SendDataToNVGate("TransactionBegin");
		if (iret == -1) return -1;

		//���¶Ͽ���������ͨ��
		tempStr.Format(_T("Re-connecting inputs for System #%d..."), nsys);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);

		if (nsys > 1)  //���ǵ�һ̨��ϵͳ
		{
			for (int i = 1; i <= m_pNumofChannel[nsys - 1]; i++)
			{
				sectionName.Format(_T("SYS%d_Input%d"), nsys, i);
				//��ȡͨ����Active����Ϣ
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

					//д��XPOD����Ϣ
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

					//���������Ĳ���
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
				else  //�Ͽ����ͨ��
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
			//�Ͽ������ͨ��
			for (int i = m_pNumofChannel[nsys - 1] + 1; i <= m_totalChannel; i++)
			{
				tempStr.Format(_T("SetSettingValue frontEnd input%d active 0"), i);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				tempStr.Format(_T("SetToDefault frontEnd input%d 0"), i);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				if (iret == -1) return -1;
			}
		}

		//��¼������
		//��¼����Ϊȱʡ
		tempStr.Format(_T("Re-setting recorder tracks for System #%d..."), nsys);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);

		iret = m_pTcpClient->SendDataToNVGate(_T("SetToDefaultCollection recorder track"));
		GetPrivateProfileString(sectionName, _T("REC"), _T(""), strINIInfo, 64, iniFileName);
		iniKeyValue = _T(strINIInfo);

		iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue recorder bandwiths 871 0"));
		GetPrivateProfileString(sectionName, _T("REC"), _T(""), strINIInfo, 64, iniFileName);
		iniKeyValue = _T(strINIInfo);

		//��Ӽ��������ͨ����RECORDER
		for (int i = 1; i <= m_pNumofChannel[nsys - 1]; i++)
		{
			sectionName.Format(_T("SYS%d_Input%d"), nsys, i);
			//��ȡͨ����Active����Ϣ
			GetPrivateProfileString(sectionName, _T("REC"), _T(""), strINIInfo, 64, iniFileName);
			iniKeyValue = _T(strINIInfo);

			if (iniKeyValue == _T("ON"))
			{
				//3����recorder
				tempStr.Format(_T("ConnectInput frontEnd.input%d recorder track%d"), i, i);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				if (iret == -1) return -1;

				//���������
				GetPrivateProfileString(sectionName, _T("REC.Sampling"), _T(""), strINIInfo, 64, iniFileName);
				iniKeyValue = _T(strINIInfo);
				tempStr.Format(_T("SetSettingValue recorder track%d 14 %s"), i, iniKeyValue);
				iret = m_pTcpClient->SendDataToNVGate(tempStr);
				if (iret == -1) return -1;
			}
		}

		//����ͨ����Automatic�ͼ�¼��λ�õ�ֵ
		iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue recorder recMode 873 0"));  //Automatic = Disable
		iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue recorder recMode 200 1"));  //Record = On PC

		//���ת��ͨ��
		GetPrivateProfileString(_T("Ext. Sync. 1"), _T("REC"), _T(""), strINIInfo, 64, iniFileName);
		iniKeyValue = _T(strINIInfo);
		if (iniKeyValue == _T("ON"))
		{
			//3����recorder
			tempStr.Format(_T("ConnectInput frontEnd.extSync1 recorder track%d"), m_pNumofChannel[nsys - 1] + 1);
			iret = m_pTcpClient->SendDataToNVGate(tempStr);
			if (iret == -1) return -1;
		}

		GetPrivateProfileString(_T("Ext. Sync. 2"), _T("REC"), _T(""), strINIInfo, 64, iniFileName);
		iniKeyValue = _T(strINIInfo);
		if (iniKeyValue == _T("ON"))
		{
			//3����recorder
			tempStr.Format(_T("ConnectInput frontEnd.extSync2 recorder track%d"), m_pNumofChannel[nsys - 1] + 2);
			iret = m_pTcpClient->SendDataToNVGate(tempStr);
			if (iret == -1) return -1;
		}

		GetPrivateProfileString(_T("Ext. Sync. 3"), _T("REC"), _T(""), strINIInfo, 64, iniFileName);
		iniKeyValue = _T(strINIInfo);
		if (iniKeyValue == _T("ON"))
		{
			//3����recorder
			tempStr.Format(_T("ConnectInput frontEnd.extSync3 recorder track%d"), m_pNumofChannel[nsys - 1] + 3);
			iret = m_pTcpClient->SendDataToNVGate(tempStr);
			if (iret == -1) return -1;
		}

		GetPrivateProfileString(_T("Ext. Sync. 4"), _T("REC"), _T(""), strINIInfo, 64, iniFileName);
		iniKeyValue = _T(strINIInfo);
		if (iniKeyValue == _T("ON"))
		{
			//3����recorder
			tempStr.Format(_T("ConnectInput frontEnd.extSync4 recorder track%d"), m_pNumofChannel[nsys - 1] + 4);
			iret = m_pTcpClient->SendDataToNVGate(tempStr);
			if (iret == -1) return -1;
		}

		//iret = SendDataToNVGate("TransactionEnd");
		//if (iret == -1) return -1;

		//iret = SendDataToNVGate("TransactionBegin");
		//if (iret == -1) return -1;

		//��ȡFFT1����Ϣ����д�뵽NVGATE
		for (int x = 1; x <= 4; x++)   //4��FFT
		{
			tempStr.Format(_T("Re-setting FFT%d channels for System #%d..."), x, nsys);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);
			//FFT��Channel���Ĭ�ϵ�
			tempStr.Format(_T("SetToDefaultCollection fft%d channel"), x);
			iret = m_pTcpClient->SendDataToNVGate(tempStr);
			//SOCKET�˳�
			if (iret == -1) return -1;

			for (int i = 1; i <= m_pNumofChannel[nsys - 1]; i++)
			{
				//��ȡINI�ļ�
				sectionName.Format(_T("SYS%d_Input%d"), nsys, i);
				keyName.Format(_T("FFT%d"), x);
				GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
				iniKeyValue = _T(strINIInfo);

				//��ͨ����FFT�Ѿ�����
				if (iniKeyValue == ("ON"))
				{
					//��������ͨ����FFT
					tempStr.Format(_T("ConnectInput frontEnd.input%d fft%d channel%d"), i, x, i);
					iret = m_pTcpClient->SendDataToNVGate(tempStr);
					if (iret == -1) return -1;

					if (iret == 0)
					{
						//д�����

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
							//���ٽ״ο��أ�1Ϊ�򿪣�0���ر�
							CString str_State = _T("");
							CString str_Order = _T("");
							CString str_BandWidth = _T("");

							//��ȡTO������
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

		//��ȡTDA����Ϣ����д�뵽NVGate��
		tempStr.Format(_T("Re-setting TDA channels for System #%d..."), nsys);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);
		{
			iret = m_pTcpClient->SendDataToNVGate(_T("SetToDefaultCollection tda channel"));
			if (iret == -1) return -1;

			for (int i = 1; i <= m_pNumofChannel[nsys - 1]; i++)
			{
				//��ȡINI�ļ�
				sectionName.Format(_T("SYS%d_Input%d"), nsys, i);
				GetPrivateProfileString(sectionName, _T("TDA"), _T(""), strINIInfo, 64, iniFileName);
				iniKeyValue = _T(strINIInfo);
				if (iniKeyValue == ("ON"))
				{
					//��������ͨ����FFT
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

		//��ȡORD����Ϣ����д�뵽NVGATE
		for (int x = 1; x <= 2; x++)   //2��soa
		{
			tempStr.Format(_T("Re-setting ORD%d channels for System #%d..."), x, nsys);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);

			tempStr.Format(_T("SetToDefaultCollection soa%d channel"), x);
			iret = m_pTcpClient->SendDataToNVGate(tempStr);
			if (iret == -1) return -1;

			for (int i = 1; i <= m_pNumofChannel[nsys - 1]; i++)
			{
				//��ȡINI�ļ�
				sectionName.Format(_T("SYS%d_Input%d"), nsys, i);
				keyName.Format(_T("ORD%d"), x);
				GetPrivateProfileString(sectionName, keyName, _T(""), strINIInfo, 64, iniFileName);
				iniKeyValue = _T(strINIInfo);

				//��ͨ����FFT�Ѿ�����
				if (iniKeyValue == ("ON"))
				{
					//��������ͨ����FFT
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
							//���ٽ״ο��أ�1Ϊ�򿪣�0���ر�
							CString str_State = _T("");
							CString str_Order = _T("");
							//CString str_BandWidth = _T("");

							//��ȡTO������
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
		//��ȡWTF����Ϣ����д�룡
		for (int i = 1; i <= m_pNumofChannel[nsys - 1]; i++)
		{
			sectionName.Format(_T("SYS%d_Input%d"), nsys, i);
			for (int x = 1; x <= 4; x++)  //4��FFT
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

			for (int x = 1; x <= 2; x++)  //2��ORD
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

		//����Ϊһ���µ�ģ��

		tempStr.Format(_T("Saving workbook model for System #%d..."), nsys);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)tempStr, (LPARAM)m_pTcpClient);

		iret = m_pTcpClient->SendDataToNVGate(_T("SetSilent 1"));
		if (iret == -1) return -1;

		tempStr.Format(_T("SaveWorkbookModel #%s_%d"), m_strConfigName, nsys);
		iret = m_pTcpClient->SendDataToNVGate(tempStr);
		if (iret == -1) return -1;
	}

	////��ȡConfiguration Number
	//GetPrivateProfileString(_T("Workbook"), _T("Number"), _T("0"), strINIInfo, 64, tempIniFileName);
	//int m_nWorkbookNumber = atoi(strINIInfo);

	//for (int i = 0; i < m_nWorkbookNumber; i++)
	//{

	//}
	
	////+1 д��
	//CString tempIniValue = _T("");
	//tempIniValue.Format(_T("%d"), m_nWorkbookNumber + 1);
	//::WritePrivateProfileStringA(_T("Workbook"), _T("Number"), tempIniValue, tempIniFileName);

	////д��Configuration Name
	//sectionName.Format(_T("Workbook%d"), m_nWorkbookNumber + 1);
	//::WritePrivateProfileStringA(_T("Workbook"), sectionName, m_strConfigName, tempIniFileName);

	////д��״̬
	//sectionName.Format(_T("Status%d"), m_nWorkbookNumber + 1);
	//::WritePrivateProfileStringA(_T("Workbook"), sectionName, _T("0"), tempIniFileName);

	return 0;
}

//��ȡNVGate Input�Ĳ������Kд�뵽INI�ļ���
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

	//�ж�iCount��ϵͳλ��
	GetIndex(icount, *startIndex, *indexChannel, 0, m_numofSystem, m_pNumofChannel);
	//int iIndex = *startIndex+1;

	tempStr.Format(commandPara, icount);
	//���ͻ�ȡ��Ϣ
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

//��ȡNVGATE�еĲ�������ֵ
int CGenerateModel::GetSettingValue(CString commandPara, CString& settingValue)
{
	CString str_Command = _T("");
	int iret = 0;

	str_Command.Format(_T("GetSettingValue %s"), commandPara);

	iret = m_pTcpClient->SendDataToNVGate(str_Command);
	if (iret == -1 || iret == 1) return iret;

	//ʶ��Setting��ֵ
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

//��ȡ����NVGate�е�ͨ�����ڼ�����ϵͳ���Լ���ϵͳ�ж�Ӧ������ͨ����
//icount = ����NVGateϵͳ�е�ͨ����
//indSystem = ��ϵͳ��
//indChannel = ��ϵͳ�е�ͨ����
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

//���ַ����л�ȡͨ����
//��ʽ��001.001.000������frontEnd.inputx �м��Ǹ����ֻ���x����ͨ����
int CGenerateModel::GetCHIndexFromString(CString str_Setting)
{
	int indexChannel = 0;
	CString b[5] = { _T("") };
	int iflag = 0;
	char* next_b = NULL;

	//�ֽ��ַ���
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
	next_b = NULL;   //��ɿ�ָ��

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

	//��ȡ��ǰ��Ӧ�ó���·������ȷ��INI�ļ��������ļ���
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
	
	// �ȴ���Զ��NVGate�ɹ�
	//HANDLE hWaitOpenNVGateEvents[2];
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);
	m_pTcpClient->SetTerminateStatus(FALSE);

	////���ð�ť
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

	//�˿ں�Ϊ3000
	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)_T("127.0.0.1"));
	m_pTcpClient->SetPort(3000);
	if (!m_pTcpClient->Connect(3))
	{
		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

		//���õ�ǰ��Ŀ����
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

	//��ȡ��ǰ��Ŀ����
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting current project name of local NVGate..."), (LPARAM)m_pTcpClient);
	iret = m_pTcpClient->SendDataToNVGate(_T("GetCurrentProjectName"));
	if (iret == -1)  //SOCKET�Ͽ�	
	{
		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//���õ�ǰ��Ŀ����
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
		//���õ�ǰ��Ŀ����
		ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
		Sleep(1);
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
		
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("The current project cannot be <Default Project>."), (LPARAM)m_pTcpClient);
		Sleep(1);

		MessageBox(m_MainHwnd, _T("The current project cannot be <Default Project>, please save as a new name."),
			_T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//�Ͽ����ӣ������̣߳��ر����ش���
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}
	else
	{
		::WritePrivateProfileStringA(_T("Parameters"), _T("ProjectName"), str_Temp, iniFileName);
	}

	//������Ŀ
	//for (int nsys = 1; nsys <= m_numofSystem; nsys++)
	{
		//���ӵ�Զ�̼����
		str_Temp.Format(_T("Attempting to connect the remote NVGate of System #%d..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		//�˿ں�Ϊ3000
		m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
		m_pTcpClient->SetPort(3000);
		if (!m_pTcpClient->Connect(3))
		{
			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

			//���õ�ǰ��Ŀ����
			ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
			Sleep(1);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Connecting to the remote NVGate #%d failed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�Ͽ�SOCKET�������߳�
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

		//����Workbook
		str_Temp.Format(_T("Loading workbook model to remote NVGate of System #%d..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		iret = m_pTcpClient->SendDataToNVGate(_T("SetSilent 1"));
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			//���õ�ǰ��Ŀ����
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
			//���õ�ǰ��Ŀ����
			ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
			Sleep(1);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting the current project of local NVGate failed!"), (LPARAM)m_pTcpClient);
			Sleep(1);

			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)_T("Getting the current project of local NVGate failed!"), (LPARAM)m_pTcpClient);
			MessageBox(m_MainHwnd, _T("Getting the current project of local NVGate failed!"), _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//���õ�ǰ��Ŀ����
			ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
			Sleep(1);

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			
			return;
		}


		str_Temp2.Replace(_T(" "), _T("\\ "));

		str_Temp.Format(_T("NewProject %s"), str_Temp2);
		iret = m_pTcpClient->SendDataToNVGate(str_Temp);
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			//���õ�ǰ��Ŀ����
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
				if (iret == -1)  //SOCKET�Ͽ�	
				{
					str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					//�ָ������
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					//���õ�ǰ��Ŀ����
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

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			//���õ�ǰ��Ŀ����
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

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			//���õ�ǰ��Ŀ����
			ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
			Sleep(1);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		default:
			break;
		}
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			
		}

		

		iret = m_pTcpClient->SendDataToNVGate("GetFanInfo");
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			//���õ�ǰ��Ŀ����
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
			//�򿪷���
			iret = m_pTcpClient->SendDataToNVGate(_T("SetFanStatus 0"));
			if (iret == -1)  //SOCKET�Ͽ�	
			{
				str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);

				str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
				MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

				//�ָ������
				ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
				//���õ�ǰ��Ŀ����
				ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
				Sleep(1);
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

				ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
				return;
			}
		}

		iret = m_pTcpClient->SendDataToNVGate("GetFanInfo");
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			//���õ�ǰ��Ŀ����
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
			if (iret == -1)  //SOCKET�Ͽ�	
			{
				str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);

				str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
				MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

				//�ָ������
				ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
				//���õ�ǰ��Ŀ����
				ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
				Sleep(1);
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
				ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
				return;
			}

			//��ȡFAN
			iret = m_pTcpClient->SendDataToNVGate("GetFanInfo");
			if (iret == -1)  //SOCKET�Ͽ�	
			{
				str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);

				str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
				MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

				//�ָ������
				ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
				//���õ�ǰ��Ŀ����
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


		////�������Ŀ
		//iret = m_pTcpClient->SendDataToNVGate(_T("SaveProject"));
		//if (iret == -1)  //SOCKET�Ͽ�	
		//{
		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		//	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		//	Sleep(1);

		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		//	MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//	//�ָ������
		//	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//	//���õ�ǰ��Ŀ����
		//	ForcePostMessage(WM_SETCURRENTPROJECTNAME, (WPARAM)(LPCTSTR)_T(""), (LPARAM)m_pTcpClient);
		//	Sleep(1);
		//	ForcePostMessage(WM_FINISHCOUNT, 3, (LPARAM)m_pTcpClient);

		//	////���ð�ť
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

		//�Ͽ�����
		//m_pTcpClient->Disconnect();
	}

	//�����߳�
	//�ָ������
	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

	str_Temp.Format(_T("Loading workbook model to remote NVGate of System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//���õ�ǰ��Ŀ����
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

// �ر��ļ������߳�
// �������κ�������Ϣ�����ն�
void COperateRemoteNVGate::CloseOperateNVGateThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // �ȴ��߳̽���
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

	//��ȡ��ǰ��Ӧ�ó���·������ȷ��INI�ļ��������ļ���
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

	// �ȴ���Զ��NVGate�ɹ�
	HANDLE hWaitOpenNVGateEvents[3];
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);

	//���ӵ�Զ�̼����
	str_Temp.Format(_T("Attempting to connect the controller PC of System #%d..."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//�˿ں�Ϊ8000
	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
	m_pTcpClient->SetPort(8000);
	if (!m_pTcpClient->Connect(0))
	{
		//ͳ������������뽻��������ȥ����
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

		//д�뵽NVGate��״̬INI�ļ���
		::WritePrivateProfileStringA(_T("NVGateStatus"), str_KeyName, _T("OFF"), iniFileName);

		//��ʾ������Ϣ
		str_Temp.Format(_T("Connecting the controller PC of System #%d failed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		//��ʾ���󱨾�
		str_Temp.Format(_T("Connecting the controller PC of System #%d failed, please check the network connection!"), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//�Ͽ�SOCKET�������߳�
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

	////Զ�̴�NVGate
	SendRequestOpenNVGateNetMsg(m_numofSystem);

	switch (WaitForMultipleObjects(3, hWaitOpenNVGateEvents, FALSE, m_nWaitTimeOut))
	{
	case WAIT_OBJECT_0: // �򿪴���
		CloseHandle(m_hOpenNVGateFail);
		CloseHandle(m_hOpenNVGateSucc);
		CloseHandle(m_hOpenNVGateMute);

		//ͳ������������뽻��������ȥ����
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

		//ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//д�뵽NVGate��״̬INI�ļ���
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
	case WAIT_OBJECT_0 + 1: // �򿪳ɹ�
		CloseHandle(m_hOpenNVGateFail);
		CloseHandle(m_hOpenNVGateSucc);
		CloseHandle(m_hOpenNVGateMute);
		break;
	case WAIT_OBJECT_0 + 2: // ����ر�
		CloseHandle(m_hOpenNVGateFail);
		CloseHandle(m_hOpenNVGateSucc);
		CloseHandle(m_hOpenNVGateMute);

		//ͳ������������뽻��������ȥ����
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

		//ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//д�뵽NVGate��״̬INI�ļ���
		::WritePrivateProfileStringA(_T("NVGateStatus"), str_KeyName, _T("OFF"), iniFileName);

		str_Temp.Format(_T("Open remote NVGate in System #%d failed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
		break;
	case WAIT_TIMEOUT: // �ȴ���ʱ
		CloseHandle(m_hOpenNVGateFail);
		CloseHandle(m_hOpenNVGateSucc);
		CloseHandle(m_hOpenNVGateMute);
		//ͳ������������뽻��������ȥ����
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

		//д�뵽NVGate��״̬INI�ļ���
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
		//ͳ������������뽻��������ȥ����
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

		//д�뵽NVGate��״̬INI�ļ���
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

	//�����߳�
	//�ָ������
	//delete[] i_pNVGateStartFlag;
	//ͳ������������뽻��������ȥ����
	ci_CountInfo->successFlag = 1;
	ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

	//д�뵽NVGate��״̬INI�ļ���
	::WritePrivateProfileStringA(_T("NVGateStatus"), str_KeyName, _T("ON"), iniFileName);

	//m_pTcpClient->GetAnalyzerStatus(m_pIpAddress, m_numofSystem, AfxGetMainWnd()->m_hWnd);

	//ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
	str_Temp.Format(_T("Open remote NVGate in System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);
	
	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// �ر��ļ������߳�
// �������κ�������Ϣ�����ն�
void COpenRemoteNVGate::CloseOpenNVGateThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // �ȴ��߳̽���
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

// �����������ļ�������Ϣ
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
	
	//�ȴ����Ĺ��
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);
	m_pTcpClient->SetTerminateStatus(FALSE);

	////���ð�ť
	//m_bBtnStates->b_ButtonStates = FALSE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	//������Ŀ
	//for (int nsys = 1; nsys <= m_numofSystem; nsys++)
	{
		//���ӵ�Զ�̼����
		str_Temp.Format(_T("Attempting to connect the remote NVGate of System #%d..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		//�˿ں�Ϊ3000
		m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
		m_pTcpClient->SetPort(3000);
		if (!m_pTcpClient->Connect(3))
		{
			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Connecting to the remote NVGate #%d failed, please open the remote NVGate or check the network connection."), m_numofSystem+1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			////���ð�ť
			//m_bBtnStates->b_ButtonStates = TRUE;
			//for (int i = 3; i <= 9; i++)
			//{
			//	m_bBtnStates->indexButton = i;
			//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			//	Sleep(1);
			//}
			//�Ͽ�SOCKET�������߳�
			
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
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}

		iret = m_pTcpClient->SendDataToNVGate(_T("SaveProject"));
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}


		//Alarm the recorder
		str_Temp.Format(_T("Arming the remote NVGate of System #%d ..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		//ֹͣ���������Ĳ�������ֹ������������
		iret = m_pTcpClient->SendDataToNVGate("GetGeneralAnalyzerState");
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem+1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			////���ð�ť
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

			////���ð�ť
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
				//����ѡ��ΪNot Save
				iret = m_pTcpClient->SendDataToNVGate(_T("SetSaveOption 0"));
				if (iret == -1)  //SOCKET�Ͽ�	
				{
					str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem+1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					//�ָ������
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					////���ð�ť
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

				//ֹͣһ�в�����������
				iret = m_pTcpClient->SendDataToNVGate(_T("Stop"));
				if (iret == -1)  //SOCKET�Ͽ�	
				{
					str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					//�ָ������
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
					return;
				}

				//���ñ���ѡ��Ϊsave without name confirmation
				iret = m_pTcpClient->SendDataToNVGate(_T("SetSaveOption 3"));
				if (iret == -1)  //SOCKET�Ͽ�	
				{
					str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					//�ָ������
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					////���ð�ť
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

				//��ȡ������״̬
				iret = m_pTcpClient->SendDataToNVGate(_T("GetGeneralAnalyzerState"));
				if (iret == -1)  //SOCKET�Ͽ�	
				{
					str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					//�ָ������
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					////���ð�ť
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

				//ֱ�������ǵ�״̬ΪSTOP,40s timeout
				while (str_StopFlag != "4")
				{
					nCurrentTime = GetTickCount();
					nDeltaTime = nCurrentTime - nStartTime;
					//��ʱ
					if (nDeltaTime > nTimeout)
					{
						ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

						str_Temp.Format(_T("Stopping the measurement of System #%d is timeout."), m_numofSystem + 1);
						ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
						Sleep(1);

						str_Temp.Format(_T("Stopping the measurement of System #%d is timeout."), m_numofSystem + 1);
						MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

						////���ð�ť
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

					//��ȡ������״̬
					iret = m_pTcpClient->SendDataToNVGate(_T("GetGeneralAnalyzerState"));
					if (iret == -1)  //SOCKET�Ͽ�	
					{
						str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
						ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
						Sleep(1);

						str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
						MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

						//�ָ������
						ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
						ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

						////���ð�ť
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

						////���ð�ť
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

		//���ARM״̬, ARM��ID��3 624 407
		//if (str_Feedback != "Armed")
		{
			iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue 3 624 407 1"));
			if (iret == -1)  //SOCKET�Ͽ�	
			{
				str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);

				str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
				MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

				//�ָ������
				ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

				//���ð�ť
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

				//�ָ������
				ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

				////���ð�ť
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

			//��ȡ������״̬
			iret = m_pTcpClient->SendDataToNVGate(_T("GetGeneralAnalyzerState"));
			if (iret == -1)  //SOCKET�Ͽ�	
			{
				str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
				ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
				Sleep(1);

				str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
				MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

				//�ָ������
				ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
				ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

				////���ð�ť
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

				////���ð�ť
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

			//ֱ�������ǵ�״̬ΪSTOP,40s timeout
			while (str_StopFlag != "11")
			{
				nCurrentTime = GetTickCount();
				nDeltaTime = nCurrentTime - nStartTime;
				//��ʱ
				if (nDeltaTime > nTimeout)
				{
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					str_Temp.Format(_T("Arming the remote NVGate of System #%d is timeout."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Arming the remote NVGate of System #%d is timeout."), m_numofSystem + 1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					////���ð�ť
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

				//��ȡ������״̬
				iret = m_pTcpClient->SendDataToNVGate(_T("GetGeneralAnalyzerState"));
				if (iret == -1)  //SOCKET�Ͽ�	
				{
					str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
					ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
					Sleep(1);

					str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
					MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

					//�ָ������
					ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
					ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

					////���ð�ť
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

					////���ð�ť
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

	//�����߳�
	//�ָ������
	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
	ci_CountInfo->successFlag = 1;
	ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

	str_Temp.Format(_T("Arming the remote NVGate of System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	////���ð�ť
	//m_bBtnStates->b_ButtonStates = TRUE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// �ر��ļ������߳�
// �������κ�������Ϣ�����ն�
void CAlarmSystem::CloseAlarmSystemThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // �ȴ��߳̽���
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

////�������ݸ�NVGate�����յ�������Ϣ
////���� -1��SOCKET�Ͽ���1�����ִ�����룻0����ȷִ��
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
//		return -1;  //SOCKET�Ͽ�
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
//			return -1;  //SOCKET�Ͽ�
//		}
//
//	}
//
//	if (bDataFromNVGate[0] == NVSUC_TAG) return 0;   //��ȷִ��
//	else return 1;    //���ִ���
//}
//
////�����յ��ķ�����Ϣ
//void CAlarmSystem::GetResString(char *Msg, int nMsgLen)
//{
//
//	bDataFromNVGate = Msg;
//	msgLen = nMsgLen;
//	m_bGetResponseFinish = TRUE;
//}
//
////��ȡ�ַ���
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
//	if (*a >= 1){//֮ǰΪֱ��ǿתΪint�Ĵ��룬���С����ʧ
//		int b = (int)(*a);
//		SettingValue.Format("%d", b);
//	}
//	else if (*a<1){//Ϊ��ά��ԭ���Ĵ��룬�˴�
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
	//�ȴ����Ĺ��
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);
	m_pTcpClient->SetTerminateStatus(FALSE);

	////���ð�ť
	//m_bBtnStates->b_ButtonStates = FALSE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	//������Ŀ
	//for (int nsys = 1; nsys <= m_numofSystem; nsys++)
	{
		//���ӵ�Զ�̼����
		str_Temp.Format(_T("Attempting to connect the remote NVGate of System #%d..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		//�˿ں�Ϊ3000
		m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
		m_pTcpClient->SetPort(3000);
		if (!m_pTcpClient->Connect(3))
		{
			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
			str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Connecting to the remote NVGate #%d failed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			////���ð�ť
			//m_bBtnStates->b_ButtonStates = TRUE;
			//for (int i = 3; i <= 9; i++)
			//{
			//	m_bBtnStates->indexButton = i;
			//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			//	Sleep(1);
			//}

			//�Ͽ�SOCKET�������߳�
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
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}

		iret = m_pTcpClient->SendDataToNVGate(_T("SaveProject"));
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�ָ������
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
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			////���ð�ť
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

	//�����߳�
	//�ָ������
	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
	ci_CountInfo->successFlag = 1;
	ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

	str_Temp.Format(_T("Starting monitoring in System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	////���ð�ť
	//m_bBtnStates->b_ButtonStates = TRUE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// �ر��ļ������߳�
// �������κ�������Ϣ�����ն�
void CStartMonitor::CloseStartMonitorThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // �ȴ��߳̽���
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

	//�ȴ����Ĺ��
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);

	m_pTcpClient->SetTerminateStatus(FALSE);

	//���ð�ť
	m_bBtnStates->b_ButtonStates = FALSE;
	for (int i = 3; i <= 9; i++)
	{
		m_bBtnStates->indexButton = i;
		ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
		Sleep(1);
	}

	//���ӵ�Զ�̼����
	str_Temp.Format(_T("Attempting to connect the remote NVGate of System #%d..."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//�˿ں�Ϊ3000
	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
	m_pTcpClient->SetPort(3000);
	if (!m_pTcpClient->Connect(3))
	{
		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);
		str_Temp.Format(_T("Connecting to the remote NVGate #%d failed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);
		//���ð�ť
		m_bBtnStates->b_ButtonStates = TRUE;
		for (int i = 3; i <= 9; i++)
		{
			m_bBtnStates->indexButton = i;
			ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			Sleep(1);
		}
		//�Ͽ�SOCKET�������߳�

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
	if (iret == -1)  //SOCKET�Ͽ�	
	{
		str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

		//���ð�ť
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
	//if (iret == -1)  //SOCKET�Ͽ�	
	//{
	//	str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
	//	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	//	Sleep(1);

	//	str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
	//	MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

	//	//�ָ������
	//	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

	//	//���ð�ť
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

	//�����߳�
	//�ָ������
	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);

	str_Temp.Format(_T("Starting the recording in System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//���ð�ť
	m_bBtnStates->b_ButtonStates = TRUE;
	for (int i = 3; i <= 9; i++)
	{
		m_bBtnStates->indexButton = i;
		ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
		Sleep(1);
	}

	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// �ر��ļ������߳�
// �������κ�������Ϣ�����ն�
void CStartRecord::CloseStartRecordThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // �ȴ��߳̽���
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
	//�ȴ����Ĺ��
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);
	m_pTcpClient->SetTerminateStatus(FALSE);

	//���ð�ť
	//m_bBtnStates->b_ButtonStates = FALSE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}
		
	//for (int nsys = 1; nsys <= m_numofSystem; nsys++)
	{
		//���ӵ�Զ�̼����
		str_Temp.Format(_T("Attempting to connect the remote NVGate of System #%d..."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		//�˿ں�Ϊ3000
		m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
		m_pTcpClient->SetPort(3000);
		if (!m_pTcpClient->Connect(3))
		{
			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
			str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);
			str_Temp.Format(_T("Connecting to the remote NVGate #%d failed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);
			////���ð�ť
			//m_bBtnStates->b_ButtonStates = TRUE;
			//for (int i = 3; i <= 9; i++)
			//{
			//	m_bBtnStates->indexButton = i;
			//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
			//	Sleep(1);
			//}
			//�Ͽ�SOCKET�������߳�

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
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			////���ð�ť
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

		//ֹͣͬ���ź����
		iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue 7 208 396 0"));
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
			return;
		}

		iret = m_pTcpClient->SendDataToNVGate(_T("Stop"));
		if (iret == -1)  //SOCKET�Ͽ�	
		{
			str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
			ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
			Sleep(1);

			str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
			MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

			//�ָ������
			ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
			ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

			////���ð�ť
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


		////�������ݲ�ָ���ļ���
		//iret = m_pTcpClient->SendDataToNVGate(_T("SaveResults 0 1"));
		//if (iret == -1)  //SOCKET�Ͽ�	
		//{
		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		//	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		//	Sleep(1);

		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		//	MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//	//�ָ������
		//	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//	ForcePostMessage(WM_FINISHCOUNT, 4, (LPARAM)m_pTcpClient);

		//	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		//	return;
		//}


		//iret = m_pTcpClient->SendDataToNVGate("GetAnalyzerState 3");
		//if (iret == -1)  //SOCKET�Ͽ�	
		//{
		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		//	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		//	Sleep(1);

		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		//	MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//	//�ָ������
		//	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//	ForcePostMessage(WM_FINISHCOUNT, 4, (LPARAM)m_pTcpClient);
		//	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		//	return;
		//}


		//iret = m_pTcpClient->SendDataToNVGate(_T("SetSettingValue 7 208 396 0"));
		//if (iret == -1)  //SOCKET�Ͽ�	
		//{
		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		//	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		//	Sleep(1);

		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		//	MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//	//�ָ������
		//	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//	ForcePostMessage(WM_FINISHCOUNT, 4, (LPARAM)m_pTcpClient);

		//	////���ð�ť
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
		//if (iret == -1)  //SOCKET�Ͽ�	
		//{
		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		//	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		//	Sleep(1);

		//	str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		//	MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//	//�ָ������
		//	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		//	ForcePostMessage(WM_FINISHCOUNT, 4, (LPARAM)m_pTcpClient);

		//	////���ð�ť
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

	//�����߳�
	//�ָ������
	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
	ci_CountInfo->successFlag = 1;
	ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

	str_Temp.Format(_T("Stop and saving the measurment in System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(100);

	////���ð�ť
	//m_bBtnStates->b_ButtonStates = TRUE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// �ر��ļ������߳�
// �������κ�������Ϣ�����ն�
void CStopRecord::CloseStopRecordThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // �ȴ��߳̽���
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

	//�ȴ����Ĺ��
	m_pTcpClient->SetTerminateStatus(FALSE);

	//������Ŀ
	//for (int nsys = 1; nsys <= m_numofSystem; nsys++)
	//while (!m_bTerminated)
	{
		m_sStateInfor->indexSystem = m_numofSystem;

	StartPoint:
		while (m_bConnectionFlag && !m_bTerminated)
		{
			//�˿ں�Ϊ3000
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
			if (iret == -1)  //SOCKET�Ͽ�	
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

				m_sStateInfor->stateString = _T("0 ��C");
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);

				goto StartPoint;
				return;
			}
			else if (iret == 1)   //��������
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
			if (iret == -1)  //SOCKET�Ͽ�	
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

				m_sStateInfor->stateString = _T("0 ��C");
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);

				goto StartPoint;
				return;
			}
			else if (iret == 1)   //��������
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
			if (iret == -1)  //SOCKET�Ͽ�	
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

				m_sStateInfor->stateString = _T("0 ��C");
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);

				goto StartPoint;
				return;
			}
			else if (iret == 1)   //��������
			{
				m_sStateInfor->stateString = _T("0 ��C");
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
			}
			else
			{
				m_pTcpClient->m_nCurrent = 18;
				m_pTcpClient->MakeLongInteger(m_pTcpClient->bDataFromNVGate, str_Feedback);
				m_sStateInfor->stateString.Format(_T("%s ��C"), str_Feedback);
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
			}

			Sleep(100);

		}

	}
	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// �ر��ļ������߳�
// �������κ�������Ϣ�����ն�
void CGetAnalyzerStatus::CloseGetAnalyzerStatusThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // �ȴ��߳̽���
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

	//�ȴ����Ĺ��
	//ForcePostMessage(WM_CURSORTYPE, 1, (LPARAM)m_pTcpClient);

	m_pTcpClient->SetTerminateStatus(FALSE);

	////���ð�ť
	//m_bBtnStates->b_ButtonStates = FALSE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	//���ӵ�Զ�̼����
	str_Temp.Format(_T("Attempting to connect the remote NVGate of System #%d..."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//�˿ں�Ϊ3000
	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
	m_pTcpClient->SetPort(3000);
	if (!m_pTcpClient->Connect(3))
	{
		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

		str_Temp.Format(_T("Connecting the remote NVGate of System #%d failed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Connecting to the remote NVGate #%d failed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		////���ð�ť
		//m_bBtnStates->b_ButtonStates = TRUE;
		//for (int i = 3; i <= 9; i++)
		//{
		//	m_bBtnStates->indexButton = i;
		//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
		//	Sleep(1);
		//}
		//�Ͽ�SOCKET�������߳�

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
	if (iret == -1)  //SOCKET�Ͽ�	
	{
		str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}

	iret = m_pTcpClient->SendDataToNVGate(_T("SetSilent 1"));
	if (iret == -1)  //SOCKET�Ͽ�	
	{
		str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}

	//���浱ǰ��WORKBOOK
	str_Temp.Format(_T("SaveWorkbookModel SuperModel_%d"), m_numofSystem + 1);
	iret = m_pTcpClient->SendDataToNVGate(str_Temp);
	if (iret == -1)  //SOCKET�Ͽ�	
	{
		str_Temp.Format(_T("Remote NVGate of System #%d is closed."), m_numofSystem + 1);
		ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
		Sleep(1);

		str_Temp.Format(_T("Remote NVGate of System #%d is closed, please open the remote NVGate or check the network connection."), m_numofSystem + 1);
		MessageBox(m_MainHwnd, str_Temp, _T("NVSuperVisor"), MB_ICONERROR | MB_OK);

		//�ָ������
		ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}


	//�����߳�
	//�ָ������
	ForcePostMessage(WM_CURSORTYPE, 0, (LPARAM)m_pTcpClient);
	ci_CountInfo->successFlag = 1;
	ForcePostMessage(WM_FINISHCOUNT, (WPARAM)ci_CountInfo, (LPARAM)m_pTcpClient);

	str_Temp.Format(_T("Starting the recording in System #%d succeed."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	////���ð�ť
	//m_bBtnStates->b_ButtonStates = TRUE;
	//for (int i = 3; i <= 9; i++)
	//{
	//	m_bBtnStates->indexButton = i;
	//	ForcePostMessage(WM_SETBUTTONSTATUS, (WPARAM)m_bBtnStates, (LPARAM)m_pTcpClient);
	//	Sleep(1);
	//}

	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// �ر��ļ������߳�
// �������κ�������Ϣ�����ն�
void CRunAutoZero::CloseRunAutoZeroThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // �ȴ��߳̽���
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

	//���ӵ�Զ�̼����
	str_Temp.Format(_T("Attempting to connect the controller PC of System #%d..."), m_numofSystem + 1);
	ForcePostMessage(WM_SETSOFTMSG, (WPARAM)(LPCTSTR)str_Temp, (LPARAM)m_pTcpClient);
	Sleep(1);

	//�˿ں�Ϊ8000
	m_pTcpClient->SetServerIpAddr((char *)(LPCTSTR)m_pIpAddress[m_numofSystem]);
	m_pTcpClient->SetPort(8000);
	if (!m_pTcpClient->Connect(0))
	{
		//�Ͽ�SOCKET�������߳�
		ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
		return;
	}

	SetSocketHandle(m_pTcpClient->m_hSocket);
	ASSERT(m_hSocket != INVALID_SOCKET);

	//Զ�̹ر�NVGate
	SendRequestCloseNVGateNetMsg(m_numofSystem);

	//���ͳɹ��󣬶Ͽ�SOCKET�������߳�
	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// �ر��ļ������߳�
// �������κ�������Ϣ�����ն�
void CCloseNVGate::CloseNVGateThreadMute(void)
{
	WaitForSingleObject(m_hThread, INFINITE); // �ȴ��߳̽���
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


// �����������ļ�������Ϣ
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

	//�ȴ����Ĺ��
	m_pTcpClient->SetTerminateStatus(FALSE);

	//������Ŀ
	//for (int nsys = 1; nsys <= m_numofSystem; nsys++)
	//while (!m_bTerminated)
	{
		m_sStateInfor->indexSystem = m_numofSystem;

	StartPoint:
		while (m_bConnectionFlag && !m_bTerminated)
		{
			//�˿ں�Ϊ3000
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
			if (iret == -1)  //SOCKET�Ͽ�	
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

				m_sStateInfor->stateString = _T("0 ��C");
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);

				goto StartPoint;
				return;
			}
			else if (iret == 1)   //��������
			{
				m_sStateInfor->stateString = _T("0 ��C");
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
			}
			else
			{
				m_pTcpClient->m_nCurrent = 18;
				m_pTcpClient->MakeLongInteger(m_pTcpClient->bDataFromNVGate, str_Feedback);
				m_sStateInfor->stateString.Format(_T("%s ��C"), str_Feedback);
				m_sStateInfor->indexStates = 3;
				ForcePostMessage(WM_ANALYZERINFO, (WPARAM)m_sStateInfor, (LPARAM)m_pTcpClient);
			}

			Sleep(50000);

		}

	}
	ForcePostMessage(WM_SOCKETCLOSE, 0, (LPARAM)m_pTcpClient);
}

// �ر��ļ������߳�
// �������κ�������Ϣ�����ն�
void CFanStatus::CloseGetFanStatusThreadMute(void)
{
	Terminate();
	WaitForSingleObject(m_hThread, INFINITE); // �ȴ��߳̽���
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

