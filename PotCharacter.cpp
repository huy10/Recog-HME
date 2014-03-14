#include"PotCharacter.h"
#include <fstream>
#include <stdlib.h>
using namespace std;

//#define MYDEBUG 1;

/*C语言读入pot文件，PointData的空间需要提前分配*/
#ifdef POTX
int openPotx(FILE *fo,unsigned short PointData[],char* code, unsigned long *size, int iterator)
{
    
	//PointData=(unsigned char *)malloc(2048*sizeof(unsigned char));	
	unsigned long BlockSize,NumberOfByteOccupied;
	if(!fo)
		return 0;
	if(fread(&BlockSize,sizeof(unsigned long),1,fo)<1)
		return 0;
	unsigned short TextLength ;
	if(fread(&TextLength,sizeof(unsigned short),1,fo)<1)
		return 0;
	//char  Text[1000] ;
	if(fread(code,TextLength,1,fo)<1)
		return 0;
	unsigned short NumOfStrokes ;
	if(fread(&NumOfStrokes,sizeof(unsigned short),1,fo)<1)
		return 0;
	* size =  BlockSize;
	
	//fseek(fo,2,SEEK_CUR);
	NumberOfByteOccupied = BlockSize - 8 - TextLength;	
	if(NumberOfByteOccupied>10000)
	{
		fread(PointData,sizeof(unsigned short),10000/2,fo);
		fseek(fo,NumberOfByteOccupied-10000,SEEK_CUR);
		NumberOfByteOccupied=10000;
	}
	else
		fread(PointData,sizeof(unsigned short),NumberOfByteOccupied/2,fo);

	int NumberOfPoint = NumberOfByteOccupied/4;	//总点数，各个笔划结束标志及字结束标志也分别作为一个点
	
	return NumberOfPoint;
}
#else
int openPot(FILE *fo,unsigned short PointData[],char* code)
{
	unsigned short BlockSize,NumberOfByteOccupied;
	if(!fo)
		return 0;
	if(fread(&BlockSize,sizeof(unsigned short),1,fo)<1)
		return 0;
	if(fread(code,2,1,fo)<1)
		return 0;
	fseek(fo,2,SEEK_CUR);
	NumberOfByteOccupied=BlockSize-6;	/*实际点坐标所占字节数*/
	if(NumberOfByteOccupied>2048)
	{
		fread(PointData,sizeof(unsigned char),2048,fo);
		fseek(fo,NumberOfByteOccupied-2048,SEEK_CUR);
		NumberOfByteOccupied=2048;
	}
	else
		fread(PointData,sizeof(unsigned char),NumberOfByteOccupied,fo);

	int NumberOfPoint = NumberOfByteOccupied/2;	//总点数，各个笔划结束标志及字结束标志也分别作为一个点
	PointData[NumberOfByteOccupied-4]=0xff;
	PointData[NumberOfByteOccupied-3]=0;
	PointData[NumberOfByteOccupied-2]=0xff;
	PointData[NumberOfByteOccupied-1]=0xff;			

	
	return NumberOfPoint;
}
#endif //POTX

