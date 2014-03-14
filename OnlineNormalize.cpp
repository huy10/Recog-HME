#include "stdlib.h"
#include "math.h"
#include "stdafx.h"
#include "ExtractFeature_inner.h"

//	对输入轨迹进行线性归一化
short LinearNorm(unsigned char *PointData,short NumberOfPoint,char *RamAddress)
{
	unsigned char *pAxis_X;
	short start_x,start_y,end_x,end_y,x,y;
	pAxis_X=PointData;
	end_x=start_x=*pAxis_X;
	end_y=start_y=*(pAxis_X+1);
	/*得到最大和最小值，也就是边框大小*/
	while(*pAxis_X!=0xff||*(pAxis_X+1)!=0xff)
	{
		if(*pAxis_X!=0xff||*(pAxis_X+1)!=0)
		{
			if(start_x>*pAxis_X)		start_x=*pAxis_X;
			if(start_y>*(pAxis_X+1))	start_y=*(pAxis_X+1);
			if(end_x<*pAxis_X)		end_x=*pAxis_X;
			if(end_y<*(pAxis_X+1))	end_y=*(pAxis_X+1);   	
		}
		pAxis_X+=2;
	}	
	if((end_x-start_x)>4*(end_y-start_y))
	{	/* maybe it is ONE*/
		x=y=(end_x-start_x+1);
	}
	else
	{
		if((end_y-start_y)>4*(end_x-start_x))
			x=y=(end_y-start_y+1);
		else
		{
			x=(end_x-start_x+1);
			y=(end_y-start_y+1);
		}
	}
	/*如果过于狭长，则保持原长宽比放大到0~63，否则将长宽均规一化到0~63*/
	pAxis_X=PointData;
	while(*pAxis_X!=0xff||*(pAxis_X+1)!=0xff)
	{
		if(*pAxis_X!=0xff||*(pAxis_X+1)!=0)
		{
			*pAxis_X=((*pAxis_X-start_x)*63/x);
			*(pAxis_X+1)=((*(pAxis_X+1)-start_y)*63/y);
		}
		pAxis_X+=2;
	}/*归一化到0~63*/
	return NumberOfPoint;
}
/********************************************************************************
 *	Prototype:	void OnlineDotDensityNorm(unsigned char *PointData,short NumberOfPoint,			*
 *					char *RamAddress)											*
 *	Purpose:	将输入数据进行归一化和均衡处理									*
 *	Parameters:	<->PointData	数据点(以每两个字节作为一对数据)				*
 *				->NumberOfPoint 数据的对数										*
 *				->RamAddress	Ram空间的地址									*
 *	Return:		none															*
 *	Comments:																	*
 ********************************************************************************/
short OnlineDotDensityNorm(unsigned char *PointData,short NumberOfPoint,char *RamAddress)
{
	short slope,x,y,k;
	unsigned short *pTemp_Current_x,*pTemp_Current_y,*pTemp_End_x;
	WT_int32 long_x,long_y,long_tempx,long_tempy,longTemp;
	short start_x,start_y,end_x,end_y;
	unsigned char *pAxis_X;
	unsigned short *mapx,*mapy;

	mapx=(unsigned short *)RamAddress;
	mapy=(unsigned short *)(RamAddress+128);

	/*
	 *	先进行线性归一化
	 */
	pAxis_X=PointData;
	end_x=start_x=*pAxis_X;
	end_y=start_y=*(pAxis_X+1);
	/*得到最大和最小值，也就是边框大小*/
	while(*pAxis_X!=0xff||*(pAxis_X+1)!=0xff)
	{
		if(*pAxis_X!=0xff||*(pAxis_X+1)!=0)
		{
			if(start_x>*pAxis_X)		start_x=*pAxis_X;
			if(start_y>*(pAxis_X+1))	start_y=*(pAxis_X+1);
			if(end_x<*pAxis_X)		end_x=*pAxis_X;
			if(end_y<*(pAxis_X+1))	end_y=*(pAxis_X+1);   	
		}
		pAxis_X+=2;
	}	
	if((end_x-start_x)>4*(end_y-start_y))
	{	/* maybe it is ONE*/
		x=y=(end_x-start_x+1);
	}
	else
	{
		if((end_y-start_y)>4*(end_x-start_x))
			x=y=(end_y-start_y+1);
		else
		{
			x=(end_x-start_x+1);
			y=(end_y-start_y+1);
		}
	}
	/*如果过于狭长，则保持原长宽比放大到0~63，否则将长宽均规一化到0~63*/
	pAxis_X=PointData;
	while(*pAxis_X!=0xff||*(pAxis_X+1)!=0xff)
	{
		if(*pAxis_X!=0xff||*(pAxis_X+1)!=0)
		{
			*pAxis_X=((*pAxis_X-start_x)*63/x);
			*(pAxis_X+1)=((*(pAxis_X+1)-start_y)*63/y);
		}
		pAxis_X+=2;
	}/*归一化到0~63*/

	pTemp_Current_x=mapx;
	pTemp_Current_y=mapy;
	pTemp_End_x=mapx+64;
	while(pTemp_Current_x<pTemp_End_x)
	{
		*pTemp_Current_x=30;
		*pTemp_Current_y=30;
		pTemp_Current_x++;
		pTemp_Current_y++;
	}

	pAxis_X=PointData;
	while(*pAxis_X==0xff)
		pAxis_X+=2;		/*	跳到笔迹开始点，防止文件错误，起点处不是笔迹点	*/
	while(*(pAxis_X+1)!=0xff)
	{
		start_x=(unsigned short)*pAxis_X;
		start_y=(unsigned short)*(pAxis_X+1);
		pAxis_X+=2;
		if(*pAxis_X==0xff)			/*该笔划已经结束，跳到下一个笔划的起点*/
		{
			while(*pAxis_X==0xff)	//防止出现几个连续的笔划结束标志
			{
				if(*(pAxis_X+1)!=0xff)	/*	不是笔迹结束	*/
				{
					pAxis_X+=2;
				}
				else
					break;
			}
			if(*(pAxis_X+1)==0xff)	/*	笔迹结束	*/
				break;
			else
				continue;
		}
		end_x=(unsigned short)*pAxis_X;
		end_y=(unsigned short)*(pAxis_X+1);		/*对一个笔划的相邻点*/
		
		if(start_x==end_x && start_y==end_y) continue;	/* 前后两点位置没有变化 */

		if(WTPEN_abs(start_y-end_y)>WTPEN_abs(start_x-end_x))
		{
			slope=((((WTPEN_abs(end_y-start_y))+1)<<6)/((WTPEN_abs(end_x-start_x))+1));
			slope=5*slope>>6;
			if(end_y>start_y)
			{
				for(y=start_y-2;y<=end_y+2;y++)
				{
					if(y>=0&&y<=63)		/*在有效的区间内*/
						for(k=y-2;k<=y+2;k++)
							if(k>=start_y&&k<=end_y)
								mapy[y]+=5;
				}
			}
			else
			{
				for(y=end_y-2;y<=start_y+2;y++)
				{
					if(y>=0&&y<=63)		/*在有效的区间内*/
						for(k=y-2;k<=y+2;k++)
							if(k>=end_y&&k<=start_y)
								mapy[y]+=5;
				}
			}
			if(start_x<end_x)
			{
				for(x=start_x-2;x<=end_x+2;x++)
				{
					if(x>=0&&x<=63)
						for(k=x-2;k<=x+2;k++)
							if(k>=start_x&&k<=end_x)
								mapx[x]+=slope;
				}
			}
			else
			{
				for(x=end_x-2;x<=start_x+2;x++)
				{
					if(x>=0&&x<=63)
						for(k=x-2;k<=x+2;k++)
							if(k>=end_x&&k<=start_x)
								mapx[x]+=slope;
				}
			}
		}
		else
		{
			if(end_x!=start_x)
				slope=((((WTPEN_abs(end_x-start_x))+1)<<6)/((WTPEN_abs(end_y-start_y))+1));
			else
				slope=1<<6;
			slope=5*slope>>6;
			if(end_x>start_x)
			{
				for(x=start_x-2;x<=end_x+2;x++)
				{
					if(x>=0&&x<=63)
						for(k=x-2;k<=x+2;k++)
							if(k>=start_x&&k<=end_x)
								mapx[x]+=5;
				}
			}
			else
			{
				for(x=end_x-2;x<=start_x+2;x++)
				{
					if(x>=0&&x<=63)
						for(k=x-2;k<=x+2;k++)
							if(k>=end_x&&k<=start_x)
								mapx[x]+=5;
				}
			}
			if(end_y>start_y)
			{
				for(y=start_y-2;y<=end_y+2;y++)
				{
					if(y>=0&&y<=63)		/*在有效的区间内*/
						for(k=y-2;k<=y+2;k++)
							if(k>=start_y&&k<=end_y)
								mapy[y]+=slope;
				}
			}
			else
			{
				for(y=end_y-2;y<=start_y+2;y++)
				{
					if(y>=0&&y<=63)		/*在有效的区间内*/
						for(k=y-2;k<=y+2;k++)
							if(k>=end_y&&k<=start_y)
								mapy[y]+=slope;
				}
			}
		}
	}
	pTemp_Current_x=mapx;
	pTemp_Current_y=mapy;
	long_x=0;
	long_y=0;
	while(pTemp_Current_x<pTemp_End_x)
	{
		long_x+=*pTemp_Current_x;
		long_y+=*pTemp_Current_y;
		pTemp_Current_x++;
		pTemp_Current_y++;
	}
	pTemp_Current_x=mapx;
	pTemp_Current_y=mapy;
	long_tempx=0;
	long_tempy=0;
	while(pTemp_Current_x<pTemp_End_x)
	{
		long_tempx+=*pTemp_Current_x;
		long_tempy+=*pTemp_Current_y;
		longTemp=(long_tempx<<6)-long_tempx;
		*pTemp_Current_x=(unsigned short)(longTemp/long_x);
		longTemp=(long_tempy<<6)-long_tempy;
		*pTemp_Current_y=(unsigned short)(longTemp/long_y);
		pTemp_Current_x++;
		pTemp_Current_y++;
	}
	mapx[63]=mapy[63]=63;  /* in case new[63]==64*/
	pAxis_X=PointData;
	while(*pAxis_X==0xff)
		pAxis_X+=2;
	while(*(pAxis_X+1)!=0xff)
	{
		if(*pAxis_X!=0xff)
		{
			*pAxis_X=(unsigned char)mapx[*pAxis_X];
			*(pAxis_X+1)=(unsigned char)mapy[*(pAxis_X+1)];
		}
		pAxis_X+=2;
	}
	return NumberOfPoint;
}
//	设置Ｘ与Ｙ方向的映射值的初值
void SetInitialMappingValue(unsigned short *mapx,unsigned short *mapy,unsigned short InitValue)
{
	unsigned short *pTemp_Current_x=mapx;
	unsigned short *pTemp_Current_y=mapy;
	unsigned short *pTemp_End_x=mapx+64;
	while(pTemp_Current_x<pTemp_End_x)
	{
		*pTemp_Current_x=InitValue;
		*pTemp_Current_y=InitValue;
		pTemp_Current_x++;
		pTemp_Current_y++;
	}	
}
//将点(start_x,start_y)与点(end_x,end_y)间的点向X与Y方向映射
//映射顺序是从坐标值小的到坐标值大的,映射时不进行扩散
void DoMappingInAscendOrder_NoDiffuse(short start_x,short start_y,short end_x,short end_y,short MapValueX,short MapValueY,unsigned short *mapx,unsigned short *mapy)
{
	short x,y;
	if(end_x>start_x)
	{
		for(x=start_x;x<=end_x;x++)
			mapx[x]+=MapValueX;
	}
	else
	{
		for(x=end_x;x<=start_x;x++)
			mapx[x]+=MapValueX;
	}
	if(end_y>start_y)
	{
		for(y=start_y;y<=end_y;y++)
			mapy[y]+=MapValueY;
	}
	else
	{
		for(y=end_y;y<=start_y;y++)
			mapy[y]+=MapValueY;
	}
}


//	进行扩散,不等扩散半径,高斯加权
void Diffuse_ChangeableWidth_GaussianWeight_New(unsigned short *map)
{
	unsigned short SavedMap[64];
	int i,j,Width;
	for(i=0;i<64;i++)
	{
		SavedMap[i]=map[i];
		map[i]=0;
	}
	for(i=0;i<64;i++)
	{
		if(SavedMap[i]>48*5)
		{
			unsigned short GaussianWeight[11]={4,4,4,4,5,5,5,4,4,4,4};
			Width=5;
			for(j=i-Width;j<=i+Width;j++)
			{
				if(j>=0&&j<64)
					map[j]+=GaussianWeight[j+Width-i]*SavedMap[i];
			}
		}
		else
		{
			unsigned short GaussianWeight[9]={1,2,2,4,5,4,2,2,1};
			Width=4;
			for(j=i-Width;j<=i+Width;j++)
			{
				if(j>=0&&j<64)
					map[j]+=GaussianWeight[j+Width-i]*SavedMap[i]*2;
			}
		}
	}
	return;
}
//计算映射函数
void CalculateMappingFunction(unsigned short *mapx,unsigned short *mapy)
{
	unsigned short *pTemp_Current_x,*pTemp_Current_y,*pTemp_End_x;
	long long_x,long_y,long_tempx,long_tempy,longTemp;

	pTemp_Current_x=mapx;
	pTemp_Current_y=mapy;
	pTemp_End_x=mapx+64;
	long_x=0;
	long_y=0;
	while(pTemp_Current_x<pTemp_End_x)
	{
		long_x+=*pTemp_Current_x;
		long_y+=*pTemp_Current_y;
		pTemp_Current_x++;
		pTemp_Current_y++;
	}
	pTemp_Current_x=mapx;
	pTemp_Current_y=mapy;
	long_tempx=0;
	long_tempy=0;
	while(pTemp_Current_x<pTemp_End_x)
	{
		long_tempx+=*pTemp_Current_x;
		long_tempy+=*pTemp_Current_y;
		longTemp=(long_tempx<<6)-long_tempx;
		*pTemp_Current_x=(unsigned short)(longTemp/long_x);
		longTemp=(long_tempy<<6)-long_tempy;
		*pTemp_Current_y=(unsigned short)(longTemp/long_y);
		pTemp_Current_x++;
		pTemp_Current_y++;
	}
	mapx[63]=mapy[63]=63;  /* in case new[63]==64*/
}

//	对轨迹按照映射函数mapx与mapy进行映射
void MappingScript(unsigned char *PointData,unsigned short *mapx,unsigned short *mapy)
{
	unsigned char *pAxis_X=PointData;
	while(*pAxis_X==0xff)
		pAxis_X+=2;
	while(*(pAxis_X+1)!=0xff)
	{
		if(*pAxis_X!=0xff)
		{
			*pAxis_X=(unsigned char)mapx[*pAxis_X];
			*(pAxis_X+1)=(unsigned char)mapy[*(pAxis_X+1)];
		}
		pAxis_X+=2;
	}
	return;
}

short OnlineNorm(unsigned char *PointData,short NumberOfPoint, char * pRamAddress)
{
	short slope,start_x,start_y,end_x,end_y;
	unsigned char *pAxis_X;
	unsigned short mapx[64],mapy[64];
	short MapValueX,MapValueY;

	//	先进行线性归一化
	LinearNorm(PointData,NumberOfPoint,pRamAddress);   // NULL  huyang 2013-08-13

	//	设置X与Y映射方向的初值
	SetInitialMappingValue(mapx,mapy,6);
	
	pAxis_X=PointData;
	while(*pAxis_X==0xff)
		pAxis_X+=2;		/*	跳到笔迹开始点，防止文件错误，起点处不是笔迹点	*/
	while(*(pAxis_X+1)!=0xff)
	{
		start_x=(unsigned short)*pAxis_X;
		start_y=(unsigned short)*(pAxis_X+1);
		pAxis_X+=2;
		if(*pAxis_X==0xff)			/*该笔划已经结束，跳到下一个笔划的起点*/
		{
			while(*pAxis_X==0xff)	//防止出现几个连续的笔划结束标志
			{
				if(*(pAxis_X+1)!=0xff)	/*	不是笔迹结束	*/
				{
					pAxis_X+=2;
				}
				else
					break;
			}
			if(*(pAxis_X+1)==0xff)	/*	笔迹结束	*/
				break;
			else
				continue;
		}
		end_x=(unsigned short)*pAxis_X;
		end_y=(unsigned short)*(pAxis_X+1);		/*对一个笔划的相邻点*/
		
		if(start_x==end_x && start_y==end_y) continue;	/* 前后两点位置没有变化 */

		if(WTPEN_abs(start_y-end_y)>WTPEN_abs(start_x-end_x))
		{
			slope=((((WTPEN_abs(end_y-start_y))+1)<<6)/((WTPEN_abs(end_x-start_x))+1));
			MapValueX=5*slope>>6;
			MapValueY=5;
		}
		else
		{
			if(end_x!=start_x)
				slope=((((WTPEN_abs(end_x-start_x))+1)<<6)/((WTPEN_abs(end_y-start_y))+1));
			else
				slope=1<<6;
			MapValueX=5;
			MapValueY=5*slope>>6;
		}
		DoMappingInAscendOrder_NoDiffuse(start_x,start_y,end_x,end_y,MapValueX,MapValueY,mapx,mapy);
	}
	//以高斯型进行扩散
	Diffuse_ChangeableWidth_GaussianWeight_New(mapx);
	Diffuse_ChangeableWidth_GaussianWeight_New(mapy);
	//计算映射函数
	CalculateMappingFunction(mapx,mapy);
	//进行映射
	MappingScript(PointData,mapx,mapy);
	return NumberOfPoint;
}
