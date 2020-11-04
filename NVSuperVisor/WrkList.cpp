// WrkList.cpp : 实现文件
//

#include "stdafx.h"
#include "NVSuperVisor.h"
#include "WrkList.h"
#include "afxdialogex.h"
#include <string>

using namespace std;




// CWrkList 对话框

IMPLEMENT_DYNAMIC(CWrkList, CDialogEx)

CWrkList::CWrkList(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWrkList::IDD, pParent)
{

	m_nCount = 0;
	m_nSel = 0;
	m_nSelText =new CString();
	m_pIniFileReadWrite = new CIniFileReadWrite();
	m_pCommonFunction = new CCommonFunction();

}

CWrkList::~CWrkList()
{
	delete m_nSelText;
	delete m_pIniFileReadWrite;
	delete m_pCommonFunction;
}

void CWrkList::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_Listbox_Workbook);
}


BEGIN_MESSAGE_MAP(CWrkList, CDialogEx)
	ON_BN_CLICKED(IDOK, &CWrkList::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CWrkList::OnBnClickedCancel)
	ON_WM_CLOSE()
END_MESSAGE_MAP()




BOOL CWrkList::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_nCount = 0;
	m_nSel = 0;

	//获取指定WORKBOOK路径下的.wkb文件，并显示在WKList中
	CString wkFolderString = m_pIniFileReadWrite->GetCurrentPath();
	m_pIniFileReadWrite->SetPathFileName(wkFolderString + _T("custom.ini"));
	m_pIniFileReadWrite->ReadValue(_T("Parameters"), _T("WorkbookFolder"), wkFolderString);

	string stringWKName(wkFolderString.GetBuffer());
	vector<string> wkbFile = m_pCommonFunction->FindFiles(stringWKName);
	CString tempStr = _T("");
	for (string afile : wkbFile)
	{
		tempStr.Format(_T("%s"), afile.c_str());
		m_Listbox_Workbook.AddString(tempStr);
	}

	m_Listbox_Workbook.SetCurSel(0);

		//m_nCount = m_Listbox_Workbook.GetCount();
		//m_nSel = m_Listbox_Workbook.GetSelCount();
		//m_Listbox_Workbook.GetText(m_nSel, m_nSelText);

	////获取当前的应用程序路径名，确定INI文件的完整文件名
	//CString strAppPath = _T("");
	//CString tempIniFileName = _T("");
	//CString sectionName = _T("");
	//GetModuleFileName(NULL, strAppPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	//strAppPath.ReleaseBuffer();
	//int nPos = strAppPath.ReverseFind('\\');
	//strAppPath = strAppPath.Left(nPos);
	//tempIniFileName.Format(_T("%s\\Tempsetting.ini"), strAppPath);

	//// TODO:  在此添加额外的初始化
	//char strINIInfo[64] = { "" };
	////读取Configuration Number
	//GetPrivateProfileString(_T("Workbook"), _T("Number"), _T("0"), strINIInfo, 64, tempIniFileName);
	//int m_nWorkbookNumber = atoi(strINIInfo);

	//if (m_nWorkbookNumber > 0)
	//{
	//	for (int i = 0; i < m_nWorkbookNumber; i++)
	//	{
	//		//读取Status
	//		sectionName.Format(_T("Status%d"), i + 1);
	//		GetPrivateProfileString(_T("Workbook"), sectionName, _T(""), strINIInfo, 64, tempIniFileName);
	//		CString strPlace;
	//		strPlace.Format(_T("%d"), m_nPlace);

	//		if (_T(strINIInfo) == strPlace)
	//		{
	//			//读取Configuration Name
	//			sectionName.Format(_T("Workbook%d"), i + 1);
	//			GetPrivateProfileString(_T("Workbook"), sectionName, _T(""), strINIInfo, 64, tempIniFileName);
	//			m_Listbox_Workbook.AddString(_T(strINIInfo));
	//		}

	//		////读取Configuration Name
	//		//sectionName.Format(_T("Workbook%d"), i + 1);
	//		//GetPrivateProfileString(_T("Workbook"), sectionName, _T(""), strINIInfo, 64, tempIniFileName);
	//		//m_Listbox_Workbook.AddString(_T(strINIInfo));
	//	}

	//	m_Listbox_Workbook.SetCurSel(0);
	//	//m_nCount = m_Listbox_Workbook.GetCount();
	//	//m_nSel = m_Listbox_Workbook.GetSelCount();
	//	//m_Listbox_Workbook.GetText(m_nSel, m_nSelText);
	//}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CWrkList::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码
	CDialogEx::OnOK();

	m_nCount = m_Listbox_Workbook.GetCount();
	m_nSel = m_Listbox_Workbook.GetCurSel();
	if (m_nSel >=0)
	{
		m_Listbox_Workbook.GetText(m_nSel, *m_nSelText);
	}
	//*m_nSelText = m_Listbox_Workbook.GetSelItems.GetText();
	
}

void CWrkList::OnBnClickedCancel()
{
	// TODO:  在此添加控件通知处理程序代码
	CDialogEx::OnCancel();

	m_nCount = 0;
	m_nSel = 0;
	*m_nSelText = _T("");
}

void CWrkList::OnClose()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnClose();
	m_nCount = 0;
	m_nSel = 0;
	*m_nSelText = _T("");
}

void CWrkList::SetPlace(int nPlace)
{
	m_nPlace = nPlace;
}
