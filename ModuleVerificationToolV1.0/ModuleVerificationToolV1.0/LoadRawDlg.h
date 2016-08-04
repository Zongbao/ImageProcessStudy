#pragma once


// CLoadRawDlg �Ի���

class CLoadRawDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLoadRawDlg)

public:
	CLoadRawDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CLoadRawDlg();

// �Ի�������
	enum { IDD = IDD_DIALOG_LOADRAW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	// The width of the raw image
	int m_width;
	// The height of the image
	int m_height;
	// The pixel bits of the image
//	int m_pixelBits;
	int m_bayerpattern;
	afx_msg void OnBnClickedRadioRggb();
	afx_msg void OnBnClickedRadioBggr();
	afx_msg void OnBnClickedRadioGrbg();
	afx_msg void OnBnClickedRadioGbrg();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeEditWidth();
	afx_msg void OnEnChangeEditHeight();
	afx_msg void OnCbnSelchangeComboPixelbits();
//	int m_format;
	afx_msg void OnCbnSelchangeComboFormat();
	CComboBox m_format;
	CComboBox m_pixelBits;
};
