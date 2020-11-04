// TcpClient
// 可以发送网络信息，可以发送文件，支持断点续传
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

// 缺省值定义
#define DFT_SOCKETPORT						(8000) // 缺省端口
#define DFT_FILEPACKAGESIZE					(1024) // 缺省文件包大小
#define DFT_WAITTIMEOUT						(60000) // 等待超时
#define DFT_SENDTIMEOUT						(10000) // 发送超时
#define DFT_SOCKETRECVTHREADPRIORITY		(THREAD_PRIORITY_NORMAL)
#define DFT_SENDFILETHREADPRIORITY			(THREAD_PRIORITY_BELOW_NORMAL)
#define DFT_PROGRESSTIMEINTERVAL			(100) // 进度消息间隔时间
#define DFT_MAXINPUTCHANNELCOUNT            (256) // 系统可允许的最大的输入通道数
#define DFT_MAXRECCHANNELCOUNT              (300) // 系统可允许的最大的j记录仪通道数
#define DFT_MAXWTFCHANNELCOUNT              (96) // 系统可允许的最大的瀑布图通道数


// 常量定义
#define MSG_TAG								('@')
#define NVSUC_TAG							('0')
#define SOCKET_RECVBUFSIZE					(32768) // socket接收缓冲区的大小
#define MAX_NETMSGPACKAGESIZE				(99999) // 网络消息包的大小, 该值决定于协议中用于表示消息长度的字节数。
#define MAX_FILEPACKAESIZE					(MAX_NETMSGPACKAGESIZE - 3) // 文件包的大小
#define NETMSG_RECVBUFSIZE					(2 * MAX_NETMSGPACKAGESIZE) // 网络消息缓冲区大小
#define STR_CLIENTSOCKETHIDEWNDCLASSNAME	("client-socket-hide-wndclass") // 隐藏窗体类名

#define WM_TCPCLIENTBASE					(WM_USER + 1000)
#define WM_SOCKETRECVERR					(WM_TCPCLIENTBASE + 0) // socket接收出错
#define WM_SOCKETSENDERR					(WM_TCPCLIENTBASE + 1) // socket发送出错
#define	WM_SOCKETCLOSE						(WM_TCPCLIENTBASE + 2) // socket被关闭
#define	WM_SOCKETREFUSE						(WM_TCPCLIENTBASE + 3) // socket被拒绝
#define	WM_DISCARDTHREAD					(WM_TCPCLIENTBASE + 4) // socket被拒绝
#define WM_CURSORTYPE                       (WM_TCPCLIENTBASE + 5) // cursor type
#define WM_ONLYCLOSESOCKET                  (WM_TCPCLIENTBASE + 6)  //仅仅关闭文件发送SOCKET
#define WM_ANALYZERINFO                     (WM_TCPCLIENTBASE + 7)  //仅仅关闭文件发


#define WM_RECVERACCEPTFILE					(WM_TCPCLIENTBASE + 20) // 接收端接受文件
#define WM_RECVERREFUSEFILE					(WM_TCPCLIENTBASE + 21) // 接收端拒绝文件
#define WM_RECVERSUCC						(WM_TCPCLIENTBASE + 22) // 接收端成功
#define WM_RECVERCANCEL						(WM_TCPCLIENTBASE + 23) // 接收端取消
#define WM_RECVERFAIL						(WM_TCPCLIENTBASE + 14) // 接收端出错
#define WM_OPENNVGATESUCC                   (WM_TCPCLIENTBASE + 15) // 打开NVGate成功
#define WM_OPENNVGATEFAIL                   (WM_TCPCLIENTBASE + 16) // 打开NVGate失败

#define WM_GETLOCALSUCC                     (WM_TCPCLIENTBASE + 32)  //获取本地NVGate信息成功
#define WM_GETLOCALFAIL                     (WM_TCPCLIENTBASE + 33)  //获取本地NVGate信息失败
#define WM_GETSOFTWAREMSG                   (WM_TCPCLIENTBASE + 34)  //显示软件信息
#define WM_SETSOFTMSG                       (WM_TCPCLIENTBASE + 35)  //发送错误信息

#define WM_WAITTIMEOUT						(WM_TCPCLIENTBASE + 40) // 等待超时
#define WM_SENDERFAIL						(WM_TCPCLIENTBASE + 41) // 发送端出错
#define WM_SENDFILEPROGRESS					(WM_TCPCLIENTBASE + 42) // 发送文件进度


#define WM_STARTGETTINGINFO                 (WM_TCPCLIENTBASE + 61) //开始获取本地NVGATE的信息
#define WM_STARTGETTINGINPUT                (WM_TCPCLIENTBASE + 62) //开始获取本地NVGATE的INPUT信息
#define WM_STARTGETTINGREC                  (WM_TCPCLIENTBASE + 63) //开始获取本地NVGATE的REC信息
#define WM_STARTGETTINGFFT                  (WM_TCPCLIENTBASE + 64) //开始获取本地NVGATE的FFT信息
#define WM_STARTGETTINGTDA                  (WM_TCPCLIENTBASE + 65) //开始获取本地NVGATE的TDA信息
#define WM_STARTGETTINGORD                  (WM_TCPCLIENTBASE + 66) //开始获取本地NVGATE的SOA信息
#define WM_STARTGETTINGWTF                  (WM_TCPCLIENTBASE + 67) //开始获取本地NVGATE的WTF信息
#define WM_GETTINGINFOSUCC                  (WM_TCPCLIENTBASE + 68) //开始获取本地NVGATE的WTF信息
#define WM_GETTINGINFOFAIL                  (WM_TCPCLIENTBASE + 69) //开始获取本地NVGATE的WTF信息

#define WM_SETBUTTONSTATUS                  (WM_TCPCLIENTBASE + 70) //设置按钮状态

#define WM_FINISHCOUNT                      (WM_TCPCLIENTBASE + 100)  //多线程同时处理，完成COUNT统计，并执行相应的动作
#define WM_EXEGETTINGSTATUS                 (WM_TCPCLIENTBASE + 101)
#define WM_SETCURRENTPROJECTNAME            (WM_TCPCLIENTBASE + 102)




// 前置声明
class CTcpClient; // Tcp客户端类
class CClientSocketRecvThread; // socket数据接收线程
class CSendFileThread; // 发送文件线程
class CGenerateModel; //读取本地NVGate信息，生成子workbook model线程
class COperateRemoteNVGate;  //操作远程NVGate
class COpenRemoteNVGate;    //打开远程NVGATE
class CAlarmSystem;         //Alarm Recorder
class CStartMonitor; //Start Monitor 
class CStartRecord; //Start Record
class CStopRecord;  //Stop Record
class CGetAnalyzerStatus;
class CRunAutoZero;
class CCloseNVGate;
class CFanStatus;
//class CRemoteDesktop;

// socket关闭的原因
typedef enum {
	SCR_PEERCLOSE, // socket被对等端关闭
	SCR_SOCKETSENDERR, // socket发送出错
	SCR_SOCKETRECVERR // socket接收出错
}EMSocketCloseReason;

// 发送文件失败原因
typedef enum {
	SFFR_SOCKETCLOSE, // socket被关闭
	SFFR_SOCKETSENDERR, // socket发送出错
	SFFR_SOCKETRECVERR, // socket接收出错
	SFFR_RECVERREFUSEFILE, // 接收端拒绝
	SFFR_RECVERCANCEL, // 接收端取消
	SFFR_RECVERFAIL, // 接收端失败
	SFFR_WAITTIMEOUT, // 等待超时
	SFFR_SENDERCANCEL, // 发送端取消
	SFFR_SENDERFAIL, // 发送端失败
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

// 回调函数声明
typedef void (*SocketConnectFun)(void *pNotifyObj, SOCKET hSocket);
typedef void (*SocketCloseFun)(void *pNotifyObj, SOCKET hSocket, EMSocketCloseReason scr);
typedef void (*OneNetMsgFun)(void *pNotifyObj, char *Msg, int nMsgLen); // 接收到一条完整的网络消息
typedef void (*OneMsgFun)(void *pNotifyObj, CString Msg); // 接收到一条完消息
//typedef void (*OneErrorMsgFun)(void *pNotifyObj, CString ErrorMsg);  //收到错误信息
typedef void (*SendFileFun)(void *pNotifyObj, char *szPathName);
typedef void (*SendFileFailFun)(void *pNotifyObj, char *szPathName, EMSendFileFailReason SendFileFailReason);
typedef void (*SendFileProgressFun)(void *pNotifyObj, UWORD nSentBytes, UWORD nTotalBytes);
typedef void (*SetCursorTypeFun)(void *pNotifyObj, int cursorType);
typedef void (*SetButtonStatusFun)(void *pNotifyObj, int buttonID, BOOL buttonStatus);
typedef void (*SetAnalyzerStatusFun)(void *pNotifyObj, int indexSystem, CString analyzerStatus, int indexStates);
typedef void (*ExeGetAnalyzerStatusFun)(void *pNotifyObj, int indexSyste, CString analyzerStates);
typedef void (*SetCurrentProjectNameFun)(void *pNotifyObj, CString projectName);

typedef void (*SetFinishCountFun)(void *pNotifyObj, int iSource, int successFlag);


// Tcp客户端
class CTcpClient
{
private:
	WSADATA									m_WSAData;
	void									*m_pNotifyObj; // 事件通知对象指针
	
	
	char									m_szServerIpAddr[MAX_PATH]; // 服务器ip地址
	WORD									m_wPort; // 端口
	DWORD									m_dwFilePackageSize; // 文件包大小
	int										m_nWaitTimeOut; // 等待超时
	int										m_nSendTimeOut; // 发送数据超时
	DWORD									m_dwProgressTimeInterval; // 进度消息间隔时间
	int										m_nSocketRecvThreadPriority;
	int										m_nSendFileThreadPriority;

	int                                     m_nSource;  //1=NVGATE, 0=MSG
	int                                     m_totalChannels; //总通道数

	HWND                                    m_MainHwnd; //窗体句柄

	


	CClientSocketRecvThread					*m_pClientSocketRecvThread; // socket接收线程
	CSendFileThread							*m_pSendFileThread; // 发送文件线程
	CGenerateModel                          *m_pGenerateModelThread; //生成子WORKBOOK
	COperateRemoteNVGate                    *m_pOperateRemoteNVGate; //操作远程NVGate线程
	
	CAlarmSystem                            *m_pAlarmSystem; //Alarm System线程
	CStartMonitor                           *m_pStartMonitor; //Start Monitor线程
	CStartRecord                            *m_pStartRecord; //Start Record线程
	CStopRecord                             *m_pStopRecord; //Stop Record线程
	
	CRunAutoZero                            *m_pRunAutoZero;    //RunAutoZero Thread
	CCloseNVGate                            *m_pCloseNVGate;    //Close NVGate
	CFanStatus                              *m_pGetFanStatus;   //Get Fan Status

	// 回调函数指针
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

	// 窗体消息处理函数
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

	////NVGATE相关参数
	int                                     m_nCurrent;
	char									*bDataFromNVGate;   //NVGATE反馈的数据BYTE
	int										msgLen; //返回数据的长度
	BOOL									m_bGetResponseFinish; //获取完成的标识符
	BOOL                                    m_bTerminateStatus; //SOCKET是否关闭
	COpenRemoteNVGate                       *m_pOpenRemoteNVGate; //打开远程
	CGetAnalyzerStatus                      *m_pGetAnalyzerStatus; //Get analyzer status ?

public:
	static LRESULT CALLBACK HideWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // 窗体消息函数
	CTcpClient(void *pNotifyObj);
	~CTcpClient(void);

	// get/set函数
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

	HWND									m_hHideWnd; // 隐藏窗体
	SOCKET									m_hSocket; // socket句柄
	
	// 网络消息处理函数
	void HandleOneNetMsg(char *Msg, int nMsgLen);
	void HandleRefuseMsg();

	////NVGate相关操作函数
	int SendDataToNVGate(CString str_Command);
	void GetResString(char *Msg, int nMsgLen);
	void MakeString(char* bDataFromNVGate, CString & szString);
	void MakeInteger(char* bDataFromNVGATE, CString& SettingValue);
	void MakeLongInteger(char* bDataFromNVGATE, CString& SettingValue);
	void MakeFloat(char* bDataFromNVGATE, CString& SettingValue);
	void MakeFloatA(char* bDataFromNVGATE, CString& SettingValue);

};

// 文件发送线程
class CSendFileThread: public CThread
{
private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // 隐藏窗体句柄
	SOCKET									m_hSocket;
	DWORD									m_dwFilePackageSize; // 包大小
	int										m_nWaitTimeOut; // 等待超时
	DWORD									m_dwProgressTimeInterval; // 进度消息间隔时间
	CFile									m_File; // 待发送文件对象

	//int                                     *m_pNumofChannel;
	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //主机的数量
	//int                                     m_totalChannel; //总的通道数
	//int                                     m_nCurrent;  //读取便宜点
	//CString                                 iniFileName; //ini文件的路径

	//char									*bDataFromNVGate;   //NVGATE反馈的数据BYTE
	//int										msgLen; //返回数据的长度
	//CString									str_Setting;  //获取Setting的字符串
	//BOOL									m_bGetResponseFinish; //获取完成的标识符


	// 状态变量
	char									m_szPathName[MAX_PATH]; // 文件路径名
	UWORD									m_dwSentBytes; // 已发送字节数
	UWORD									m_dwFileSize; // 文件大小
	UWORD									m_dwLastProgressTick;

	// 事件句柄
	HANDLE									m_hCloseThreadMute; // 关闭线程
	HANDLE									m_hCloseThreadCancel; // 由于发送端取消发送而关闭线程
	HANDLE									m_hAcceptFile; // 接收端接受文件
	HANDLE									m_hRecvFileSucc; // 接收端接收文件成功

	HWND                                    m_MainHwnd;   //主窗体

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

	// get / set函数
	void SetHideWndHandle(HWND hHideWnd);
	void SetSocketHandle(SOCKET hSocket);
	void SetFilePackageSize(DWORD dwFilePackageSize);
	void SetWaitTimeOut(int nWaitTimeOut);
	void SetProgressTimeInterval(DWORD dwProgressTimeInterval);
	void SetHwnd(HWND mHwnd);
	void SetCfgName(CString cfgName);
	void SetSendPlace(int nPlace);   //设置Workbook传输的位置
	
	char *GetPathName(void);
	UWORD GetSentBytes(void);
	UWORD GetFileSize(void);

	void TrigAcceptFile(DWORD dwRecvedBytes);
	void TrigRecvFileSucc(void);

	BOOL PrepareSendFile(char *szPathName);
	BOOL IsFileAccepted(void); // 检查文件是否已经接收端接受
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

// 客户端socket接收线程
class CClientSocketRecvThread: public CThread
{
private:
	CTcpClient								*m_pTcpClient;
	SOCKET									m_hSocket;
	HWND									m_hHideWnd;
	char									m_RecvBuf[NETMSG_RECVBUFSIZE];
	int										m_nRecvBufSize; // 接收缓冲区中字节数
	int                                     m_source;       //确定接受信息的来源 0 =  FILE, 1 = NVGATE

	void ForcePostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	void Execute(void);
	void DoRecv(char *Buf, int nBytes);
	void HandleRecvedNetMsg(char *Msg, int nMsgLen);
public:
	CClientSocketRecvThread(CTcpClient *pTcpClient, int nPriority, int source);
	~CClientSocketRecvThread(void);

	// get / set 函数
	void SetSocketHandle(SOCKET hSocket);
	void SetHideWndHandle(HWND hHideWnd);

};

class CGenerateModel : public CThread
{
private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // 隐藏窗体句柄
	SOCKET									m_hSocket;
	int										m_nWaitTimeOut; // 等待超时

	int                                     *m_pNumofChannel;
	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //主机的数量
	int                                     m_totalChannel; //总的通道数
	int                                     m_nCurrent;  //读取便宜点
	CString                                 iniFileName; //ini文件的路径
	CString                                 tempIniFileName;  //Tempsetting ini文件路径
	char									*bDataFromNVGate;   //NVGATE反馈的数据BYTE
	int										msgLen; //返回数据的长度

	
	CString									str_Setting;  //获取Setting的字符串
	//BOOL									m_bGetResponseFinish; //获取完成的标识符

	HWND                                    m_MainHwnd;   //主窗?

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


//操作远程NVGate线程
class COperateRemoteNVGate :
	public CThread
{

private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // 隐藏窗体句柄
	SOCKET									m_hSocket;
	int										m_nWaitTimeOut; // 等待超时

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //主机的数量

	//char									*bDataFromNVGate;   //NVGATE反馈的数据BYTE
	//int										msgLen; //返回数据的长度
	//BOOL									m_bGetResponseFinish; //获取完成的标识符
	CString                                 iniFileName;

	HWND                                    m_MainHwnd;   //主窗?

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


//开启远程NVGate线程
class COpenRemoteNVGate :
	public CThread
{
private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // 隐藏窗体句柄
	SOCKET									m_hSocket;
	int										m_nWaitTimeOut; // 等待超时

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //主机的数量
	CString                                 *m_pHardIpAddr;   //硬件的IP地址
	CString                                 *m_PHardType;     //系统类型

	HANDLE                                  m_hOpenNVGateSucc; //打开成功
	HANDLE                                  m_hOpenNVGateFail; //打开失败
	HANDLE                                  m_hOpenNVGateMute;  //主体关闭

	HWND                                    m_MainHwnd;   //主窗体句柄

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


//Alarm Recorder线程

class CAlarmSystem :
	public CThread
{

private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // 隐藏窗体句柄
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //主机的数量

	HWND                                    m_MainHwnd;   //主窗体句柄

	//int                                     m_nCurrent;  //读取便宜点
	//char									*bDataFromNVGate;   //NVGATE反馈的数据BYTE
	//int										msgLen; //返回数据的长度
	//BOOL									m_bGetResponseFinish; //获取完成的标识符
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



//Start Monitor线程

class CStartMonitor :
	public CThread
{

private:
	CTcpClient								*m_pTcpClient;
	HWND									m_hHideWnd; // 隐藏窗体句柄
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //主机的数量

	HWND                                    m_MainHwnd;   //主窗体句柄

	//int                                     m_nCurrent;  //读取便宜点
	//char									*bDataFromNVGate;   //NVGATE反馈的数据BYTE
	//int										msgLen; //返回数据的长度
	//BOOL									m_bGetResponseFinish; //获取完成的标识符
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
	HWND									m_hHideWnd; // 隐藏窗体句柄
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //主机的数量
	HWND                                    m_MainHwnd;   //主窗?

	//int                                     m_nCurrent;  //读取便宜点
	//char									*bDataFromNVGate;   //NVGATE反馈的数据BYTE
	//int										msgLen; //返回数据的长度
	//BOOL									m_bGetResponseFinish; //获取完成的标识符
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
	HWND									m_hHideWnd; // 隐藏窗体句柄
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //主机的数量
	HWND                                    m_MainHwnd;   //主?

	//int                                     m_nCurrent;  //读取便宜点
	//char									*bDataFromNVGate;   //NVGATE反馈的数据BYTE
	//int										msgLen; //返回数据的长度
	//BOOL									m_bGetResponseFinish; //获取完成的标识符
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
	HWND									m_hHideWnd; // 隐藏窗体句柄
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //主机的数量

	HWND                                    m_MainHwnd;   //主窗体句柄

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
	HWND									m_hHideWnd; // 隐藏窗体句柄
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //主机的数量

	HWND                                    m_MainHwnd;   //主窗体句柄

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
	HWND									m_hHideWnd; // 隐藏窗体句柄
	SOCKET									m_hSocket;
	int										m_nWaitTimeOut; // 等待超时

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //主机的数量

	HANDLE                                  m_hOpenNVGateSucc; //打开成功
	HANDLE                                  m_hOpenNVGateFail; //打开失败

	HWND                                    m_MainHwnd;   //主窗体句柄

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
	HWND									m_hHideWnd; // 隐藏窗体句柄
	SOCKET									m_hSocket;

	CString                                 *m_pIpAddress;
	int                                     m_numofSystem; //主机的数量

	HWND                                    m_MainHwnd;   //主窗体句柄

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


