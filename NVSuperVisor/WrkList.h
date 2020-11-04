#pragma once
#include "afxwin.h"
#include "IniFileReadWrite.h"
#include "CommonFunction.h"


// CWrkList �Ի���

class CWrkList : public CDialogEx
{
	DECLARE_DYNAMIC(CWrkList)

public:
	CWrkList(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CWrkList();

// �Ի�������
	enum { IDD = IDD_WORKBOOKLIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_Listbox_Workbook;

	int m_nCount;
	int m_nSel;
	CString *m_nSelText;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnClose();

	void SetPlace(int nPlace);

private:
	int m_nPlace;
	CIniFileReadWrite *m_pIniFileReadWrite;
	CCommonFunction *m_pCommonFunction;

};
