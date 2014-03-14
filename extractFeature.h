#ifndef __EXTRACT_FEATURE_H__
#define __EXTRACT_FEATURE_H__

//	PDA版处理时分配的变量的空间
#define __PDA_GLOBAL_RAM_SIZE	5000
#define GLOBAL_PARAMETER_POSITION 3536

//	抽取联机特征相关的类型及函数
short ExtractOnlineFourDirectionFeature(unsigned char *PointData,short PointsNumber,short *pFeature,char *pRamAddress);
short ExtractOnlineEightDirectionFeature(unsigned char *PointData,short PointsNumber,short *pFeature,char *pRamAddress);
short OnlineNorm(unsigned char *PointData,short NumberOfPoint, char *RamAddress);   
#endif // __EXTRACT_FEATURE_H__
