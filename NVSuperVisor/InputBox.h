#pragma once
#include "afxwin.h"


// CInputBox 对话框

class CInputBox : public CDialogEx
{
	DECLARE_DYNAMIC(CInputBox)

public:
	CInputBox(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CInputBox();

// 对话框数据
	enum { IDD = IDD_INPUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	CString m_Input;
};
