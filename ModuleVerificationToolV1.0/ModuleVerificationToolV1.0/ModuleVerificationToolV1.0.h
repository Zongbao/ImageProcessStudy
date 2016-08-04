
// ModuleVerificationToolV1.0.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CModuleVerificationToolV10App:
// See ModuleVerificationToolV1.0.cpp for the implementation of this class
//

class CModuleVerificationToolV10App : public CWinApp
{
public:
	CModuleVerificationToolV10App();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CModuleVerificationToolV10App theApp;