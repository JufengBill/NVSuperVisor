// InputBox.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "NVSuperVisor.h"
#include "InputBox.h"
#include "afxdialogex.h"


// CInputBox �Ի���

IMPLEMENT_DYNAMIC(CInputBox, CDialogEx)

CInputBox::CInputBox(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInputBox::IDD, pParent)
	, m_Input(_T(""))
{

}

CInputBox::~CInputBox()
{
}

void CInputBox::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_Input);
}


BEGIN_MESSAGE_MAP(CInputBox, CDialogEx)
END_MESSAGE_MAP()


// CInputBox ��Ϣ�������
