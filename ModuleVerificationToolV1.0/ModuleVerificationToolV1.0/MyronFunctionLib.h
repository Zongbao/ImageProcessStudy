/*********************************************************************/
//Program:
//	Myron Image Process Function lib.h
//	This lib file contain some necessary function which can help you 
//	read the image file or do other related function.
//History:
//	2016/05/27	Myron	First release
/*********************************************************************/
#ifndef MYRON_FUNCTIONLIB_H_
#define MYRON_FUNCTIONLIB_H_

#include "stdafx.h"
#include "ModuleVerificationToolV1.0.h"

void ReadQualcommPacked10bitRaw( CString fileName, int inwidth, int inheight );
void ReadMipi10bitRaw( CString fileName, int inwidth, int inheight );
void ReadMipi8bitRaw( CString fileName, int inwidth, int inheight );
void ReadUnpackedRaw( CString fileName, int inwidth, int inheight );

bool Demosaic_RGGB( unsigned char *inBuf, int inwidth, int inheight );
bool Demosaic_BGGR( unsigned char *inBuf, int inwidth, int inheight );
bool Demosaic_GRBG( unsigned char *inBuf, int inwidth, int inheight );
bool Demosaic_GBRG( unsigned char *inBuf, int inwidth, int inheight );
bool Demosaic_raw_to_rgb( unsigned char *inBuf, int inwidth, int inheight, int pattern );

bool SaveBmp(CString sFileName, unsigned char *bmpData, int inwidth, int inheight, int bitCount);
double OC_Test( unsigned char* in_rgbBuf, int inwidth, int inheight);

#endif
