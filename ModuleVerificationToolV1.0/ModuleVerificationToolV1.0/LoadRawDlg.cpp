// LoadRawDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ModuleVerificationToolV1.0.h"
#include "LoadRawDlg.h"
#include "afxdialogex.h"

//declare variable value
//======================
extern int width;
extern int height;
extern int pixelBits;
extern int format;
extern int bpattern;

// CLoadRawDlg 对话框

IMPLEMENT_DYNAMIC(CLoadRawDlg, CDialogEx)

CLoadRawDlg::CLoadRawDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLoadRawDlg::IDD, pParent)
	, m_width(0)
	, m_height(0)
	, m_bayerpattern(0)
{

	//  m_format = 0;
}

CLoadRawDlg::~CLoadRawDlg()
{
}

void CLoadRawDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_WIDTH, m_width);
	DDV_MinMaxInt(pDX, m_width, 0, 10000);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_height);
	DDV_MinMaxInt(pDX, m_height, 0, 10000);
	//  DDX_CBIndex(pDX, IDC_COMBO_PIXELBITS, m_pixelBits);
	//  DDV_MinMaxInt(pDX, m_pixelBits, 0, 3);
	//  DDX_CBIndex(pDX, IDC_COMBO_FORMAT, m_format);
	//  DDV_MinMaxInt(pDX, m_format, 0, 2);
	DDX_Control(pDX, IDC_COMBO_FORMAT, m_format);
	DDX_Control(pDX, IDC_COMBO_PIXELBITS, m_pixelBits);
}


BEGIN_MESSAGE_MAP(CLoadRawDlg, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO_RGGB, &CLoadRawDlg::OnBnClickedRadioRggb)
	ON_BN_CLICKED(IDC_RADIO_BGGR, &CLoadRawDlg::OnBnClickedRadioBggr)
	ON_BN_CLICKED(IDC_RADIO_GRBG, &CLoadRawDlg::OnBnClickedRadioGrbg)
	ON_BN_CLICKED(IDC_RADIO_GBRG, &CLoadRawDlg::OnBnClickedRadioGbrg)
	ON_BN_CLICKED(IDOK, &CLoadRawDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT_WIDTH, &CLoadRawDlg::OnEnChangeEditWidth)
	ON_EN_CHANGE(IDC_EDIT_HEIGHT, &CLoadRawDlg::OnEnChangeEditHeight)
	ON_CBN_SELCHANGE(IDC_COMBO_PIXELBITS, &CLoadRawDlg::OnCbnSelchangeComboPixelbits)
	ON_CBN_SELCHANGE(IDC_COMBO_FORMAT, &CLoadRawDlg::OnCbnSelchangeComboFormat)
END_MESSAGE_MAP()


// CLoadRawDlg 消息处理程序


void CLoadRawDlg::OnBnClickedRadioRggb()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bayerpattern = 0;
	bpattern = 1;
	//((CButton*)GetDlgItem(IDC_RADIO_RGGB))->SetCheck(TRUE);
	//((CButton*)GetDlgItem(IDC_RADIO_BGGR))->SetCheck(FALSE);
	//((CButton*)GetDlgItem(IDC_RADIO_GRBG))->SetCheck(FALSE);
	//((CButton*)GetDlgItem(IDC_RADIO_GBRG))->SetCheck(FALSE);
}


void CLoadRawDlg::OnBnClickedRadioBggr()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bayerpattern = 1;
	bpattern = 1;
}


void CLoadRawDlg::OnBnClickedRadioGrbg()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bayerpattern = 2;
	bpattern = 2;
}


void CLoadRawDlg::OnBnClickedRadioGbrg()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bayerpattern = 3;
	bpattern = 3;
}


void CLoadRawDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();

}


void CLoadRawDlg::OnEnChangeEditWidth()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData( TRUE );
	width = m_width;
	UpdateData( FALSE );
	TRACE("m_widh = %d\n", m_width);

}


void CLoadRawDlg::OnEnChangeEditHeight()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData( TRUE );
	height = m_height;
	UpdateData( FALSE );
	TRACE("m_height = %d\n", m_height);

}


void CLoadRawDlg::OnCbnSelchangeComboPixelbits()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData( TRUE );
	switch( m_pixelBits.GetCurSel() )
	{
		case 0:
			pixelBits = 8;
			break;
		case 1:
			pixelBits = 10;
			break;
		default:
			pixelBits = 12;
	}
	TRACE("pixelBites = %d\n", pixelBits);
	UpdateData( FALSE );
}


void CLoadRawDlg::OnCbnSelchangeComboFormat()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData( TRUE );
	switch( m_format.GetCurSel() )
	{
		case 0:
			format = 0;				//Mipi raw
			TRACE("format = Mipi\n");
			break;
		case 1:
			format = 1;				//Packed raw
			TRACE("format = Packed\n");
			break;
		default:
			format = 2;				//Unpacked raw
			TRACE("format = Unpacked\n");
	}
	UpdateData( FALSE );
}
