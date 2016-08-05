
/***********************************************************************************
//Program:
//========
//	This program is used to do the module verification
//	Currently, it is just a demo
//	I will try my best to fix it well
//History:
//========
//	2016/06/19	Myron	First release
//	2016/07/27			Add Optical Center Verification
/***********************************************************************************/
// ModuleVerificationToolV1.0Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "ModuleVerificationToolV1.0.h"
#include "ModuleVerificationToolV1.0Dlg.h"
#include "afxdialogex.h"

//add header file
//===============
#include "LoadRawDlg.h"
#include "MyronFunctionLib.h"

//declare variable value
//======================
int width;
int height;
int pixelBits;
int format;
int bpattern;
extern unsigned char *rawBuf;
extern unsigned char *rgbBuf;
BITMAPINFO* pBmpInfo;
extern BITMAPINFOHEADER infohead;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CModuleVerificationToolV10Dlg dialog




CModuleVerificationToolV10Dlg::CModuleVerificationToolV10Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CModuleVerificationToolV10Dlg::IDD, pParent)
	, m_edit_oc(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CModuleVerificationToolV10Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_OC, m_edit_oc);
}

BEGIN_MESSAGE_MAP(CModuleVerificationToolV10Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_FILE_OPEN, &CModuleVerificationToolV10Dlg::OnFileOpen)
	ON_BN_CLICKED(IDC_BUTTON_OC, &CModuleVerificationToolV10Dlg::OnClickedButtonOc)
END_MESSAGE_MAP()


// CModuleVerificationToolV10Dlg message handlers

BOOL CModuleVerificationToolV10Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	//add menubar
	//==========

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CModuleVerificationToolV10Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CModuleVerificationToolV10Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CModuleVerificationToolV10Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//Menubar function
//FileOpen
//==============================================
void CModuleVerificationToolV10Dlg::OnFileOpen()
{
	// TODO: 在此添加命令处理程序代码

	//Open a raw file
	//===============
	CFileDialog RawDlg( TRUE,	//TRUE means read
						NULL, NULL,
						OFN_HIDEREADONLY,
						L"ALL Files(*.*) || *.bmp || *.raw");
	RawDlg.m_ofn.lpstrTitle = L"Load an image";

	CString strPathName;
	CString strFileName;	

	
	if( IDOK != RawDlg.DoModal())
	{
		TRACE("Open failed!\n");
		return;
	}
		
	else
		TRACE("Open succeed!\n");

	strFileName = RawDlg.GetFileName();
	strPathName = RawDlg.GetPathName();

	CLoadRawDlg loadRawDlg;
	loadRawDlg.DoModal();

	TRACE("width  = %d\n", width);
	TRACE("height = %d\n", height);

	switch( format )
	{
	case 0:
		if( pixelBits == 8 )
		{
			ReadMipi8bitRaw( strPathName, width, height );
			TRACE("This format is not suppored! \n");
		}
		
		else if( pixelBits == 10 )
			ReadMipi10bitRaw( strPathName, width, height );
//			ReadUnpackedRaw( strPathName, width, height );
		else
			TRACE("This format is not supported!\n");
		break;
	case 1:
		if( pixelBits != 12 )
			ReadQualcommPacked10bitRaw( strPathName, width, height );
		else
			TRACE("This format is not supported!\n");
		break;
	case 2:
		if( pixelBits == 8 )
		{
			ReadMipi8bitRaw( strPathName, width, height );
			TRACE("This format is unpacked 8bit raw! \n");			
		}
		else if( pixelBits == 10 )
		{
			ReadUnpackedRaw( strPathName, width, height );
			TRACE("This format is unpacked 10bit! \n");
		}
		else
			TRACE("This format is unpacked 10bit!\n");
		break;
	default:
		TRACE("This format is not supported!\n");
		break;
	}
	Demosaic_raw_to_rgb( rawBuf, width, height, bpattern );
	CString bmpfile ("result.bmp", 10);
	SaveBmp( bmpfile, rgbBuf, width, height, 24);

	//Show the image
	/****************************************************************************************/
	pBmpInfo = (BITMAPINFO*) new char[ sizeof(BITMAPINFOHEADER) ];
	memcpy(pBmpInfo, &infohead, sizeof(BITMAPINFOHEADER));
	CWnd *pWnd = GetDlgItem(IDC_STATIC_IMAGE);		//获得picture控件窗口的句柄
	CRect rect;									//定义一个矩形，用于描述控件矩形区域
	pWnd->GetClientRect( &rect);				//获得picture控件所在的矩形区域
	CDC *pDC = pWnd->GetDC();					//获得picture控件的DC
	pDC->SetStretchBltMode(COLORONCOLOR);

	StretchDIBits(pDC->GetSafeHdc(), 
					0, 0, rect.Width(), rect.Height(), 
					0, 0, infohead.biWidth, infohead.biHeight, 
					rgbBuf, pBmpInfo,
					DIB_RGB_COLORS, SRCCOPY);
	/****************************************************************************************/
	delete[]pBmpInfo;
}



void CModuleVerificationToolV10Dlg::OnClickedButtonOc()
{
	// TODO: 在此添加控件通知处理程序代码
	if( rawBuf != 0 )
	{
//		TRACE("The OC_THRESHOLD is: %d\n", OC_THRESHOLD );
//		float oc_dis=0;
		m_edit_oc = OC_Test( rgbBuf, width, height );
		UpdateData( FALSE );
	}
		
	else
		TRACE("Please input a raw image!!!\n");
}
