#ifndef __RECOCORE_H
#define __RECOCORE_H
/*
 * Declaration of functioins				
 */
#include "wtpen.h"
#define GLOBAL_PARAMETER_POSITION 1536
typedef struct
{
	/*用户自适应功能*/
	unsigned short *pModelSequenceToInnerCode;
	unsigned short *pActiveModelSequenceToInnerCode;
	unsigned short *pCodeTableOfUsrModel;
	unsigned short *pActiveCodeTableOfUsrModel;

	unsigned short *pTableIndex;
	unsigned short *pTreeTable;
	unsigned short *pModelZoneOfEachBit;

	unsigned short *pNumberOfUsrModel;
	unsigned short *TanTable;
	unsigned short *SquareTable;
	unsigned short *TableForMatching;
	unsigned short *pSequenceOfUsrModel;//指向用户库的模板序号

	short *pGeneralInformationSection;
	short *pUsrInformationSection;
	short *pModelFrequency;
	short *pBelongCharset;
	short *g_pFeatureTranslationVector;	/*	特征变换矩阵	*/

	
	unsigned char  *CosValue;

	char *pAntiForgeSection;
	char *pFeatureOfUsrModel;
	char *pTraSimLib;
	char *pCenterFeature;
	char *pModelFeature;					/*	指向实际存放模板Feature的各个record	*/
	char *LittleLibAddress;
	char *UsrLibAddress;
	
	//WT_int32 UsrRamSize;/*用户库大小*/
	WT_int32 RequiredCharset;				/*候选集范围*/
	WT_int32 SupportedCharset;				/*库所支持的字符集范围*/
	
	unsigned short g_nTotalModel;
	unsigned short Speed_const1;
	unsigned short Speed_const2;
	unsigned short nAreaMaxLength;//记录输入区域最长的边长
	
	unsigned short unMinCandDistance;
	unsigned short unReserved;//对齐
	
	short nLangNote;
	short SelfAdaptEnabled;					
	short SelfDefineEnabled;				
	short g_dimFeat;						/*	原始特征维数	*/
	short g_dimComp;						/*	变换后特征维数	*/
	short LDA_LOG2MultiValue;					/*	LDA变换阵的倍乘系数	*/
	short g_nTreeNodes;
	short NumberOfSupportedInnerCodeType;
	short WTInnerCodeType;					
	short NumberOfCandidate;
	short left;  //手写区域左上角的X坐标
    short top;   //手写区域左上角的Y坐标
    short right; //手写区域右下角的X坐标
    short bottom;//手写区域右下角的Y坐标
	short TraSimTag;
	short FullScreenInput;
	
}WTParameter;

#endif


