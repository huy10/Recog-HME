#include<iostream>
#include"PotCharacter.h"
#include <stdlib.h>
#include "math.h"
#include <fstream>
#include <iomanip>
#include <clocale>
#include <wchar.h>
#include "HmcInterface.h"
#include "hmcCommon.h"
#include "hmcSegment.h"
#include "hmcAnalyse.h"
#include "hmcSgmntGmm.h"
#include "UnicodeSymbolClass.h"
//#include "wtpen.h"
//#include "wtpen_V4.h"

#define MYDEBUG
using namespace std;

#ifdef POTX
int getCandidates (unsigned short character[],unsigned short & can1,unsigned   short & can2, unsigned  short & can3, unsigned  short & can4,unsigned  short & can5,unsigned  short & can6, unsigned short & can7, unsigned short & can8, unsigned short & can9, unsigned short & can10,unsigned short & can11,unsigned   short & can12, unsigned  short & can13, unsigned  short & can14,unsigned  short & can15,unsigned  short & can16, unsigned short & can17, unsigned short & can18, unsigned short & can19, unsigned short & can20)//////////////////
{
	
	char szFileName[260];
	FILE *fp = 0;
	long lFileSize;
	

	//m_szWorkPathA = "..\\HmcDemo";
	// data file 
	sprintf(szFileName, ".\\WTPENpda.lib");//, m_szWorkPathA);   //C:\\yanghu\\Formula\\HC_improve\\HmcDemo
	fp = fopen(szFileName, "rb");

	fseek(fp, 0, SEEK_END);
	lFileSize = ftell(fp);
	char *m_pDatFileBuf;
	 m_pDatFileBuf = new char [lFileSize];

	fseek(fp, 0, SEEK_SET);
	fread(m_pDatFileBuf, 1, lFileSize, fp);
	fclose(fp);
	fp = 0;

	// global RAM
	char *m_pGlobalRam = 0;
	m_pGlobalRam = new char [HCR_RAM_SIZE];

	// initialization interface
	HmcInit(m_pDatFileBuf, m_pGlobalRam);

	unsigned short uPointNum = 0;

	int i=0;
	bool bFlag = true;
	unsigned int right = 0,bottom = 0;

	while(bFlag)
	{
		unsigned short a = character[i];
		unsigned short b = character[i+1];

		if (a!=0xffff && a > right)
			right = a;
		if (b!=0xffff && b > bottom)
			bottom = b;
		if(a== 0xffff && b==0xffff)
		  bFlag = false;

		i+=2;
        uPointNum++;
	}
	unsigned short * data = new unsigned short [uPointNum*2+6];
	i = 0;
	bFlag = true;
	while(bFlag)
	{
		unsigned short a = character[i];
		unsigned short b = character[i+1];

			data[i] = a;
		


			data[i+1] = b;
		if(a== 0xffff && b==0xffff)
		  bFlag = false;
		i+=2;
	}

   unsigned short   uCandidateResult[100];
	//uCandidateResult = new unsigned short [20];


	for (int j = 0; j < 100; j++)
		uCandidateResult[j] = 0;



	HmcSetRect(right * 2,bottom * 2, m_pGlobalRam);
 	HmcRecog(data,	 uPointNum,	   5,	   1,   m_pGlobalRam,  uCandidateResult);

	//can1 = uCandidateResult;

	if (uCandidateResult[0] != 0xffff)
		can1 = uCandidateResult[0];
	if (uCandidateResult[1] != 0xffff)
		can2 = uCandidateResult[1];
	if (uCandidateResult[2] != 0xffff)
		can3 = uCandidateResult[2];
	if (uCandidateResult[3] != 0xffff)
		can4 = uCandidateResult[3];
	if (uCandidateResult[4] != 0xffff)
		can5 = uCandidateResult[4];
	if (uCandidateResult[5] != 0xffff)
		can6 = uCandidateResult[5];
	if (uCandidateResult[6] != 0xffff)
		can7 = uCandidateResult[6];
	if (uCandidateResult[7] != 0xffff)
		can8 = uCandidateResult[7];
	if (uCandidateResult[8] != 0xffff)
		can9 = uCandidateResult[8];
	if (uCandidateResult[9] != 0xffff)
		can10 = uCandidateResult[9];
	if (uCandidateResult[9] != 0xffff)
		can10 = uCandidateResult[9];
	if (uCandidateResult[10] != 0xffff)
		can11 = uCandidateResult[10];
	if (uCandidateResult[11] != 0xffff)
		can12 = uCandidateResult[11];
		if (uCandidateResult[12] != 0xffff)
		can13 = uCandidateResult[12];
	if (uCandidateResult[13] != 0xffff)
		can14 = uCandidateResult[13];
	if (uCandidateResult[14] != 0xffff)
		can15 = uCandidateResult[14];
	if (uCandidateResult[15] != 0xffff)
		can16 = uCandidateResult[15];
	if (uCandidateResult[16] != 0xffff)
		can17 = uCandidateResult[16];
	if (uCandidateResult[17] != 0xffff)
		can18 = uCandidateResult[17];
	if (uCandidateResult[18] != 0xffff)
		can19 = uCandidateResult[18];
		if (uCandidateResult[19] != 0xffff)
		can20 = uCandidateResult[19];
	return 1;
}
#else
int getCandidates (unsigned char character[],unsigned short & can1,unsigned   short & can2, unsigned  short & can3, unsigned  short & can4,unsigned  short & can5,unsigned  short & can6, unsigned short & can7, unsigned short & can8, unsigned short & can9, unsigned short & can10,unsigned short & can11,unsigned   short & can12, unsigned  short & can13, unsigned  short & can14,unsigned  short & can15,unsigned  short & can16, unsigned short & can17, unsigned short & can18, unsigned short & can19, unsigned short & can20)//////////////////
{
	
	char szFileName[260];
	FILE *fp = 0;
	long lFileSize;
	

	//m_szWorkPathA = "C:\\yanghu\\Formula\\HC\\HmcDemo";
	// data file 
	sprintf(szFileName, ".\\WTPENpda.lib");//, m_szWorkPathA);   //C:\\yanghu\\Formula\\HC_improve\\HmcDemo
	fp = fopen(szFileName, "rb");

	fseek(fp, 0, SEEK_END);
	lFileSize = ftell(fp);
	char *m_pDatFileBuf;
	 m_pDatFileBuf = new char [lFileSize];

	fseek(fp, 0, SEEK_SET);
	fread(m_pDatFileBuf, 1, lFileSize, fp);
	fclose(fp);
	fp = 0;

	// global RAM
	char *m_pGlobalRam = 0;
	m_pGlobalRam = new char [HCR_RAM_SIZE];

	// initialization interface
	HmcInit(m_pDatFileBuf, m_pGlobalRam);

	unsigned short uPointNum = 0;

	int i=0;
	bool bFlag = true;
	unsigned int right = 0,bottom = 0;

	while(bFlag)
	{
		unsigned char a = character[i];
		unsigned char b = character[i+1];

		if (a!=0xff && a > right)
			right = a;
		if (b!=0xff && b > bottom)
			bottom = b;
		if(a== 0xff && b==0xff)
		  bFlag = false;

		i+=2;
        uPointNum++;
	}
	unsigned short * data = new unsigned short [uPointNum*2+6];
	i = 0;
	bFlag = true;
	while(bFlag)
	{
		unsigned char a = character[i];
		unsigned char b = character[i+1];
		if(a == 0xFF)
			data[i] = 0xFFFF;
		else
			data[i] = (unsigned short)a;
		

		if(b==0xFF)
			data[i+1] = 0xFFFF;
		else
			data[i+1] = (unsigned short)b;
		if(a== 0xff && b==0xff)
		  bFlag = false;
		i+=2;
	}

   unsigned short   uCandidateResult[100];
	//uCandidateResult = new unsigned short [20];


	for (int j = 0; j < 100; j++)
		uCandidateResult[j] = 0;



	HmcSetRect(right*2,bottom*2, m_pGlobalRam);
 	HmcRecog(data,	 uPointNum,	   5,	   1,   m_pGlobalRam,  uCandidateResult);

	//can1 = uCandidateResult;

	if (uCandidateResult[0] != 0xffff)
		can1 = uCandidateResult[0];
	if (uCandidateResult[1] != 0xffff)
		can2 = uCandidateResult[1];
	if (uCandidateResult[2] != 0xffff)
		can3 = uCandidateResult[2];
	if (uCandidateResult[3] != 0xffff)
		can4 = uCandidateResult[3];
	if (uCandidateResult[4] != 0xffff)
		can5 = uCandidateResult[4];
	if (uCandidateResult[5] != 0xffff)
		can6 = uCandidateResult[5];
	if (uCandidateResult[6] != 0xffff)
		can7 = uCandidateResult[6];
	if (uCandidateResult[7] != 0xffff)
		can8 = uCandidateResult[7];
	if (uCandidateResult[8] != 0xffff)
		can9 = uCandidateResult[8];
	if (uCandidateResult[9] != 0xffff)
		can10 = uCandidateResult[9];
	if (uCandidateResult[9] != 0xffff)
		can10 = uCandidateResult[9];
	if (uCandidateResult[10] != 0xffff)
		can11 = uCandidateResult[10];
	if (uCandidateResult[11] != 0xffff)
		can12 = uCandidateResult[11];
		if (uCandidateResult[12] != 0xffff)
		can13 = uCandidateResult[12];
	if (uCandidateResult[13] != 0xffff)
		can14 = uCandidateResult[13];
	if (uCandidateResult[14] != 0xffff)
		can15 = uCandidateResult[14];
	if (uCandidateResult[15] != 0xffff)
		can16 = uCandidateResult[15];
	if (uCandidateResult[16] != 0xffff)
		can17 = uCandidateResult[16];
	if (uCandidateResult[17] != 0xffff)
		can18 = uCandidateResult[17];
	if (uCandidateResult[18] != 0xffff)
		can19 = uCandidateResult[18];
		if (uCandidateResult[19] != 0xffff)
		can20 = uCandidateResult[19];
	return 1;
}
#endif  //POTX
int main(int argc,char* argv[])
{

	/*for (int i = 0; i <pc.samples[0].strokes[0].data[0])
	pc.samples[0].strokes.at(i).*/
	char * code;
	int iterator = 1;
#ifdef POTX
	
	FILE * fp = fopen(argv[1],"rb");
	unsigned short * PointData ;

	PointData = new unsigned short[10000];
	memset(PointData, 0, 10000*sizeof(unsigned short));
	code = new char[100];
	unsigned long size = 0;
	openPotx(fp,PointData,code,&size,iterator);
	//openPotx(fp,PointData,code,&size,iterator,argv[2]);
	iterator ++;
	
#else
	unsigned char * PointData ;
	PointData = new unsigned char[2048];
	code = new char[10];
	FILE * fp = fopen(argv[1],"rb");
	openPot(fp,PointData,code);
#endif	
	unsigned  short  i1 = 0;
   unsigned  short i2 = 0;
    unsigned short i3 = 0;
    unsigned short i4 = 0;
    unsigned short i5 = 0;
    unsigned short i6 = 0;
    unsigned short i7 = 0;
    unsigned short i8 = 0;
    unsigned short i9 = 0;
    unsigned short i10 = 0;
	unsigned short i11 = 0;
	unsigned short i12 = 0;
	unsigned short i13 = 0;
	unsigned short i14 = 0;
	unsigned short i15 = 0;
	unsigned short i16 = 0;
	unsigned short i17 = 0;
	unsigned short i18 = 0;
	unsigned short i19 = 0;
	unsigned short i20 = 0;

	int FinalResult = getCandidates ( PointData,  i1, i2,i3,i4,i5 , i6,i7, i8, i9, i10,i11,i12,i13,i14,i15,i16,i17,i18,i19,i20);
	setlocale(LC_ALL, "chs");       
	wprintf(L"%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc\n",i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,i14,i15,i16,i17,i18,i19,i20);
	
	UnicodeSymbolClass	m_UnicodeSymbolClass;

	//wprintf("%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x",i1,i2,i3,i4,i5,i6,i7,i8,i9,i10);

	/*FILE * fptxt;
	char * potx_iterator;
	iterator ++;
	potx_iterator = strcat("num" , (char*)iterator) ;
	potx_iterator = strcat(potx_iterator , ".txt");
	fptxt = fopen(potx_iterator,"a");
	fprintf(fptxt,"%lc %lc %lc %lc %lc %lc %lc %lc %lc %lc %lc %lc %lc %lc %lc %lc %lc %lc %lc %lc",i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,i14,i15,i16,i17,i18,i19,i20);
	fprintf(fptxt,"\n");
	fclose(fptxt);*/
	delete []code;
	delete []PointData;

	

	//system("pause");

	
	return 1;
}
