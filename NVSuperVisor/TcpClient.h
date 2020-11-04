// TcpClient
// ���Է���������Ϣ�����Է����ļ���֧�ֶϵ�����
// XSoft
// Contact: hongxing777@msn.com or lovelypengpeng@eyou.com
//
#include "RemoteDesktop.h"
#include "IniFileReadWrite.h"

//#include <winsock2.h>
//#include <stdio.h>
//#pragma comment(lib,"ws2_32.lib")


#ifndef _TCPCLIENT_INCLUDE_
#define _TCPCLIENT_INCLUDE_

#include <winsock2.h>
#include "Thread.h"
#include "io.h"
#include <string>

// ȱʡֵ����
#define DFT_SOCKETPORT						(8000) // ȱʡ�˿�
#define DFT_FILEPACKAGESIZE					(1024) // ȱʡ�ļ�����С
#define DFT_WAITTIMEOUT						(60000) // �ȴ���ʱ
#define DFT_SENDTIMEOUT						(10000) // ���ͳ�ʱ
#define DFT_SOCKETRECVTHREADPRIORITY		(THREAD_PRIORITY_NORMAL)
#define DFT_SENDFILETHREADPRIORITY			(THREAD_PRIORITY_BELOW_NORMAL)
#define DFT_PROGRESSTIMEINTERVAL			(100) // ������Ϣ���ʱ��
#define DFT_MAXINPUTCHANNELCOUNT            (256) // ϵͳ���������������ͨ����
#define DFT_MAXRECCHANNELCOUNT              (300) // ϵͳ�����������j��¼��ͨ����
#define DFT_MAXWTFCHANNELCOUNT              (96) // ϵͳ������������ٲ�ͼͨ����


// ��������
#define MSG_TAG								('@')
#define NVSUC_TAG							('0')
#define SOCKET_RECVBUFSIZE					(32768) // socket���ջ������Ĵ�С
#define MAX_NETMSGPACKAGESIZE				(99999) // ������Ϣ���Ĵ�С, ��ֵ������Э�������ڱ�ʾ��Ϣ���ȵ��ֽ�����
#define MAX_FILEPACKAESIZE					(MAX_NETMSGPACKAGESIZE - 3) // �ļ����Ĵ�С
#define NETMSG_RECVBUFSIZE					(2 * MAX_NETMSGPACKAGESIZE) // ������Ϣ��������С
#define STR_CLIENTSOCKETHIDEWNDCLASSNAME	("client-socket-hide-wndclass") // ���ش�������

#define WM_TCPCLIENTBASE					(WM_USER + 1000)
#define WM_SOCKETRECVERR					(WM_TCPCLIENTBASE + 0) // socket���ճ���
#define WM_SOCKETSENDERR					(WM_TCPCLIENTBASE + 1) // socket���ͳ���
#define	WM_SOCKETCLOSE						(WM_TCPCLIENTBASE + 2) // socket���ر�
#define	WM_SOCKETREFUSE						(WM_TCPCLIENTBASE + 3) // socket���ܾ�
#define	WM_DISCARDTHREAD					(WM_TCPCLIENTBASE + 4) // socket���ܾ�
#define WM_CURSORTYPE                       (WM_TCPCLIENTBASE + 5) // cursor type
#define WM_ONLYCLOSESOCKET                  (WM_TCPCLIENTBASE + 6)  //�����ر��ļ�����SOCKET
#define WM_ANALYZERINFO                     (WM_TCPCLIENTBASE + 7)  //�����ر��ļ���


#define WM_RECVERACCEPTFILE					(WM_TCPCLIENTBASE + 20) // ���ն˽����ļ�
#define WM_RECVERREFUSEFILE					(WM_TCPCLIENTBASE + 21) // ���ն˾ܾ��ļ�
#define WM_RECVERSUCC						(WM_TCPCLIENTBASE + 22) // ���ն˳ɹ�
#define WM_RECVERCANCEL						(WM_TCPCLIENTBASE + 23) // ���ն�ȡ��
#define WM_RECVERFAIL						(WM_TCPCLIENTBASE + 14) // ���ն˳���
#define WM_OPENNVGATESUCC                   (WM_TCPCLIENTBASE + 15) // ��NVGate�ɹ�
#define WM_OPENNVGATEFAIL                   (WM_TCPCLIENTBASE + 16) // ��NVGateʧ��

#define WM_GETLOCALSUCC                     (WM_TCPCLIENTBASE + 32)  //��ȡ����NVGate��Ϣ�ɹ�
#define WM_GETLOCALFAIL                     (WM_TCPCLIENTBASE + 33)  //��ȡ����NVGate��Ϣʧ��
#define WM_GETSOFTWAREMSG                   (WM_TCPCLIENTBASE + 34)  //��ʾ�����Ϣ
#define WM_SETSOFTMSG                       (WM_TCPCLIENTBASE + 35)  //���ʹ�����Ϣ

#define WM_WAITTIMEOUT						(WM_TCPCLIENTBASE + 40) // �ȴ���ʱ
#define WM_SENDERFAIL						(WM_TCPCLIENTBASE + 41) // ���Ͷ˳���
#define WM_SENDFILEPROGRESS					(WM_TCPCLIENTBASE + 42) // �����ļ�����


#define WM_STARTGETTINGINFO                 (WM_TCPCLIENTBASE + 61) //��ʼ��ȡ����NVGATE����Ϣ
#define WM_STARTGETTINGINPUT                (WM_TCPCLIENTBASE + 62) //��ʼ��ȡ����NVGATE��INPUT��Ϣ
#define WM_STARTGETTINGREC                  (WM_TCPCLIENTBASE + 63) //��ʼ��ȡ����NVGATE��REC��Ϣ
#define WM_STARTGETTINGFFT                  (WM_TCPCLIENTBASE + 64) //��ʼ��ȡ����NVGATE��FFT��Ϣ
#define WM_STARTGETTINGTDA                  (WM_TCPCLIENTBASE + 65) //��ʼ��ȡ����NVGATE��TDA��Ϣ
#define WM_STARTGETTINGORD                  (WM_TCPCLIENTBASE + 66) //��ʼ��ȡ����NVGATE��SOA��Ϣ
#define WM_STARTGETTINGWTF                  (WM_TCPCLIENTBASE + 67) //��ʼ��ȡ����NVGATE��WTF��Ϣ
#define WM_GETTINGINFOSUCC                  (WM_TCPCLIENTBASE + 68) //��ʼ��ȡ����NVGATE��WTF��Ϣ
#define WM_GETTINGINFOFAIL                  (WM_TCPCLIENTBASE + 69) //��ʼ��ȡ����NVGATE��WTF��Ϣ

#define WM_SETBUTTONSTATUS                  (WM_TCPCLIENTBASE + 70) //���ð�ť״̬

#define WM_FINISHCOUNT                      (WM_TCPCLIENTBASE + 100)  //���߳�ͬʱ�������COUNTͳ�ƣ���ִ����Ӧ�Ķ���
#define WM_EXEGETTINGSTATUS                 (WM_TCPCLIENTBASE + 101)
#define WM_SETCURRENTPROJECTNAME            (WM_TCPCLIENTBASE + 102)




// ǰ������
class CTcpClient; // Tcp�ͻ�����
class CClientSocketRecvThread; // socket���ݽ����߳�
class CSendFileThread; // �����ļ��߳�
class CGenerateModel; //��ȡ����NVGate��Ϣ��������workbook model�߳�
class COperateRemoteNVGate;  //����Զ��NVGate
class COpenRemoteNVGate;    //��Զ��NVGATE
class CAlarmSystem;         //Alarm Recorder
class CStartMonitor; //Start Monitor 
class CStartRecord; //Start Record
class CStopRecord;  //Stop Record
class CGetAnalyzerStatus;
class CRunAutoZero;
class CCloseNVGate;
class CFanStatus;
//class CRemoteDesktop;

// socket�رյ�ԭ��
typedef enum {
	SCR_PEERCLOSE, // socket���Եȶ˹ر�
	SCR_SOCKETSENDERR, // socket���ͳ���
	SCR_SOCKETRECVERR // socket���ճ���
}EMSocketCloseReason;

// �����ļ�ʧ��ԭ��
typedef enum {
	SFFR_SOCKETCLOSE, // socket���ر�
	SFFR_SOCKETSENDERR, // socket���ͳ���
	SFFR_SOCKETRECVERR, // socket���ճ���
	SFFR_RECVERREFUSEFILE, // ���ն˾ܾ�
	SFFR_RECVERCANCEL, // ���ն�ȡ��
	SFFR_RECVERFAIL, // ���ն�ʧ��
	SFFR_WAITTIMEOUT, // �ȴ���ʱ
	SFFR_SENDERCANCEL, // ���Ͷ�ȡ��
	SFFR_SENDERFAIL, // ���Ͷ�ʧ��
}EMSendFileFailReason;

typedef unsigned long long  UWORD;

typedef struct StatesInfo
{
	int indexSystem;
	int indexStates;
	CString stateString;

}STATEINFO;

typedef struct countInfo
{
	int iSource;
	int successFlag;
}COUNTINFO;

typedef struct buttonStates
{
	int indexButton;
	BOOL b_ButtonStates;
}BTNSTATES;

// �ص���������
typedef void (*SocketConnectFun)(void *pNotifyObj, SOCKET hSocket);
typedef void (*SocketCloseFun)(void *pNotifyObj, SOCKET hSocket, EMSocketCloseReason scr);
typedef void (*OneNetMsgFun)(void *pNotifyObj, char *Msg, int nMsgLen); // ���յ�һ��������������Ϣ
typedef void (*OneMsgFun)(void *pNotifyObj, CString Msg); // ���յ�һ������Ϣ
//typedef void (*OneErrorMsgFun)(void *pNotifyObj, CString ErrorMsg);  //�յ�������Ϣ
typedef void (*SendFileFun)(void *pNotifyObj, char *szPathName);
typedef void (*SendFileFailFun)(void *pNotifyObj, char *szPathName, EMSendFileFailReason SendFileFailReason);
typedef void (*SendFileProgressFun)(void *pNotifyObj, UWORD nSentBytes, UWORD nTotalBytes);
typedef void (*SetCursorTypeFun)(void *pNotifyObj, int cursorType);
typedef void (*SetButtonStatusFun)(void *pNotifyObj, int buttonID, BOOL buttonStatus);
typedef void (*SetAnalyzerStatusFun)(void *pNotifyObj, int indexSystem, CString analyzerStatus, int indexStates);
typedef void (*ExeGetAnalyzerStatusFun)(void *pNotifyObj, int indexSyste, CString analyzerStates);
typedef void (*SetCurrentProjectNameFun)(void *pNotifyObj, CString projectName);

typedef void (*SetFinishCountFun)(void *pNotifyObj, int iSource, int successFlag);


// Tcp�ͻ���
class CTcpClient
{
private:
	WSADATA									m_WSAData;
	void									*m_pNotifyObj; // �¼�֪ͨ����ָ��
	
	
	char									m_szServerIpAddr[MAX_PATH]; // ������ip��ַ
	WORD									m_wPort; // �˿�
	DWORD									m_dwFilePackageSize; // �ļ�����С
	int										m_nWaitTimeOut; // �ȴ���ʱ
	int										m_nSendTimeOut; // �������ݳ�ʱ
	DWORD									m_dwProgressTimeInterval; // ������Ϣ���ʱ��
	int										m_nSocketRecvThreadPriority;
	int										m_nSendFileThreadPriority;

	int                                     m_nSource;  //1=NVGATE, 0=MSG
	int                                     m_totalChannels; //��ͨ����

	HWND                                    m_MainHwnd; //������

	


	CClientSocketRecvThread					*m_pClientSocketRecvThread; // socket�����߳�
	CSendFileThread							*m_pSendFileThread; // �����ļ��߳�
	CGenerateModel                          *m_pGenerateModelThread; //������WORKBOOK
	COperateRemoteNVGate                    *m_pOperateRemoteNVGate; //����Զ��NVGate�߳�
	
	CAlarmSystem                            *m_pAlarmSystem; //Alarm System�߳�
	CStartMonitor                           *m_pStartMonitor; //Start Monitor�߳�
	CStartRecord                            *m_pStartRecord; //Start Record�߳�
	CStopRecord                             *m_pStopRecord; //Stop Record�߳�
	
	CRunAutoZero                            *m_pRunAutoZero;    //RunAutoZero Thread
	CCloseNVGate                            *m_pCloseNVGate;    //Close NVGate
	CFanStatus                              *m_pGetFanStatus;   //Get Fan Status

	// �ص�����ָ��
	SocketConnectFun                        m_OnSocketConnect;
	SocketCloseFun							m_OnSocketClose;
	OneNetMsgFun							m_OnOneNetMsg;
	OneMsgFun                               m_OnOneMsg;
	//OneErrorMsgFun                          m_OnOneErrorMsg;
	SendFileFun								m_OnSendFileSucc;
	SendFileFailFun							m_OnSendFileFail;
	SendFileProgressFun						m_OnSendFileProgress;
	SetCursorTypeFun                        m_OnSetCursorType;
	SetButtonStatusFun                      m_OnSetButtonStatus;
	SetAnalyzerStatusFun                    m_OnSetAnalyzerStatus;
	ExeGetAnalyzerStatusFun                 m_OnExeGetAnalyzerStatus;
	SetCurrentProjectNameFun                m_OnSetCurrentProjectName;

	SetFinishCountFun                       m_OnSetFinishCount;

	

	BOOL RegisterClientSocketHideWndClass(void);
	BOOL StartupSocket(void);
	void CleanupSocket(void);
	void InitSockAddr(SOCKADDR_IN  *pSockAddr, char *szBindIpAddr, WORD wPort);
	BOOL AllocateWindow(void);
	void DeallocateWindow(void);

	// ������Ϣ������
	void HandleSocketSendErrMsg(void);
	void HandleSocketRecvErrMsg(void);
	void HandleSocketRefuseMsg(void);
	void HandleRecverAcceptFileMsg(DWORD dwRecvedBytes);
	void HandleRecverRefuseFileMsg(void);
	void HandleRecverSuccMsg(void);
	void HandleRecverFailMsg(void);
	void HandleRecverCancelMsg(void);
	void HandleSenderFailMsg(void);
	void HandleWaitTimeOutMsg(void);
	void HandleSendFileProgressMsg(void);
	void HandleOpenNVGateSuccMsg(void);
	void HandleOpenNVGateFailMsg(void);
	
	void HandleSetAnalyzerStatus(STATEINFO sStateInfo);
	void HandleExeGetAnalyzerStatus(int indexSyste, CString analyzerSates);
	void HandleSetCurrentProjectName(CString projectName);

	void HandleSetFinishCount(int iSource, int successFlag);

public:

	////NVGATE��ز���
	int                                     m_nCurrent;
	char									*bDataFromNVGate;   //NVGATE����������BYTE
	int										msgLen; //�������ݵĳ���
	BOOL									m_bGetResponseFinish; //��ȡ��ɵı�ʶ��
	BOOL                                    m_bTerminateStatus; //SOCKET�Ƿ�ر�
	COpenRemoteNVGate                       *m_pOpenRemoteNVGate; //��Զ��
	CGetAnalyzerStatus                      *m_pGetAnalyzerStatus; //Get analyzer status ?

public:
	static LRESULT CALLBACK HideWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // ������Ϣ����
	CTcpClient(void *pNotifyObj);
	~CTcpClient(void);

	// get/set����
	void SetServerIpAddr(char *szServerIpAddr);
	void SetPort(WORD wPort);
	void SetFilePackageSize(DWORD dwFilePackageSize);
	void SetWaitTimeOut(int nWaitTimeOut);
	void SetSendTimeOut(int nSendTimeOut);
	void SetProgressTimeInterval(DWORD dwProgressTimeInterval);
	void SetSocketRecvThreadPriority(int nPriority);
	void SetSendFileThreadPriority(int nPriority);
	void SetOnSocketConnect(SocketConnectFun OnSocketConnect);
	void SetOnSocketClose(SocketCloseFun OnSocketClose);
	void SetOnOneNetMsg(OneNetMsgFun OnOneNetMsg);
	void SetOnOneMsg(OneMsgFun OnOneMsg);
	//void SetOnOneErrorMsg(OneErrorMsgFun OnOneErrorMsg);
	void SetOnSendFileSucc(SendFileFun OnSendSucc);
	void SetOnSendFileFail(SendFileFailFun OnSendFail);
	void SetOnSendFileProgress(SendFileProgressFun OnSendFileProgressFun);
	void SetOnSetCursorType(SetCursorTypeFun OnSetCursorType);
	void SetOnButtonStatus(SetButtonStatusFun OnSetButtonStatus);
	void SetOnAnalyzerStatus(SetAnalyzerStatusFun OnSetAnalyzerStatus);
	void SetOnExeGetAnalyzerStatus(ExeGetAnalyzerStatusFun OnExeGetAnalyzerStatus);
	void SetOnCurrentProjectName(SetCurrentProjectNameFun OnSetCurrentProjectName);

	void HandleOpenNVGateMuteMsg(void);

	void setOnFinishCount(SetFinishCountFun OnSetFinishCount);

	void SetTerminateStatus(BOOL b_Status);
	void SetFinishStatus(BOOL b_Status);
	void SetHwnd(HWND m_Hwnd);
	void HandleSocketCloseMsg();
	void HandleSocketOnlyCloseMsg(void);
	//int SendDataToNVGateA(CString str_Command);

	void SetGettingStateFlag(void);
	void SetGettingFanStateFlag(void);

	void HandleGetSoftwareMsg(CString Msg);
	//void HandleGetErrorMsg(CString Msg);
	void HandleSetCursor(int cursorType);
	void HandleSetButtonStatus(int buttonID, BOOL buttonStatus);

	BOOL Connect(int source);
	void Disconnect(void);
	void DiscardThread(void);
	BOOL IsConnect(void);
	BOOL IsSending(void);
	BOOL SendFile(CString *ipAddress, int numofSystem, CString cfgName, int nPlace, HWND mHwnd);
	BOOL GetGenerateWorkbook(int *numofChannel, CString *ipAddress, int numofSystem, int totalChannels, CString configName, HWND mHwnd);
	BOOL OperateRemoteNVGate(CString *ipAddress, int numofSystem, CString workbookName, HWND mHwnd);
	BOOL OpenRemoteNVGate(CString *ipAddress, int numofSystem, HWND mHwnd, CString *HardIpAddr, CString *HardType);
	BOOL AlarmRecorder(CString *ipAddress, int numofSystem, HWND mHwnd);
	BOOL StartMonitor(CString *ipAddress, int numofSystem, HWND mHwnd);
	BOOL StartRecord(CString *ipAddress, int numofSystem, HWND mHwnd);
	BOOL StopRecord(CString *ipAddress, int numofSystem, HWND mHwnd);
	BOOL GetAnalyzerStatus(CString *ipAddress, int numofSystem, HWND mHwnd);
	BOOL RunAutoZero(CString *ipAddress, int numofSystem, HWND mHwnd);
	BOOL CloseNVGate(CString *ipAddress, int numofSystem, HWND mHwnd);
	BOOL GetFanStatus(CString *ipAddress, int numofSystem, HWND mHwnd);
	//BOOL GetLocalNVGateInfor(NVMI *nvMaster, int numofSystem, int totalChannels);
	void CancelSendFile(void);
	BOOL SendNetMsg(char *Msg, int nMsgLen, int source);    //source = 0 from file transfering, source = 1 from NVGate

	HWND									m_hHideWnd; // ���ش���
	SOCKET									m_hSocket; // socket���
	
	// ������Ϣ������
	void HandleOneNetMsg(char *Msg, int nMsgLen);
	void HandleRefuseMsg();

	////NVGate��ز�������
	int SendDataToNVGate(CString str_Command);
	void GetResString(char *Msg, int nMsgLen);
	void MakeString(char* bDataFromNVGate, CString & szString);
	void MakeInteger(char* bDataFromNVGATE, CString& SettingValue);
	void MakeLongInteger(char* bDataFromNVGATE, CString& SettingValue);
	void MakeFloat(char* bDataFromNVGATE, CString& SettingValue);
	void MakeFloatA(char* bDataFromNVGATE, CString& SettingValue);

};

// �ļ������߳�
class CSendFileThread: public CThread
{
private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // ���ش�����
	SOCKET									m_hSocket;
	DWORD									m_dwFilePackageSize; // ����С
	int										m_nWaitTimeOut; // �ȴ���ʱ
	DWORD									m_dwProgressTimeInterval; // ������Ϣ���ʱ��
	CFile									m_File; // �������ļ�����

	//int                                     *m_pNumofChannel;
	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //����������
	//int                                     m_totalChannel; //�ܵ�ͨ����
	//int                                     m_nCurrent;  //��ȡ���˵�
	//CString                                 iniFileName; //ini�ļ���·��

	//char									*bDataFromNVGate;   //NVGATE����������BYTE
	//int										msgLen; //�������ݵĳ���
	//CString									str_Setting;  //��ȡSetting���ַ���
	//BOOL									m_bGetResponseFinish; //��ȡ��ɵı�ʶ��


	// ״̬����
	char									m_szPathName[MAX_PATH]; // �ļ�·����
	UWORD									m_dwSentBytes; // �ѷ����ֽ���
	UWORD									m_dwFileSize; // �ļ���С
	UWORD									m_dwLastProgressTick;

	// �¼����
	HANDLE									m_hCloseThreadMute; // �ر��߳�
	HANDLE									m_hCloseThreadCancel; // ���ڷ��Ͷ�ȡ�����Ͷ��ر��߳�
	HANDLE									m_hAcceptFile; // ���ն˽����ļ�
	HANDLE									m_hRecvFileSucc; // ���ն˽����ļ��ɹ�

	HWND                                    m_MainHwnd;   //������

	CString                                 m_sCfgName;   //Configuration Name

	BTNSTATES                               *m_bBtnStates;
	CString                                 m_sWorkbookFolder;
	CString                                 iniFileName;

	int                                     m_nSendPlace;

	COUNTINFO                               *ci_CountInfo;


	void SendFileData(void);
	BOOL SendRequestSendFileNetMsg();
	BOOL SendSenderCancelNetMsg(void);
	BOOL SendSenderFailNetMsg(void);

	BOOL SendRequestOpenNVGateNetMsg();
	
	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	//void SendSoftwareMessage(CString errorMsg);
	void ResetSendFile(void);
	void Execute(void);

	int ConnectToNVGate(CString str_IPAddress);

public:
	CSendFileThread(CTcpClient *pTcpClientSocket, int nPriority);
	~CSendFileThread(void);

	// get / set����
	void SetHideWndHandle(HWND hHideWnd);
	void SetSocketHandle(SOCKET hSocket);
	void SetFilePackageSize(DWORD dwFilePackageSize);
	void SetWaitTimeOut(int nWaitTimeOut);
	void SetProgressTimeInterval(DWORD dwProgressTimeInterval);
	void SetHwnd(HWND mHwnd);
	void SetCfgName(CString cfgName);
	void SetSendPlace(int nPlace);   //����Workbook�����λ��
	
	char *GetPathName(void);
	UWORD GetSentBytes(void);
	UWORD GetFileSize(void);

	void TrigAcceptFile(DWORD dwRecvedBytes);
	void TrigRecvFileSucc(void);

	BOOL PrepareSendFile(char *szPathName);
	BOOL IsFileAccepted(void); // ����ļ��Ƿ��Ѿ����ն˽���
	void CloseSendFileThreadMute(void);
	void CloseSendFileThreadCancel(void);
	//void OpenNVGateSucc(void);

	BOOL PrepareSendWorkbookFiles(CString *ipAddress, int numofSystem);
	//void GetResString(char *Msg, int nMsgLen);
	
//private:
//	int GetLocalNVGateData(void);
//	int GenerateSubModel(void);
//	int SendDataToNVGate(CString str_Command);
//	int GenNVGatePara(CString commandPara, int icount, CString paraName, CString section);
//	int GetSettingValue(CString commandPara, CString& settingValue);
//	
//	void MakeString(char* bDataFromNVGate, CString & szString);
//	void MakeInteger(char* bDataFromNVGATE, CString& SettingValue);
//	void MakeFloat(char* bDataFromNVGATE, CString& SettingValue);
//
//	void GetIndex(int icount, int &indSystem, int &indChannel, int tempCount, int nbSystem, const int* m_pNumberOfChannel);
//	int GetCHIndexFromString(CString str_Setting);


};

// �ͻ���socket�����߳�
class CClientSocketRecvThread: public CThread
{
private:
	CTcpClient								*m_pTcpClient;
	SOCKET									m_hSocket;
	HWND									m_hHideWnd;
	char									m_RecvBuf[NETMSG_RECVBUFSIZE];
	int										m_nRecvBufSize; // ���ջ��������ֽ���
	int                                     m_source;       //ȷ��������Ϣ����Դ 0 =  FILE, 1 = NVGATE

	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	void Execute(void);
	void DoRecv(char *Buf, int nBytes);
	void HandleRecvedNetMsg(char *Msg, int nMsgLen);
public:
	CClientSocketRecvThread(CTcpClient *pTcpClient, int nPriority, int source);
	~CClientSocketRecvThread(void);

	// get / set ����
	void SetSocketHandle(SOCKET hSocket);
	void SetHideWndHandle(HWND hHideWnd);

};

class CGenerateModel : public CThread
{
private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // ���ش�����
	SOCKET									m_hSocket;
	int										m_nWaitTimeOut; // �ȴ���ʱ

	int                                     *m_pNumofChannel;
	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //����������
	int                                     m_totalChannel; //�ܵ�ͨ����
	int                                     m_nCurrent;  //��ȡ���˵�
	CString                                 iniFileName; //ini�ļ���·��
	CString                                 tempIniFileName;  //Tempsetting ini�ļ�·��
	char									*bDataFromNVGate;   //NVGATE����������BYTE
	int										msgLen; //�������ݵĳ���

	
	CString									str_Setting;  //��ȡSetting���ַ���
	//BOOL									m_bGetResponseFinish; //��ȡ��ɵı�ʶ��

	HWND                                    m_MainHwnd;   //����?

	BTNSTATES                               *m_bBtnStates;
	CString                                 m_strConfigName;

	CIniFileReadWrite                       *m_pIniFileReadWrite;


public:
	CGenerateModel(CTcpClient *pTcpClientSocket, int nPriority);
	~CGenerateModel(void);

	BOOL PrepareGetLocalInformation(int *numofChannel, CString *ipAddress, int numofSystem, int totalChannels);
	void SetHideWndHandle(HWND hHideWnd);
	void SetSocketHandle(SOCKET hSocket);
	void SetConfigName(CString configName);
	void SetHwnd(HWND mHwnd);
	int SendDataToNVGate(CString str_Command);
	void GetResString(char *Msg, int nMsgLen);
	void CloseGenerateWorkbookThreadMute(void);

private:
	int GetLocalNVGateData(void);
	int GenerateSubModel(void);
	
	int GenNVGatePara(CString commandPara, int icount, CString paraName, CString section);
	int GetSettingValue(CString commandPara, CString& settingValue);
	
	void MakeString(char* bDataFromNVGate, CString & szString);
	void MakeInteger(char* bDataFromNVGATE, CString& SettingValue);
	void MakeFloat(char* bDataFromNVGATE, CString& SettingValue);

	void GetIndex(int icount, int &indSystem, int &indChannel, int tempCount, int nbSystem, const int* m_pNumberOfChannel);
	int GetCHIndexFromString(CString str_Setting);
	int ConnectToNVGate(CString str_IPAddress);

	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	//void SendSoftwareMessage(CString errorMsg);
	void Execute(void);

};


//����Զ��NVGate�߳�
class COperateRemoteNVGate :
	public CThread
{

private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // ���ش�����
	SOCKET									m_hSocket;
	int										m_nWaitTimeOut; // �ȴ���ʱ

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //����������

	//char									*bDataFromNVGate;   //NVGATE����������BYTE
	//int										msgLen; //�������ݵĳ���
	//BOOL									m_bGetResponseFinish; //��ȡ��ɵı�ʶ��
	CString                                 iniFileName;

	HWND                                    m_MainHwnd;   //����?

	BTNSTATES                               *m_bBtnStates;
	CRemoteDesktop                          *m_pRemoteDesktop;
	COUNTINFO                               *ci_CountInfo;

	CString                                 m_sWorkbookName;

public:
	COperateRemoteNVGate(CTcpClient *pTcpClientSocket, int nPriority);
	~COperateRemoteNVGate(void);

	void SetHideWndHandle(HWND hHideWnd);
	void SetSocketHandle(SOCKET hSocket);
	void SetHwnd(HWND m_Hwnd);
	void SetWorkbookName(CString wrkName);
	void SetIPAddress(CString *ipAddress);
	void SetNumOfSystem(int numofSystem);
	/*void OpenRemoteNVGateSucc();*/
	//int SendDataToNVGate(CString str_Command);
	//void GetResString(char *Msg, int nMsgLen);
	void CloseOperateNVGateThreadMute(void);

private:
	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	//void SendSoftwareMessage(CString errorMsg);
	void Execute(void);
};


//����Զ��NVGate�߳�
class COpenRemoteNVGate :
	public CThread
{
private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // ���ش�����
	SOCKET									m_hSocket;
	int										m_nWaitTimeOut; // �ȴ���ʱ

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //����������
	CString                                 *m_pHardIpAddr;   //Ӳ����IP��ַ
	CString                                 *m_PHardType;     //ϵͳ����

	HANDLE                                  m_hOpenNVGateSucc; //�򿪳ɹ�
	HANDLE                                  m_hOpenNVGateFail; //��ʧ��
	HANDLE                                  m_hOpenNVGateMute;  //����ر�

	HWND                                    m_MainHwnd;   //��������

	BTNSTATES                               *m_bBtnStates;
	COUNTINFO                               *ci_CountInfo;
	
	CString                                 iniFileName;

	BOOL SendRequestOpenNVGateNetMsg(int nSys);

public:
	COpenRemoteNVGate(CTcpClient *pTcpClientSocket, int nPriority);
	~COpenRemoteNVGate(void);

	void SetHideWndHandle(HWND hHideWnd);
	void SetSocketHandle(SOCKET hSocket);

	void SetIPAddress(CString *ipAddress);
	void SetNumOfSystem(int numofSystem);
	void SetHardIpAddr(CString *HardIpAddr);
	void SetHardType(CString *HardType);
	void SetHwnd(HWND m_Hwnd);
	void OpenRemoteNVGateSucc();
	void OpenRemoteNVGateFail();
	void OpenRemoteNVGateMute();

	void CloseOpenNVGateThreadMute(void);

private:
	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	void Execute(void);
};


//Alarm Recorder�߳�

class CAlarmSystem :
	public CThread
{

private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // ���ش�����
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //����������

	HWND                                    m_MainHwnd;   //��������

	//int                                     m_nCurrent;  //��ȡ���˵�
	//char									*bDataFromNVGate;   //NVGATE����������BYTE
	//int										msgLen; //�������ݵĳ���
	//BOOL									m_bGetResponseFinish; //��ȡ��ɵı�ʶ��
	BTNSTATES                               *m_bBtnStates;
	COUNTINFO                               *ci_CountInfo;

public:
	CAlarmSystem(CTcpClient *pTcpClientSocket, int nPriority);
	~CAlarmSystem(void);

	void SetHideWndHandle(HWND hHideWnd);
	void SetSocketHandle(SOCKET hSocket);
	void SetIPAddress(CString *ipAddress);
	void SetNumOfSystem(int numofSystem);
	void SetHwnd(HWND m_Hwnd);

	//int SendDataToNVGate(CString str_Command);
	//void GetResString(char *Msg, int nMsgLen);
	//void MakeString(char* bDataFromNVGate, CString & szString);
	//void MakeInteger(char* bDataFromNVGATE, CString& SettingValue);
	//void MakeFloat(char* bDataFromNVGATE, CString& SettingValue);

	void CloseAlarmSystemThreadMute(void);

private:
	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	void Execute(void);
};



//Start Monitor�߳�

class CStartMonitor :
	public CThread
{

private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // ���ش�����
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //����������

	HWND                                    m_MainHwnd;   //��������

	//int                                     m_nCurrent;  //��ȡ���˵�
	//char									*bDataFromNVGate;   //NVGATE����������BYTE
	//int										msgLen; //�������ݵĳ���
	//BOOL									m_bGetResponseFinish; //��ȡ��ɵı�ʶ��
	BTNSTATES                               *m_bBtnStates;
	COUNTINFO                               *ci_CountInfo;


public:
	CStartMonitor(CTcpClient *pTcpClientSocket, int nPriority);
	~CStartMonitor(void);

	void SetHideWndHandle(HWND hHideWnd);
	void SetSocketHandle(SOCKET hSocket);

	void SetIPAddress(CString *ipAddress);
	void SetNumOfSystem(int numofSystem);
	void SetHwnd(HWND m_Hwnd);

	//int SendDataToNVGate(CString str_Command);
	//void GetResString(char *Msg, int nMsgLen);
	//void MakeString(char* bDataFromNVGate, CString & szString);
	//void MakeInteger(char* bDataFromNVGATE, CString& SettingValue);
	//void MakeFloat(char* bDataFromNVGATE, CString& SettingValue);

	void CloseStartMonitorThreadMute(void);

private:
	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	void Execute(void);

};

//-----------------------------------------------------

class CStartRecord :
	public CThread
{
private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // ���ش�����
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //����������
	HWND                                    m_MainHwnd;   //����?

	//int                                     m_nCurrent;  //��ȡ���˵�
	//char									*bDataFromNVGate;   //NVGATE����������BYTE
	//int										msgLen; //�������ݵĳ���
	//BOOL									m_bGetResponseFinish; //��ȡ��ɵı�ʶ��
	BTNSTATES                               *m_bBtnStates;

public:
	CStartRecord(CTcpClient *pTcpClientSocket, int nPriority);
	~CStartRecord(void);

	void SetHideWndHandle(HWND hHideWnd);
	void SetSocketHandle(SOCKET hSocket);
	void SetIPAddress(CString *ipAddress);
	void SetNumOfSystem(int numofSystem);
	void SetHwnd(HWND m_Hwnd);

	//int SendDataToNVGate(CString str_Command);
	//void GetResString(char *Msg, int nMsgLen);

	void CloseStartRecordThreadMute(void);

private:
	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	void Execute(void);

};



//------------------------------------------------------------------------------

class CStopRecord :
	public CThread
{
private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // ���ش�����
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //����������
	HWND                                    m_MainHwnd;   //��?

	//int                                     m_nCurrent;  //��ȡ���˵�
	//char									*bDataFromNVGate;   //NVGATE����������BYTE
	//int										msgLen; //�������ݵĳ���
	//BOOL									m_bGetResponseFinish; //��ȡ��ɵı�ʶ��
	BTNSTATES                               *m_bBtnStates;
	COUNTINFO                               *ci_CountInfo;


public:
	CStopRecord(CTcpClient *pTcpClientSocket, int nPriority);
	~CStopRecord(void);

	void SetHideWndHandle(HWND hHideWnd);
	void SetSocketHandle(SOCKET hSocket);

	void SetIPAddress(CString *ipAddress);
	void SetNumOfSystem(int numofSystem);
	void SetHwnd(HWND m_Hwnd);

	//int SendDataToNVGate(CString str_Command);
	//void GetResString(char *Msg, int nMsgLen);

	void CloseStopRecordThreadMute(void);

private:
	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	void Execute(void);

};


//------------------------------------------------------------------------------



class CGetAnalyzerStatus :
	public CThread
{
private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // ���ش�����
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //����������

	HWND                                    m_MainHwnd;   //��������

	STATEINFO                               *m_sStateInfor;
	BOOL                                    m_bStartFlag;
	BOOL                                    m_bConnectionFlag;
	CString                                 iniFileName;

					
public:
	CGetAnalyzerStatus(CTcpClient *pTcpClientSocket, int nPriority);
	~CGetAnalyzerStatus(void);

	void SetHideWndHandle(HWND hHideWnd);
	void SetSocketHandle(SOCKET hSocket);
	void SetIPAddress(CString *ipAddress);
	void SetNumOfSystem(int numofSystem);
	void SetHwnd(HWND m_Hwnd);
	void SetStartFlag(BOOL startFlag);
	void SetConnectionFlag(BOOL connectionFlag);

	void CloseGetAnalyzerStatusThreadMute(void);

private:
	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	void Execute(void);

};



class CRunAutoZero :
	public CThread
{
private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // ���ش�����
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //����������

	HWND                                    m_MainHwnd;   //��������

	BTNSTATES                               *m_bBtnStates; //
	COUNTINFO                               *ci_CountInfo;
	//BOOL                                    m_bStartFlag;
	//BOOL                                    m_bConnectionFlag;


public:
	CRunAutoZero(CTcpClient *pTcpClientSocket, int nPriority);
	~CRunAutoZero(void);

	void SetHideWndHandle(HWND hHideWnd);
	void SetSocketHandle(SOCKET hSocket);
	void SetIPAddress(CString *ipAddress);
	void SetNumOfSystem(int numofSystem);
	void SetHwnd(HWND m_Hwnd);
	//void SetStartFlag(BOOL startFlag);
	//void SetConnectionFlag(BOOL connectionFlag);

	void CloseRunAutoZeroThreadMute(void);

private:
	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	void Execute(void);

};




class CCloseNVGate :
	public CThread
{
private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // ���ش�����
	SOCKET									m_hSocket;
	int										m_nWaitTimeOut; // �ȴ���ʱ

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //����������

	HANDLE                                  m_hOpenNVGateSucc; //�򿪳ɹ�
	HANDLE                                  m_hOpenNVGateFail; //��ʧ��

	HWND                                    m_MainHwnd;   //��������

	BOOL SendRequestCloseNVGateNetMsg(int nSys);

public:
	CCloseNVGate(CTcpClient *pTcpClientSocket, int nPriority);
	~CCloseNVGate(void);

	void SetHideWndHandle(HWND hHideWnd);
	void SetSocketHandle(SOCKET hSocket);

	void SetIPAddress(CString *ipAddress);
	void SetNumOfSystem(int numofSystem);
	void SetHwnd(HWND m_Hwnd);

	void CloseNVGateThreadMute(void);

private:
	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	void Execute(void);


};




class CFanStatus :
	public CThread
{

private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // ���ش�����
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //����������

	HWND                                    m_MainHwnd;   //��������

	STATEINFO                               *m_sStateInfor;
	BOOL                                    m_bStartFlag;
	BOOL                                    m_bConnectionFlag;

public:
	CFanStatus(CTcpClient *pTcpClientSocket, int nPriority);
	virtual ~CFanStatus();

	void SetHideWndHandle(HWND hHideWnd);
	void SetSocketHandle(SOCKET hSocket);
	void SetIPAddress(CString *ipAddress);
	void SetNumOfSystem(int numofSystem);
	void SetHwnd(HWND m_Hwnd);
	void SetStartFlag(BOOL startFlag);
	void SetConnectionFlag(BOOL connectionFlag);

	void CloseGetFanStatusThreadMute(void);

private:
	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	void Execute(void);
};


#endif // #ifndef _TCPCLIENT_INCLUDE_


