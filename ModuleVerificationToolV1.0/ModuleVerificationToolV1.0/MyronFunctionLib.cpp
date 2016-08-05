/*********************************************************************
//Program:
//	This cpp file is a detail instruction about all the function used 
//	in the main exe. I manage all of them here and make it convenient
//	to do some change.
//History:
//	2016/05/25	Myron	First Release
//	2016/07/27			Add Optical Center Verification
//	2016/08/04			Add MTK unpacked Raw decode
*********************************************************************/
#include "stdafx.h"
#include "MyronFunctionLib.h"
#include "malloc.h"
#include "math.h"

//#define OC_THRESHOLD 180
int OC_THRESHOLD = 0;
unsigned char *rawBuf;
unsigned char *rgbBuf;
unsigned char *Y_Channel;
BITMAPFILEHEADER fileHead;
BITMAPINFOHEADER infohead;


//Read Qualcomm Packed 10bit Raw
//==============================
//	CString: the filename or pathname
//	inwidth: the width of the image
//	inheight:the height of the image
//ATTENTION:
//	THIS FORMAT MEANS 6 PIXELS IN 8 BYTE AND 8 BYTE ALIGNMENT
//	IF YOU WANT TO TRANSFER IT TO 8BIT, CUT FIRST 2 BITS
//	FOR EXAMPLE: 
//	a1 a2 a3 a4 a5 a6 a7 a8 a9 a10 b1 b2 b3 b4 b5 b6 b7 b8 b9 b10 c1 c2 c3 c4 c5 c6 c7 c8 c9 c10 ...
//	======================= ------------------------ ======================== ------------------ ...
//Read step:
//1.load the raw data
//2.translate 10bit to 8bit
//
void ReadQualcommPacked10bitRaw( CString fileName, int inwidth, int inheight)
{	
	unsigned char *temp_original_10bit, *temp_result_8bit;
	int Byte_Cnt;
	int packed_cnt;
	int pixel_per_line;
	pixel_per_line = (inwidth + 5) / 6 * 6;	//4208 -> 4212
	Byte_Cnt = (inwidth + 5) / 6 * 8;		//calculate the bytes per line ( 16bytes alignment )	//4208 10bit pixel -> 5616 bytes
	TRACE("Byte_Cnt = %3d\n", Byte_Cnt);
	temp_original_10bit = new unsigned char [ Byte_Cnt * inheight ];

	//save temp data ( the data read from the original raw )
	//======================================================
	CFile rawFile;
	rawFile.Open( fileName, CFile::modeRead | CFile::typeBinary );
	rawFile.Read( temp_original_10bit, Byte_Cnt*inheight );
	rawFile.Close();  
	TRACE("Get 10bit raw data successfully!\n");

	temp_result_8bit = new unsigned char [ pixel_per_line * inheight];
	int width_cnt = Byte_Cnt / 8 ;		//16 -> 0 & 1
	int m = 0;
	for( int i = 0; i < inheight; ++ i )
		for( int j = 0; j < width_cnt; ++ j )
		{
			//pixel -> byte
			//first pixel
			//===========
			temp_result_8bit[6*m] = (temp_original_10bit[i*Byte_Cnt+8*j]>>2) | ((temp_original_10bit[i*Byte_Cnt+8*j+1]&0x03)<<6) ;
			//second pixel
			//============
			//ATTENTION: ( A & 3 << 6 ) !=  ( A & 3 ) << 6
			//ATTENTION: i*inheight(x) ---> i*Byte_Cnt
			temp_result_8bit[6*m+1] = (temp_original_10bit[i*Byte_Cnt+8*j+1]>>4) | ((temp_original_10bit[i*Byte_Cnt+8*j+2]&0x0f)<<4);
			//third pixel
			//===========
			temp_result_8bit[6*m+2] = (temp_original_10bit[i*Byte_Cnt+8*j+2]>>6) | ((temp_original_10bit[i*Byte_Cnt+8*j+3]&0x3f)<<2);
			//Forth pixel
			//===========
			temp_result_8bit[6*m+3] = temp_original_10bit[i*Byte_Cnt+8*j+4];
			//Fifth pixel
			//===========
			temp_result_8bit[6*m+4] = (temp_original_10bit[i*Byte_Cnt+8*j+5]>>2) | ((temp_original_10bit[i*Byte_Cnt+8*j+6]&0x03)<<6) ;
			//Sixth pixel
			temp_result_8bit[6*m+5] = (temp_original_10bit[i*Byte_Cnt+8*j+6]>>4) | ((temp_original_10bit[i*Byte_Cnt+8*j+7]&0x0f)<<4);
			m++;			
		}
		rawBuf = new unsigned char [ inwidth* inheight ];
		for(int m = 0, i = 0; i < inheight; ++i )
			for(int j =0; j < pixel_per_line-4; ++j )
			{
				rawBuf[ m ] = temp_result_8bit[ pixel_per_line * i + j];	
				m++;
			}
			
	TRACE("Translate 10bit to 8bit succeed!\n");
	TRACE("Read Packed10bit Raw succeed!\n");
	TRACE("================================\n\n");
	return;
}

//ReadMipi8bitRaw
//==============================
//	CString: the filename or pathname
//	inwidth: the width of the image
//	inheight:the height of the image
void ReadMipi8bitRaw( CString fileName, int inwidth, int inheight )
{
	
	CFile rawFile;
	rawFile.Open(fileName, CFile::modeRead | CFile::typeBinary );
	rawBuf = new unsigned char( inheight * inwidth );
	rawFile.Read(rawBuf, inheight*inwidth);
	rawFile.Close();

	TRACE("Read Packed10bit Raw succeed!\n");
	TRACE("================================\n\n");

	return;
}

//ReadMipi10bitRaw
//==============================
//	CString: the filename or pathname
//	inwidth: the width of the image
//	inheight:the height of the image
void ReadMipi10bitRaw( CString fileName, int inwidth, int inheight )
{
	//MIPI Raw format: 4 pixel in 5 byte, so you can directly delete the fifth byte which stored all the pixel data
	int pixel_per_line = ( inwidth + 3 ) / 4 * 4;	//pixel per line
	int lineByte =  ( inwidth + 3 ) / 4 * 5;		//5 byte alignment		//4208+3=4211//4211/4=1052//1052*5=5260
	lineByte = (lineByte+7)/8*8;
	int packed = lineByte / 5.0;												//5260/5=1052
	int resultlineByte = (inwidth + 3)/4*4;			//4211

	CFile rawFile;
	rawFile.Open( fileName, CFile::modeRead | CFile::typeBinary );
	
	unsigned char *temp_original_10bit;
	unsigned char *temp_result_8bit;
	temp_original_10bit = new unsigned char [ lineByte * inheight ];
	temp_result_8bit = new unsigned char [ resultlineByte * inheight ];
	rawBuf = new unsigned char [ inwidth*inheight ];

	rawFile.Read( temp_original_10bit, lineByte*inheight );
	rawFile.Close();

	for( int m = 0,i = 0; i < inheight; ++i )
		for( int j = 0; j < packed; ++j )
		{
			temp_result_8bit[ 4*m + 0 ] = temp_original_10bit[ i * lineByte + 5*j + 0 ];
			temp_result_8bit[ 4*m + 1 ] = temp_original_10bit[ i * lineByte + 5*j + 1 ];
			temp_result_8bit[ 4*m + 2 ] = temp_original_10bit[ i * lineByte + 5*j + 2 ];
			temp_result_8bit[ 4*m + 3 ] = temp_original_10bit[ i * lineByte + 5*j + 3 ];
			m++;
		}
	TRACE("Translate 10bit to 8bit succeed!\n");
	//save the 8bit raw data to rawBuf;
	for( int i = 0; i < inheight; ++ i )
		for( int j = 0; j < inwidth; ++j )
		{
			rawBuf[ i*inwidth + j ] = temp_result_8bit[ i*packed*4+j ];
		}
	TRACE("ReadMipi10bitRaw succeed!\n");
	TRACE("==================\n\n");
}


//Myron add
//20160804
//Read MTK unpacked 10bit Raw
//==================================================================
void ReadUnpackedRaw( CString fileName, int inwidth, int inheight )
{
	//Raw format: 1 pixel in 2 byte, 2bytes alignment ??
	int lineByte = inwidth * 2;
	int resultlineByte = inwidth;
	CFile rawFile;
	rawFile.Open( fileName, CFile::modeRead | CFile::typeBinary );
	unsigned char *temp_original_10bit;
	unsigned char *temp_result_8bit;

	temp_original_10bit = new unsigned char [ lineByte * inheight ];
	temp_result_8bit = new unsigned char [ resultlineByte * inheight ];
	rawBuf = new unsigned char [ inwidth*inheight ];

	rawFile.Read( temp_original_10bit, lineByte*inheight );
	rawFile.Close();

	for( int m = 0,i = 0; i < inheight; ++i )
		for( int j = 0; j < lineByte/2; ++j )
		{
			temp_result_8bit[m] = (temp_original_10bit[i*lineByte+2*j]>>2) | ((temp_original_10bit[i*lineByte+2*j+1]&0x03)<<6) ;
			m++;
		}
	TRACE("Translate 10bit to 8bit succeed!\n");
	//save the 8bit raw data to rawBuf;
	for( int i = 0; i < inheight; ++ i )
		for( int j = 0; j < inwidth; ++j )
		{
			rawBuf[ i*inwidth + j ] = temp_result_8bit[ i*inwidth+j ];
		}
	TRACE("ReadUnpacked10bitRaw succeed!\n");
	TRACE("==================\n\n");

}

bool Demosaic_RGGB( unsigned char *inBuf, int inwidth, int inheight)	//bpattern = 0
{
	//ATTENTION: WATCH OUT THE SIZE HERE	
	int sum = inwidth * inheight;
	unsigned char *temp;
	temp = new unsigned char [ sum * 3 ];
	rgbBuf = new unsigned char [ sum * 3 ];
	//calculate the size of the 8bit raw

	//先处理四条边的数据
	//处理第一行
	for( int i = 0; i < inwidth; i ++ )				
	{												
		if( i == 0 )		//第一行，第一列，R
		{		
			temp[ 3 * i ]     = inBuf[ i ];									//10
			temp[ 3 * i + 1 ] = 1.0/2.0 * ( inBuf[ i + 1 ] + inBuf[inwidth + i ]);	//90
			temp[ 3 * i + 2 ] = inBuf[ inwidth + i + 1];						//170
		}
		else if( i!=0 && ( i % 2 == 0 ) )	//第一行，偶数列，R
		{
			temp[ 3 * i ]     = inBuf[ i ];	
			temp[ 3 * i + 1 ] = 1.0/3.0 * ( inBuf[ i -1 ]+ inBuf[ i + 1 ] + inBuf[inwidth + i ]);
			temp[ 3 * i + 2 ] = 1.0/2.0 * ( inBuf[ inwidth + i - 1] + inBuf[ inwidth + i + 1]);						
		}
		else if( i == inwidth -1 )			//第一行，最后一列，Gr
		{
			temp[ 3 * i ]     = inBuf[ i - 1];	
			temp[ 3 * i + 1 ] = inBuf[ i ];		
			temp[ 3 * i + 2 ] = inBuf[ inwidth + i ];								
		}
		else								//第一行，奇数列，Gr
		{
			temp[ 3 * i ]     = 1.0/2.0 * ( inBuf[ i - 1 ] + inBuf[ i + 1 ]);	//R~ = R11
			temp[ 3 * i + 1 ] = inBuf[ i ];								//G~= G
			temp[ 3 * i + 2 ] = inBuf[ inwidth + i ];						//B~ = B						
		}
	}
	//处理最后一行
	for( int i = 0; i < inwidth; i ++ )
	{
		if( i == 0 )		//最后一行，第一列,R
		{		
			temp[ inwidth *3*(inheight-1) + 3*i ]     = inBuf[ inwidth*(inheight-1)+ i - inwidth ];	//R~ = R11
			temp[ inwidth *3*(inheight-1) + 3*i + 1 ] = inBuf[ inwidth*(inheight-1)+ i ];			//G~ = G
			temp[ inwidth *3*(inheight-1) + 3*i + 2 ] = inBuf[ inwidth*(inheight-1)+ i + 1];			//B~ = B22			
		}
		else if( i!=0 && ( i % 2 == 0 ) )	//最后一行，偶数列Gb
		{
			temp[ inwidth *3*(inheight-1) +3*i ]     = inBuf[ inwidth*(inheight-1)+ i - inwidth ];	
			temp[ inwidth *3*(inheight-1) +3*i + 1 ] = inBuf[ inwidth*(inheight-1)+ i ];			
			temp[ inwidth *3*(inheight-1) +3*i + 2 ] = 1.0/2.0 * ( inBuf[ inwidth*(inheight-1)+ i - 1] + inBuf[ inwidth*(inheight-1)+ i + 1] );						
		}
		else if( i == inwidth -1 )		//最后一行，最后一列,B
		{
			temp[ inwidth *3*(inheight-1) +3*i ]     = inBuf[ inwidth*(inheight-1)+ i - inwidth - 1];	
			temp[ inwidth *3*(inheight-1) +3*i + 1 ] = 1.0/2.0 * (inBuf[inwidth*(inheight-1)-inwidth + i ] + inBuf[ inwidth*(inheight-1)+ i - 1]);		
			temp[ inwidth *3*(inheight-1) +3*i + 2 ] = inBuf[ inwidth*(inheight-1)+ i ];									
		}
		else									//最后一行，奇数列，B
		{
			temp[ inwidth *3*(inheight-1) +3*i ]     = 1.0/2.0 * ( inBuf[ inwidth*(inheight-1)+ i - inwidth - 1]+inBuf[inwidth*(inheight-1)+ i-inwidth + 1]);								//R~ = R11
			temp[ inwidth *3*(inheight-1) +3*i + 1 ] = 1.0/3.0 * (  inBuf[ inwidth*(inheight-1)+ i - inwidth ] + inBuf[inwidth*(inheight-1)+ i - 1 ] + inBuf[ inwidth*(inheight-1)+ i + 1 ] );			//G~= G
			temp[ inwidth *3*(inheight-1) +3*i + 2 ] = inBuf[ inwidth*(inheight-1)+ i ];										//B~ = B						
		}
	}
	//处理第一列
	for( int i = 1; i < inheight - 1; i ++ )
	{
		if( i % 2 == 1)	//第一列，奇数行，Gb
		{
			temp[ inwidth *3 * i ]     = 1.0/2.0*(inBuf[ inwidth * ( i  - 1 )] + inBuf[ inwidth * ( i  + 1 )]);								//R~ = R11
			temp[ inwidth *3 * i + 1 ] = inBuf[ inwidth * i ];			//G~= G
			temp[ inwidth *3 * i + 2 ] = inBuf[ inwidth * i + 1];										//B~ = B						
		}
		else			//第一列, 偶数行，R
		{
			temp[ inwidth *3 * i ]     = inBuf[ inwidth * i ];								//R~ = R11
			temp[ inwidth *3 * i + 1 ] = 1.0/3.0 * ( inBuf[inwidth*(i-1)]+inBuf[inwidth*i+1]+inBuf[inwidth*(i+1)]);			//G~= G
			temp[ inwidth *3 * i + 2 ] = 1.0/2.0 * ( inBuf[inwidth * (i -1) + 1 ]+inBuf[inwidth*(i+1) +1]);										//B~ = B						
		}
	}
	//处理最后一列
	for( int i = 1; i < inheight - 1; i ++ )
	{
		if( i % 2 != 0)	//最后一列，奇数行，B
		{
			temp[ inwidth *3 * i ]     = 1.0/2.0*(inBuf[ inwidth * i - 2] + inBuf[ inwidth * ( i  + 2 ) - 2]);	//R~ = R11
			temp[ inwidth *3 * i + 1 ] = 1.0/3.0*(inBuf[ inwidth * i-1]+inBuf[inwidth*(i+1)-2]+inBuf[inwidth*(i+2)-1]);//G~= G
			temp[ inwidth *3 * i + 2 ] = inBuf[ inwidth * (i + 1)-1];										//B~ = B						
		}
		else			//最后一列，偶数行，Gr
		{
			temp[ inwidth *3 * i ]     = inBuf[ inwidth * (i+1)-2];								
			temp[ inwidth *3 * i + 1 ] = inBuf[ inwidth * (i+1)-1];			
			temp[ inwidth *3 * i + 2 ] = 1.0/2.0 * ( inBuf[ inwidth * (i)-1 ]+inBuf[inwidth*(i+2)-1]);										//B~ = B						
		}
	
	}
	//处理中间部分
	for( int i = 1; i < inheight-1; i ++ )
		for( int j = 1; j < inwidth-1; j ++ )
		{
			if( i %2 != 0)
			{
				if( j %2 != 0)	//B Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = 1.0/4.0 * (inBuf[inwidth*(i-1)+j-1]+inBuf[inwidth*(i-1)+j+1]+inBuf[inwidth*(i+1)+j-1]+inBuf[inwidth*(i+1)+j+1]);	//R~ = R1+R2+R3+R4
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = 1.0/4.0 * (inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]+inBuf[inwidth*(i-1)+j]+inBuf[inwidth*(i+1)+j]);
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = inBuf[inwidth*i+j];
				}
				else			//Gb Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = 1.0/2.0 * (inBuf[inwidth*(i-1)+j]+inBuf[inwidth*(i+1)+j]);	
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = inBuf[inwidth*i+j];
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = 1.0/2.0 * (inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]);
						
				}
			}
			else
			{
				if( j %2 != 0)	//Gr Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = 1.0/2.0 * (inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]);	
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = inBuf[inwidth*i+j];
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = 1.0/2.0 * (inBuf[inwidth*(i-1)+j]+inBuf[inwidth*(i+1)+j]);
				}
				else			//R Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = inBuf[inwidth*i+j];//	
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = 1.0/4.0 * (inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]+inBuf[inwidth*(i-1)+j]+inBuf[inwidth*(i+1)+j]);
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = 1.0/4.0 * (inBuf[inwidth*(i-1)+j-1]+inBuf[inwidth*(i-1)+j+1]+inBuf[inwidth*(i+1)+j-1]+inBuf[inwidth*(i+1)+j+1]);
				}
			}
		}

	for(int i = 0; i < inheight; ++i )
		for( int j = 0; j < inwidth ; ++j)
		{
			rgbBuf[ ( inwidth * 3 * i ) + 3*j]   = temp[( inwidth * 3 * i ) + 3*j];		//R
			rgbBuf[ ( inwidth * 3 * i ) + 3*j+1] = temp[( inwidth * 3 * i ) + 3*j+1];	//G
			rgbBuf[ ( inwidth * 3 * i ) + 3*j+2] = temp[( inwidth * 3 * i ) + 3*j+2];	//B
		}

	TRACE( "DemosaicRGGB Successfully !!!\n" );
	TRACE( "=========================\n\n");
	return 1;		
} 

bool Demosaic_BGGR( unsigned char *inBuf, int inwidth, int inheight)	//bpattern = 1
{
	//ATTENTION: WATCH OUT THE SIZE HERE	
	int sum = inwidth * inheight;
	unsigned char *temp;
	temp = new unsigned char [ sum * 3 ];
	rgbBuf = new unsigned char [ sum * 3 ];
	//calculate the size of the 8bit raw

	//先处理四条边的数据
	//处理第一行
	for( int i = 0; i < inwidth; i ++ )				
	{												
		if( i == 0 )		//第一行，第一列，B
		{		
			temp[ 3 * i ]     = inBuf[ inwidth + i + 1 ];									//R
			temp[ 3 * i + 1 ] = 1.0/2.0 * ( inBuf[ i + 1 ] + inBuf[inwidth + i ]);		//G
			temp[ 3 * i + 2 ] = inBuf[ i ];												//B
		}
		else if( i!=0 && ( i % 2 == 0 ) )	//第一行，偶数列，B
		{
			temp[ 3 * i ]     = 1.0/2.0 * ( inBuf[ inwidth + i - 1] + inBuf[ inwidth + i + 1]);		//R	
			temp[ 3 * i + 1 ] = 1.0/3.0 * ( inBuf[ i -1 ]+ inBuf[ i + 1 ] + inBuf[inwidth + i ]);	//G
			temp[ 3 * i + 2 ] = inBuf[ i ];														//B
		}
		else if( i == inwidth -1 )			//第一行，最后一列，Gb
		{
			temp[ 3 * i ]     = inBuf[ inwidth + i ];										//R
			temp[ 3 * i + 1 ] = inBuf[ i ];												//G
			temp[ 3 * i + 2 ] = inBuf[ i - 1];											//B
		}
		else								//第一行，奇数列，Gb
		{
			temp[ 3 * i ]     = inBuf[ inwidth + i ];										//R
			temp[ 3 * i + 1 ] = inBuf[ i ];												//G
			temp[ 3 * i + 2 ] = 1.0/2.0 * ( inBuf[ i - 1 ] + inBuf[ i + 1 ]);			//B						
		}
	}
	//处理最后一行
	for( int i = 0; i < inwidth; i ++ )
	{
		if( i == 0 )		//最后一行，第一列,Gr
		{		
			temp[ inwidth *3*(inheight-1) + 3*i ]     = inBuf[ inwidth*(inheight-1)+ i + 1];		//R
			temp[ inwidth *3*(inheight-1) + 3*i + 1 ] = inBuf[ inwidth*(inheight-1)+ i ];			//G
			temp[ inwidth *3*(inheight-1) + 3*i + 2 ] = inBuf[ inwidth*(inheight-1)+ i - inwidth ];	//B			
		}
		else if( i!=0 && ( i % 2 == 0 ) )	//最后一行，偶数列Gr
		{
			temp[ inwidth *3*(inheight-1) +3*i ]     = 1.0/2.0 * ( inBuf[ inwidth*(inheight-1)+ i - 1] + inBuf[ inwidth*(inheight-1)+ i + 1] );		//R	
			temp[ inwidth *3*(inheight-1) +3*i + 1 ] = inBuf[ inwidth*(inheight-1)+ i ];														//G
			temp[ inwidth *3*(inheight-1) +3*i + 2 ] = inBuf[ inwidth*(inheight-1)+ i - inwidth ];												//B
		}
		else if( i == inwidth -1 )		//最后一行，最后一列,R
		{
			temp[ inwidth *3*(inheight-1) +3*i ]     = inBuf[ inwidth*(inheight-1)+ i ];				//R
			temp[ inwidth *3*(inheight-1) +3*i + 1 ] = 1.0/2.0 * (inBuf[inwidth*(inheight-1)-inwidth + i ] + inBuf[ inwidth*(inheight-1)+ i - 1]);	//G	
			temp[ inwidth *3*(inheight-1) +3*i + 2 ] = inBuf[ inwidth*(inheight-1)+ i - inwidth - 1];		//B									
		}
		else									//最后一行，奇数列，R
		{
			temp[ inwidth *3*(inheight-1) +3*i ]     = inBuf[ inwidth*(inheight-1)+ i ];																								//R
			temp[ inwidth *3*(inheight-1) +3*i + 1 ] = 1.0/3.0 * (  inBuf[ inwidth*(inheight-1)+ i - inwidth ] + inBuf[ inwidth*(inheight-1)+ i - 1 ] + inBuf[ inwidth*(inheight-1)+ i + 1 ] );	//G
			temp[ inwidth *3*(inheight-1) +3*i + 2 ] = 1.0/2.0 * ( inBuf[ inwidth*(inheight-1)+ i - inwidth - 1]+inBuf[inwidth*(inheight-1)+ i-inwidth + 1]);									//B						
		}
	}
	//处理第一列
	for( int i = 1; i < inheight - 1; i ++ )
	{
		if( i % 2 == 1)	//第一列，奇数行，Gr
		{
			temp[ inwidth *3 * i ]     = inBuf[ inwidth * i + 1];												//R
			temp[ inwidth *3 * i + 1 ] = inBuf[ inwidth * i ];													//G
			temp[ inwidth *3 * i + 2 ] = 1.0/2.0*(inBuf[ inwidth * ( i  - 1 )] + inBuf[ inwidth * ( i  + 1 )]);	//B						
		}
		else			//第一列, 偶数行，B
		{
			temp[ inwidth *3 * i ]     = 1.0/2.0 * ( inBuf[ inwidth * (i -1) + 1 ]+inBuf[inwidth*(i+1) +1]);		//R
			temp[ inwidth *3 * i + 1 ] = 1.0/3.0 * ( inBuf[inwidth*(i-1)]+inBuf[inwidth*i+1]+inBuf[inwidth*(i+1)]);	//G
			temp[ inwidth *3 * i + 2 ] = inBuf[ inwidth * i ];													//B					
		}
	}
	//处理最后一列
	for( int i = 1; i < inheight - 1; i ++ )
	{
		if( i % 2 != 0)	//最后一列，奇数行，B
		{
			temp[ inwidth *3 * i ]     = inBuf[ inwidth * (i + 1)-1];												//R
			temp[ inwidth *3 * i + 1 ] = 1.0/3.0*(inBuf[ inwidth * i-1]+inBuf[inwidth*(i+1)-2]+inBuf[inwidth*(i+2)-1]);//G
			temp[ inwidth *3 * i + 2 ] = 1.0/2.0*(inBuf[ inwidth * i - 2] + inBuf[ inwidth * ( i  + 2 ) - 2]);	//B						
		}
		else			//最后一列，偶数行，Gr
		{
			temp[ inwidth *3 * i ]     = 1.0/2.0 * ( inBuf[ inwidth * (i)-1 ]+inBuf[inwidth*(i+2)-1]);	//R								
			temp[ inwidth *3 * i + 1 ] = inBuf[ inwidth * (i+1)-1];										//G
			temp[ inwidth *3 * i + 2 ] = inBuf[ inwidth * (i+1)-2];										//B						
		}
	
	}
	//处理中间部分
	for( int i = 1; i < inheight-1; i ++ )
		for( int j = 1; j < inwidth-1; j ++ )
		{
			if( i %2 != 0)
			{
				if( j %2 != 0)	//R Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = inBuf[inwidth*i+j];	//R
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = 1.0/4.0 * (inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]+inBuf[inwidth*(i-1)+j]+inBuf[inwidth*(i+1)+j]);	//G
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = 1.0/4.0 * (inBuf[inwidth*(i-1)+j-1]+inBuf[inwidth*(i-1)+j+1]+inBuf[inwidth*(i+1)+j-1]+inBuf[inwidth*(i+1)+j+1]);	//B
				}
				else			//Gr Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = 1.0/2.0 * (inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]);		//R
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = inBuf[inwidth*i+j];//G
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = 1.0/2.0 * (inBuf[inwidth*(i-1)+j]+inBuf[inwidth*(i+1)+j]);	//B	
						
				}
			}
			else
			{
				if( j %2 != 0)	//Gb Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = 1.0/2.0 * (inBuf[inwidth*(i-1)+j]+inBuf[inwidth*(i+1)+j]);	//R
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = inBuf[inwidth*i+j];//G
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = 1.0/2.0 * (inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]);		//B
				}
				else			//B Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = 1.0/4.0 * (inBuf[inwidth*(i-1)+j-1]+inBuf[inwidth*(i-1)+j+1]+inBuf[inwidth*(i+1)+j-1]+inBuf[inwidth*(i+1)+j+1]);//R
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = 1.0/4.0 * (inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]+inBuf[inwidth*(i-1)+j]+inBuf[inwidth*(i+1)+j]);			//G
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = inBuf[inwidth*i+j];//																						//B
				}
			}
		}

	for(int i = 0; i < inheight; ++i )
		for( int j = 0; j < inwidth ; ++j)
		{
			rgbBuf[ ( inwidth * 3 * i ) + 3*j]   = temp[( inwidth * 3 * i ) + 3*j];		//R
			rgbBuf[ ( inwidth * 3 * i ) + 3*j+1] = temp[( inwidth * 3 * i ) + 3*j+1];	//G
			rgbBuf[ ( inwidth * 3 * i ) + 3*j+2] = temp[( inwidth * 3 * i ) + 3*j+2];	//B
		}

	TRACE( "Demosaic Successfully !!!\n" );
	TRACE( "=========================\n\n");

	////save demosaic data in a txt file
	//FILE *demosaic_rgb = fopen( "demosaic.txt",  "wb" );
	//for( int i = 0; i < inwidth * inheight * 3; i ++ )
	//{
	//	if( ( i != 0 ) && ( i % ( 3 * inwidth ) == 0 ) )
	//		fprintf( demosaic_rgb, "\n" );
	//	fprintf( demosaic_rgb, "%3d ", rgbBuf[i] );
	//}
	//fclose( demosaic_rgb );
	return 1;
}

bool Demosaic_GRBG( unsigned char *inBuf, int inwidth, int inheight)
{
	//ATTENTION: WATCH OUT THE SIZE HERE	
	int sum = inwidth * inheight;
	unsigned char *temp;
	temp = new unsigned char [ sum * 3 ];
	rgbBuf = new unsigned char [ sum * 3 ];
	//for(int i = 0; i < sum*3; ++i )
	//	rgbBuf[i] = 0;
	//calculate the size of the 8bit raw

	//先处理四条边的数据
	//处理第一行
	for( int i = 0; i < inwidth; i ++ )				
	{												
		if( i == 0 )		//第一行，第一列，Gr
		{		
			temp[ 3 * i ]     = inBuf[ i + 1 ];									//R
			temp[ 3 * i + 1 ] = 1.0/2.0 * ( inBuf[i] + inBuf[inwidth +i+1 ]);	//G
			temp[ 3 * i + 2 ] = inBuf[ inwidth+ i ];							//B
		}
		else if( i!=0 && ( i % 2 == 0 ) )	//第一行，偶数列，Gr
		{
			temp[ 3 * i ]     = 1.0/2.0 * ( inBuf[ i - 1] + inBuf[ i + 1 ]);		//R	
			temp[ 3 * i + 1 ] = 1.0/3.0 * ( inBuf[ i]+ inBuf[ inwidth+i + 1 ] + inBuf[inwidth + i-1 ]);	//G
			temp[ 3 * i + 2 ] = inBuf[ inwidth + i ];														//B
		}
		else if( i == inwidth -1 )			//第一行，最后一列，R
		{
			temp[ 3 * i ]     = inBuf[ i ];										//R
			temp[ 3 * i + 1 ] = 1.0/2.0 * ( inBuf[ i - 1] + inBuf[ inwidth + i ]);	//G
			temp[ 3 * i + 2 ] = inBuf[ inwidth + i - 1];						//B
		}
		else								//第一行，奇数列，R
		{
			temp[ 3 * i ]     = inBuf[ i ];										//R
			temp[ 3 * i + 1 ] = 1.0/3.0 * ( inBuf[i-1]+ inBuf[ i + 1 ] + inBuf[inwidth + i ]);//G
			temp[ 3 * i + 2 ] = 1.0/2.0 * ( inBuf[ inwidth+ i - 1 ] + inBuf[ inwidth + i + 1 ]);			//B						
		}
	}
	//处理最后一行
	for( int i = 0; i < inwidth; i ++ )
	{
		if( i == 0 )		//最后一行，第一列,B
		{		
			temp[ inwidth *3*(inheight-1) + 3*i ]     = inBuf[ inwidth*(inheight-1)-inwidth +i+1 ];		//R
			temp[ inwidth *3*(inheight-1) + 3*i + 1 ] = 1.0/2.0*(inBuf[ inwidth*(inheight-1)-inwidth+i]+inBuf[ inwidth*(inheight-1)+i+1]);			//G
			temp[ inwidth *3*(inheight-1) + 3*i + 2 ] = inBuf[ inwidth*(inheight-1)+ i];				//B			
		}
		else if( i!=0 && ( i % 2 == 0 ) )	//最后一行，偶数列B
		{
			temp[ inwidth *3*(inheight-1) +3*i ]     = 1.0/2.0 * ( inBuf[ inwidth*(inheight-1)-inwidth+i - 1] + inBuf[ inwidth*(inheight-1)-inwidth + i + 1] );		//R	
			temp[ inwidth *3*(inheight-1) +3*i + 1 ] = 1.0/3.0 * ( inBuf[ inwidth*(inheight-1)-inwidth+i]+inBuf[ inwidth*(inheight-1)-inwidth+i-1 ]+inBuf[ inwidth*(inheight-1)-inwidth+i+1 ]);														//G
			temp[ inwidth *3*(inheight-1) +3*i + 2 ] = inBuf[ inwidth*(inheight-1)+ i];					//B
		}
		else if( i == inwidth -1 )		//最后一行，最后一列,Gb
		{
			temp[ inwidth *3*(inheight-1) +3*i ]     = inBuf[ inwidth*(inheight-1)-inwidth+ i ];		//R
			temp[ inwidth *3*(inheight-1) +3*i + 1 ] = 1.0/2.0 * (inBuf[inwidth*(inheight-1)-inwidth + i-1 ] + inBuf[ inwidth*(inheight-1)+ i]);	//G	
			temp[ inwidth *3*(inheight-1) +3*i + 2 ] = inBuf[ inwidth*(inheight-1)+ i - 1];				//B									
		}
		else									//最后一行，奇数列，Gb
		{
			temp[ inwidth *3*(inheight-1) +3*i ]     = inBuf[ inwidth*(inheight-1)-inwidth + i ];																								//R
			temp[ inwidth *3*(inheight-1) +3*i + 1 ] = 1.0/3.0 * (  inBuf[ inwidth*(inheight-1)+ i-1 - inwidth ] + inBuf[ inwidth*(inheight-1)+ i+1-inwidth] + inBuf[ inwidth*(inheight-1)+ i ] );	//G
			temp[ inwidth *3*(inheight-1) +3*i + 2 ] = 1.0/2.0 * ( inBuf[ inwidth*(inheight-1)+ i - 1]+inBuf[inwidth*(inheight-1)+ i + 1]);									//B						
		}
	}
	//处理第一列
	for( int i = 1; i < inheight - 1; i ++ )
	{
		if( i % 2 == 1)	//第一列，奇数行，B
		{
			temp[ inwidth *3 * i ]     = 1.0/2.0 * ( inBuf[ inwidth * (i-1) + 1] + inBuf[ inwidth * (i+1) + 1] );												//R
			temp[ inwidth *3 * i + 1 ] = 1.0/3.0 * ( inBuf[ inwidth * i +1] + inBuf[inwidth*(i-1)]+inBuf[inwidth*(i+1)]);													//G
			temp[ inwidth *3 * i + 2 ] = inBuf[ inwidth * i];	//B						
		}
		else			//第一列, 偶数行，Gr
		{
			temp[ inwidth *3 * i ]     = inBuf[ inwidth * i + 1 ];		//R
			temp[ inwidth *3 * i + 1 ] = 1.0/3.0 * (inBuf[inwidth*i]+inBuf[inwidth*(i-1)+1]+inBuf[inwidth*(i+1)+1]);	//G
			temp[ inwidth *3 * i + 2 ] = 1.0/2.0 * ( inBuf[ inwidth * (i+1)] + inBuf[ inwidth * (i-1) ] );													//B					
		}
	}
	//处理最后一列
	for( int i = 1; i < inheight - 1; i ++ )
	{
		if( i % 2 != 0)	//最后一列，奇数行，Gb
		{
			temp[ inwidth *3 * i+inwidth-3 ] = 1.0/2.0 * ( inBuf[ inwidth * (i+1)+ inwidth-1 ] + inBuf[ inwidth * (i-1)+inwidth-1 ] );												//R
			temp[ inwidth *3 * i+inwidth-2 ] = inBuf[ inwidth * i+inwidth-1];//G
			temp[ inwidth *3 * i+inwidth-1 ] = inBuf[ inwidth * i - 1+inwidth-1];	//B						
		}
		else			//最后一列，偶数行，R
		{
			temp[ inwidth *3 * i +inwidth-3 ]     = inBuf[ inwidth * i +inwidth-1];	//R								
			temp[ inwidth *3 * i +inwidth-2 ] = 1.0/3.0 * ( inBuf[ inwidth * i -1+inwidth-1] + inBuf[inwidth*(i-1)+inwidth-1]+inBuf[inwidth*(i+1)+inwidth-1]);										//G
			temp[ inwidth *3 * i +inwidth-1 ] = 1.0/2.0 * ( inBuf[ inwidth * (i-1)-1+inwidth-1] + inBuf[ inwidth * (i+1)-1+inwidth-1] );										//B						
		}
	
	}
	//处理中间部分
	for( int i = 1; i < inheight-1; i ++ )
		for( int j = 1; j < inwidth-1; j ++ )
		{
			if( i %2 != 0)
			{
				if( j %2 != 0)	//Gb Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = 1.0/2.0 * ( inBuf[inwidth*(i-1)+j] + inBuf[inwidth*(i+1)+j] );	//R
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = inBuf[inwidth*i+j];	//G
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = 1.0/2.0 * (inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]);	//B
				}
				else			//B Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = 1.0/4.0 * (inBuf[inwidth*(i-1)+j-1]+inBuf[inwidth*(i-1)+j+1]+inBuf[inwidth*(i+1)+j-1]+inBuf[inwidth*(i+1)+j+1]);//R
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = 1.0/4.0 * (inBuf[inwidth*(i-1)+j]+inBuf[inwidth*(i+1)+j]+inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]);//G
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = inBuf[inwidth*i+j];	//B	
						
				}
			}
			else
			{
				if( j %2 != 0)	//R Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = inBuf[inwidth*i+j];	//R
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = 1.0/4.0 * (inBuf[inwidth*(i-1)+j]+inBuf[inwidth*(i+1)+j]+inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]);//G
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = 1.0/4.0 * (inBuf[inwidth*(i-1)+j-1]+inBuf[inwidth*(i-1)+j+1]+inBuf[inwidth*(i+1)+j-1]+inBuf[inwidth*(i+1)+j+1]);//B
				}
				else			//Gr Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = 1.0/2.0 * (inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]);//R
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = inBuf[inwidth*i+j];			//G
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = 1.0/2.0 * ( inBuf[inwidth*(i-1)+j] + inBuf[inwidth*(i+1)+j] );	//B
				}
			}
		}

	for(int i = 0; i < inheight; ++i )
		for( int j = 0; j < inwidth ; ++j)
		{
			rgbBuf[ ( inwidth * 3 * i ) + 3*j]   = temp[( inwidth * 3 * i ) + 3*j];		//R
			rgbBuf[ ( inwidth * 3 * i ) + 3*j+1] = temp[( inwidth * 3 * i ) + 3*j+1];	//G
			rgbBuf[ ( inwidth * 3 * i ) + 3*j+2] = temp[( inwidth * 3 * i ) + 3*j+2];	//B
		}

	TRACE( "Demosaic Successfully !!!\n" );
	TRACE( "=========================\n\n");
	return 1;
}

bool Demosaic_GBRG( unsigned char *inBuf, int inwidth, int inheight)
{
	//ATTENTION: WATCH OUT THE SIZE HERE	
	int sum = inwidth * inheight;
	unsigned char *temp;
	temp = new unsigned char [ sum * 3 ];
	rgbBuf = new unsigned char [ sum * 3 ];
	//for(int i = 0; i < sum*3; ++i )
	//	rgbBuf[i] = 0;
	//calculate the size of the 8bit raw

	//先处理四条边的数据
	//处理第一行
	for( int i = 0; i < inwidth; i ++ )				
	{												
		if( i == 0 )		//第一行，第一列，Gr
		{		
			temp[ 3 * i ]     = inBuf[ i + 1 ];									//R
			temp[ 3 * i + 1 ] = 1.0/2.0 * ( inBuf[i] + inBuf[inwidth +i+1 ]);	//G
			temp[ 3 * i + 2 ] = inBuf[ inwidth+ i ];							//B
		}
		else if( i!=0 && ( i % 2 == 0 ) )	//第一行，偶数列，Gr
		{
			temp[ 3 * i ]     = 1.0/2.0 * ( inBuf[ i - 1] + inBuf[ i + 1 ]);		//R	
			temp[ 3 * i + 1 ] = 1.0/3.0 * ( inBuf[ i]+ inBuf[ inwidth+i + 1 ] + inBuf[inwidth + i-1 ]);	//G
			temp[ 3 * i + 2 ] = inBuf[ inwidth + i ];														//B
		}
		else if( i == inwidth -1 )			//第一行，最后一列，R
		{
			temp[ 3 * i ]     = inBuf[ i ];										//R
			temp[ 3 * i + 1 ] = 1.0/2.0 * ( inBuf[ i - 1] + inBuf[ inwidth + i ]);	//G
			temp[ 3 * i + 2 ] = inBuf[ inwidth + i - 1];						//B
		}
		else								//第一行，奇数列，R
		{
			temp[ 3 * i ]     = inBuf[ i ];										//R
			temp[ 3 * i + 1 ] = 1.0/3.0 * ( inBuf[i-1]+ inBuf[ i + 1 ] + inBuf[inwidth + i ]);//G
			temp[ 3 * i + 2 ] = 1.0/2.0 * ( inBuf[ inwidth+ i - 1 ] + inBuf[ inwidth + i + 1 ]);			//B						
		}
	}
	//处理最后一行
	for( int i = 0; i < inwidth; i ++ )
	{
		if( i == 0 )		//最后一行，第一列,B
		{		
			temp[ inwidth *3*(inheight-1) + 3*i ]     = inBuf[ inwidth*(inheight-1)-inwidth +i+1 ];		//R
			temp[ inwidth *3*(inheight-1) + 3*i + 1 ] = 1.0/2.0*(inBuf[ inwidth*(inheight-1)-inwidth+i]+inBuf[ inwidth*(inheight-1)+i+1]);			//G
			temp[ inwidth *3*(inheight-1) + 3*i + 2 ] = inBuf[ inwidth*(inheight-1)+ i];				//B			
		}
		else if( i!=0 && ( i % 2 == 0 ) )	//最后一行，偶数列B
		{
			temp[ inwidth *3*(inheight-1) +3*i ]     = 1.0/2.0 * ( inBuf[ inwidth*(inheight-1)-inwidth+i - 1] + inBuf[ inwidth*(inheight-1)-inwidth + i + 1] );		//R	
			temp[ inwidth *3*(inheight-1) +3*i + 1 ] = 1.0/3.0 * ( inBuf[ inwidth*(inheight-1)-inwidth+i]+inBuf[ inwidth*(inheight-1)-inwidth+i-1 ]+inBuf[ inwidth*(inheight-1)-inwidth+i+1 ]);														//G
			temp[ inwidth *3*(inheight-1) +3*i + 2 ] = inBuf[ inwidth*(inheight-1)+ i];					//B
		}
		else if( i == inwidth -1 )		//最后一行，最后一列,Gb
		{
			temp[ inwidth *3*(inheight-1) +3*i ]     = inBuf[ inwidth*(inheight-1)-inwidth+ i ];		//R
			temp[ inwidth *3*(inheight-1) +3*i + 1 ] = 1.0/2.0 * (inBuf[inwidth*(inheight-1)-inwidth + i-1 ] + inBuf[ inwidth*(inheight-1)+ i]);	//G	
			temp[ inwidth *3*(inheight-1) +3*i + 2 ] = inBuf[ inwidth*(inheight-1)+ i - 1];				//B									
		}
		else									//最后一行，奇数列，Gb
		{
			temp[ inwidth *3*(inheight-1) +3*i ]     = inBuf[ inwidth*(inheight-1)-inwidth + i ];																								//R
			temp[ inwidth *3*(inheight-1) +3*i + 1 ] = 1.0/3.0 * (  inBuf[ inwidth*(inheight-1)+ i-1 - inwidth ] + inBuf[ inwidth*(inheight-1)+ i+1-inwidth] + inBuf[ inwidth*(inheight-1)+ i ] );	//G
			temp[ inwidth *3*(inheight-1) +3*i + 2 ] = 1.0/2.0 * ( inBuf[ inwidth*(inheight-1)+ i - 1]+inBuf[inwidth*(inheight-1)+ i + 1]);									//B						
		}
	}
	//处理第一列
	for( int i = 1; i < inheight - 1; i ++ )
	{
		if( i % 2 == 1)	//第一列，奇数行，B
		{
			temp[ inwidth *3 * i ]     = 1.0/2.0 * ( inBuf[ inwidth * (i-1) + 1] + inBuf[ inwidth * (i+1) + 1] );												//R
			temp[ inwidth *3 * i + 1 ] = 1.0/3.0 * ( inBuf[ inwidth * i +1] + inBuf[inwidth*(i-1)]+inBuf[inwidth*(i+1)]);													//G
			temp[ inwidth *3 * i + 2 ] = inBuf[ inwidth * i];	//B						
		}
		else			//第一列, 偶数行，Gr
		{
			temp[ inwidth *3 * i ]     = inBuf[ inwidth * i + 1 ];		//R
			temp[ inwidth *3 * i + 1 ] = 1.0/3.0 * (inBuf[inwidth*i]+inBuf[inwidth*(i-1)+1]+inBuf[inwidth*(i+1)+1]);	//G
			temp[ inwidth *3 * i + 2 ] = 1.0/2.0 * ( inBuf[ inwidth * (i+1)] + inBuf[ inwidth * (i-1) ] );													//B					
		}
	}
	//处理最后一列
	for( int i = 1; i < inheight - 1; i ++ )
	{
		if( i % 2 != 0)	//最后一列，奇数行，Gb
		{
			temp[ inwidth *3 * i+inwidth-3 ] = 1.0/2.0 * ( inBuf[ inwidth * (i+1)+ inwidth-1 ] + inBuf[ inwidth * (i-1)+inwidth-1 ] );												//R
			temp[ inwidth *3 * i+inwidth-2 ] = inBuf[ inwidth * i+inwidth-1];//G
			temp[ inwidth *3 * i+inwidth-1 ] = inBuf[ inwidth * i - 1+inwidth-1];	//B						
		}
		else			//最后一列，偶数行，R
		{
			temp[ inwidth *3 * i +inwidth-3 ]     = inBuf[ inwidth * i +inwidth-1];	//R								
			temp[ inwidth *3 * i +inwidth-2 ] = 1.0/3.0 * ( inBuf[ inwidth * i -1+inwidth-1] + inBuf[inwidth*(i-1)+inwidth-1]+inBuf[inwidth*(i+1)+inwidth-1]);										//G
			temp[ inwidth *3 * i +inwidth-1 ] = 1.0/2.0 * ( inBuf[ inwidth * (i-1)-1+inwidth-1] + inBuf[ inwidth * (i+1)-1+inwidth-1] );										//B						
		}
	
	}
	//处理中间部分
	for( int i = 1; i < inheight-1; i ++ )
		for( int j = 1; j < inwidth-1; j ++ )
		{
			if( i %2 != 0)
			{
				if( j %2 != 0)	//Gb Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = 1.0/2.0 * ( inBuf[inwidth*(i-1)+j] + inBuf[inwidth*(i+1)+j] );	//R
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = inBuf[inwidth*i+j];	//G
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = 1.0/2.0 * (inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]);	//B
				}
				else			//B Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = 1.0/4.0 * (inBuf[inwidth*(i-1)+j-1]+inBuf[inwidth*(i-1)+j+1]+inBuf[inwidth*(i+1)+j-1]+inBuf[inwidth*(i+1)+j+1]);//R
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = 1.0/4.0 * (inBuf[inwidth*(i-1)+j]+inBuf[inwidth*(i+1)+j]+inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]);//G
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = inBuf[inwidth*i+j];	//B	
						
				}
			}
			else
			{
				if( j %2 != 0)	//R Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = inBuf[inwidth*i+j];	//R
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = 1.0/4.0 * (inBuf[inwidth*(i-1)+j]+inBuf[inwidth*(i+1)+j]+inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]);//G
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = 1.0/4.0 * (inBuf[inwidth*(i-1)+j-1]+inBuf[inwidth*(i-1)+j+1]+inBuf[inwidth*(i+1)+j-1]+inBuf[inwidth*(i+1)+j+1]);//B
				}
				else			//Gr Channel
				{
						temp[ ( inwidth * 3 * i ) + 3 * j ]     = 1.0/2.0 * (inBuf[inwidth*i+j-1]+inBuf[inwidth*i+j+1]);//R
						temp[ ( inwidth * 3 * i ) + 3 * j + 1 ] = inBuf[inwidth*i+j];			//G
						temp[ ( inwidth * 3 * i ) + 3 * j + 2 ] = 1.0/2.0 * ( inBuf[inwidth*(i-1)+j] + inBuf[inwidth*(i+1)+j] );	//B
				}
			}
		}

	for(int i = 0; i < inheight; ++i )
		for( int j = 0; j < inwidth ; ++j)
		{
			rgbBuf[ ( inwidth * 3 * i ) + 3*j]   = temp[( inwidth * 3 * i ) + 3*j];		//R
			rgbBuf[ ( inwidth * 3 * i ) + 3*j+1] = temp[( inwidth * 3 * i ) + 3*j+1];	//G
			rgbBuf[ ( inwidth * 3 * i ) + 3*j+2] = temp[( inwidth * 3 * i ) + 3*j+2];	//B
		}

	TRACE( "Demosaic Successfully !!!\n" );
	TRACE( "=========================\n\n");
	return 1;
}

bool Demosaic_raw_to_rgb( unsigned char *inBuf, int inwidth, int inheight, int pattern )
{
	switch( pattern )
	{
	case 0:	//BGGR
		Demosaic_BGGR( inBuf, inwidth, inheight );
		break;
	case 1:	//RGGB
		Demosaic_RGGB( inBuf, inwidth, inheight );
		break;
	case 2:
		Demosaic_GRBG( inBuf, inwidth, inheight );
		break;
	default:
		Demosaic_GBRG( inBuf, inwidth, inheight );
		break;
	}
	return 1;
}


bool SaveBmp(CString sFileName, unsigned char *bmpData, int inwidth, int inheight, int bitCount)
{
	//judge the bmpBuf is available
	if( !bmpData )
	{
		TRACE( "Error: The raw data is not valid!!!" );
		return 0;
	}
	//Reshape the bmpData
	//BGR from Left bottom to up and right
	//BGR <--- RGB
	unsigned char *tempBuf;
	//ATTENTION: WATCH OUT THE SIZE HERE
//	inwidth = (inwidth + 11) /12 * 12;
	int lineByte = inwidth * 3;//( inwidth * bitCount /8 + 3 ) / 4 * 4;	
	tempBuf = new unsigned char [ lineByte * inheight ];
	//correct the raw data order, from left down to left up
	for( int i = 0; i < inheight; i ++ )
		for( int j = 0; j < inwidth; j ++ )
	{
		tempBuf[  lineByte * i + 3 * j ]     = bmpData[  lineByte * (inheight - 1 - i ) + 3 * j + 2 ];		//B = r,g,B
		tempBuf[  lineByte * i + 3 * j + 1 ] = bmpData[  lineByte * (inheight - 1 - i ) + 3 * j + 1 ];		//G = r,G,b
		tempBuf[  lineByte * i + 3 * j + 2 ] = bmpData[  lineByte * (inheight - 1 - i ) + 3 * j ];			//R = R,g,b
	}
	
	//save demosaic data in a txt file
	//FILE *bmp_rgb = fopen( "bmp_rgb.txt",  "wb" );
	//for( int i = 0; i < inwidth * inheight * 3; i ++ )
	//{
	//	if( ( i != 0 ) && ( i % ( 3 * inwidth ) == 0 ) )
	//		fprintf( bmp_rgb, "\n" );
	//	fprintf( bmp_rgb, "%3d ", tempBuf[i] );
	//}
	//fclose( bmp_rgb );

	//calculate the lineByte to make sure it is 4 times bigger	//4 bytes alignment

	//open a file by bin
	CFile sBmp;
	sBmp.Open( sFileName, CFile::modeCreate | CFile::modeWrite);

	//fill in the bitmap file header
//	BITMAPFILEHEADER fileHead;
	fileHead.bfType	= 0x4d42;	//type is "BM"
	fileHead.bfSize	= sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + lineByte * inheight;
	fileHead.bfReserved1 = 0;
	fileHead.bfReserved2 = 0;
	fileHead.bfOffBits	= 54;

//	TRACE("bfSize = %d\n", fileHead.bfSize);
	//write the file header into the file
//	fwrite( &fileHead, sizeof(BITMAPFILEHEADER), 1, sBmp );
	sBmp.Write( &fileHead, sizeof(BITMAPFILEHEADER) );

	//fill in the bitmap info header
//	BITMAPINFOHEADER infohead;
	infohead.biBitCount	= bitCount;
	infohead.biClrImportant	= 0;
	infohead.biClrUsed	= 0;
	infohead.biCompression	= 0;
	infohead.biHeight = inheight;
	infohead.biPlanes = 1;
	infohead.biSize = 40;
	infohead.biSizeImage = lineByte * inheight;
	infohead.biWidth = inwidth;
	infohead.biXPelsPerMeter = 0;
	infohead.biYPelsPerMeter = 0;

	//write the info header into the file
//	fwrite( &infohead, sizeof(BITMAPINFOHEADER), 1, sBmp);
	sBmp.Write( &infohead, sizeof(BITMAPINFOHEADER));

	TRACE("size = lineByte %d * height %d = %d\n", lineByte, inheight, lineByte*inheight);
	//write the bmp data into the file
//	fwrite( tempBuf, 1, lineByte* inheight, sBmp);
	sBmp.Write( tempBuf, lineByte * inheight );

	//close the file
//	fclose( sBmp );
	sBmp.Close();
//	delete []tempBuf;
	TRACE("Save Bmp succeed!\n");
	return 1;
}
 
//Optical Center Test
//===================
//This function is used to test the optical center of the raw image
//Myron add 2016/07/27
double OC_Test( unsigned char* in_rgbBuf, int inwidth, int inheight)
{
	double oc_x=0;
	double oc_y=0;
	double count=0;

	double OC_Dis_X = 0;
	double OC_Dis_Y = 0;
	double OC_Dis = 0;
	Y_Channel = new unsigned char[ inwidth * inheight ];
//calculate the Y channel value
	for( int i = 0; i < inheight; i ++ )
		for( int j = 0; j < inwidth; j++ )
		{
			Y_Channel[ inwidth *i + j ] = 0.299 * in_rgbBuf[ inwidth*i+3*j] +
										 0.587 * in_rgbBuf[ inwidth*i+3*j+1] +
										 0.114 * in_rgbBuf[ inwidth*i+3*j+2];
		}
//calculate the OC_THRESHOLD
//==========================
//myron add 2016/07/28
//1.Devote the image into 100*100pixel blocks
//2.Calculate the block average values of the corner and the center
//3.The 4 cornet blocks average value is A1, The center average value is A2, The total average is A3
//4.OC_THRESHOLD = A3
//The ROI is like below:
//======++======
//=            =
//+     ++     +
//=            =
//======++======
	int A1 = 0;
	int A2 = 0;
	int A3 = 0;	
	//Top 100*100
	for( int i= 0; i < 100; i ++ )
		for( int j = inwidth/2-50; j < inwidth/2+50; j++ )
		{
			A1 += Y_Channel[ inwidth*i + j ];
		}
	TRACE("A1_TOP = %d\n", A1/10000 );
	//Left 100*100
	for( int i = inheight/2-50; i< inheight/2+50; i++ )
		for( int j = 0; j < 100; j ++ )
		{
			A1 += Y_Channel[ inwidth*i + j ];
		}
	TRACE("A1_LEFT = %d\n", A1/20000 );
	//Bottom 100*100
	for( int i = inheight-100; i < inheight; i++ )
		for( int j= inwidth/2-50; j < inwidth/2+50; j++ )
		{
			A1 += Y_Channel[ inwidth*i + j ];
		}
	TRACE("A1_BOTTOM = %d\n", A1/30000 );
	//Right 100*100
	for( int i = inheight/2-50; i < inheight/2+50; i++ )
		for( int j= inwidth-100; j < inwidth; j++ )
		{
			A1 += Y_Channel[ inwidth*i + j ];
		}
	TRACE("A1_RIGHT = %d\n", A1/40000 );
	//Center 100*100
	for( int i = inheight/2-50; i < inheight/2+50; i++ )
		for( int j = inwidth/2-50; j < inwidth/2+50; j++ )
		{
			A2 += Y_Channel[ inwidth*i + j ];
		}
	TRACE("A2_CENTER = %d\n", A2/10000 );
	A1 = A1 / 40000;
	A2 = A2 / 10000;
	A3 = ( A1 + A2 ) / 2;
	OC_THRESHOLD = A3;
	TRACE("OC_THRESHOLD = %d\n", OC_THRESHOLD );

//calculate the OC_Dis
//==========================
	for( int i = 0; i < inheight; i ++ )
		for( int j = 0 ; j < inwidth; j ++)
		{		
			if( Y_Channel[ inwidth*i+j ] > OC_THRESHOLD )
			{
				oc_x += j+1;	
				oc_y += i+1;
				count ++;
			}
		}
	oc_x = oc_x / count;
	oc_y = oc_y / count;
	OC_Dis_X = oc_x - inwidth / 2;
	OC_Dis_Y = oc_y - inheight / 2;
	OC_Dis = OC_Dis_X*OC_Dis_X + OC_Dis_Y*OC_Dis_Y;
	OC_Dis = sqrt( (double)OC_Dis );

	TRACE("OC_count=%f\n", count);
	TRACE("OC_X=%f\n", oc_x);
	TRACE("OC_Y=%f\n", oc_y);
	TRACE("OC_Dis_X=%f\n", OC_Dis_X);
	TRACE("OC_Dis_Y=%f\n", OC_Dis_Y);
	return OC_Dis;
}