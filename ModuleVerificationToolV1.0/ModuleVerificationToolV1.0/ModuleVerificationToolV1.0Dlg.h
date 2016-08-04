
// ModuleVerificationToolV1.0Dlg.h : header file
//

#pragma once


// CModuleVerificationToolV10Dlg dialog
class CModuleVerificationToolV10Dlg : public CDialogEx
{
// Construction
public:
	CModuleVerificationToolV10Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MODULEVERIFICATIONTOOLV10_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileOpen();
	afx_msg void OnClickedButtonOc();
	// Record the result of oc test
	float m_edit_oc;
};
