/******************************************************************
* Function: HmcInit
* Description: load database
* Commit: Liangrui Peng
* Call:
* Call By:
* General Variables:
* Input & Output:
*	->pDatabase:	database buffer
*	->pGlobalRam:	global ram
* Return:
*	0	success
*	else	fail
* Others:
******************************************************************/

#include "stdafx.h"

#include "MacroDef.h"
#include "Version.h"
#include "ExtractFeature.h"
#include "wtpen.h"

#define RECORD_POT
#ifdef RECORD_POT
 FILE * fpPot;
#endif
#define	STATIS_CORE_COUNT_1_DIM_DIST(pFeatVector,pModelFeaturePos)										\
										{	OneDimDist=(*pFeatVector) - (short)(*pModelFeaturePos);		\
											distance+=*(pTableForMatching+OneDimDist);					\
											pModelFeaturePos++;											\
											pFeatVector++;												\
										}
#define STATIS_CORE_COUNT_4_DIM_DIST(pFeatVector,pModelFeaturePos)										\
										{	STATIS_CORE_COUNT_1_DIM_DIST(pFeatVector,pModelFeaturePos)	\
											STATIS_CORE_COUNT_1_DIM_DIST(pFeatVector,pModelFeaturePos)	\
											STATIS_CORE_COUNT_1_DIM_DIST(pFeatVector,pModelFeaturePos)	\
											STATIS_CORE_COUNT_1_DIM_DIST(pFeatVector,pModelFeaturePos)	\
										}
#define STATIS_CORE_COUNT_16_DIM_DIST(pFeatVector,pModelFeaturePos)										\
										{	STATIS_CORE_COUNT_4_DIM_DIST(pFeatVector,pModelFeaturePos)	\
											STATIS_CORE_COUNT_4_DIM_DIST(pFeatVector,pModelFeaturePos)	\
											STATIS_CORE_COUNT_4_DIM_DIST(pFeatVector,pModelFeaturePos)	\
											STATIS_CORE_COUNT_4_DIM_DIST(pFeatVector,pModelFeaturePos)	\
										}

typedef struct tagWTParameter
{
	/*	常量表	*/
	unsigned char  *CosValue;
	unsigned short *TanTable;
	unsigned short *SquareTable;
	unsigned short *TableForMatching;

	/*	识别库地址	*/
	char *pLibAddress;						

	/*	用户库相关	*/
	char *UsrLibAddress;
	WT_int32 UsrRamSize;
	unsigned short *pNumberOfUsrModel;
	char *pFeatureOfUsrModel;
	unsigned short * pCodeTableOfUsrModel;
	unsigned short * pActiveCodeTableOfUsrModel;

	/*	识别器相关	*/
	unsigned short g_nTotalModel;						/*	总模板数		*/
	short g_dimFeat;									/*	原始特征维数	*/
	short g_dimComp;									/*	变换后特征维数	*/
	short LDA_LOG2MultiValue;							/*	LDA变换阵的倍乘系数	*/
	short * g_pFeatureTranslationVector;				/*	特征变换矩阵	*/
	char  *	pModelFeature;								/*	指向实际存放模板Feature的各个record	*/

	/*	分类树相关	*/
	short g_nTreeNodes;									/*	树节点数	*/
	unsigned short	OutRangeTreeNode;					/*	极端情形下，树节点与叶节点间连接超过65536，此时出现pTableIndex值越界,使用该值来判断是否有越界	*/
	char *pCenterFeature;								/*	树节点特征	*/
	unsigned short *pTableIndex;						/*	树节点->叶节点表索引	*/
	unsigned short *pTreeTable;							/*	叶节点表	*/
	unsigned short *pModelZoneOfEachBit;				/*	旧库中表示每个bit的模板序号范围，如使用新的库(>0x420)，可以舍弃此值	*/

	/*	辅助参数	*/
	unsigned short * pModelSequenceToInnerCode;			/*	模板序号到内码的查找表	*/
	unsigned short * pActiveModelSequenceToInnerCode;	/*	当前使用的模板序号到内码的查找表	*/
	short *pModelFrequency;								/*	模板先验概率	*/
	union
	{
		short *pBelongCharset;							/*	模板所属字符集范围(16位bit)	*/
		WT_int32 *pBelongCharset32;						/*	模板所属字符集范围(32位bit) */
	};
	short *pModelFunctionalInfo;				/*	模板的功能信息(如果存在) */
	char *pAntiForgeSection;
	char *pOrignModelFeature;
	short *pGeneralInformationSection;

	WT_int32 SupportedCharset;					/*	库所支持的字符集范围	*/
	short NumberOfSupportedInnerCodeType;		/*	库所支持的内码类型	*/
	short FeatureType;							/*	库的特征类型		*/
	short SelfAdaptEnabled;						/*	1,启用用户自适应;0,禁用用户自适应 */
	short Use32BitCharset;						/*	库中是否使用了32bit模板范围 */

	WT_int32 RequiredCharset;					/*	要求的候选集范围	*/
	short WTInnerCodeType;						/*	要求的输出内码	*/
	unsigned short Speed_const1,Speed_const2;	/*	速度参数		*/
	short NumberOfCandidate;					/*	要求的候选数目	*/


}WTParameter;


/*
 *	进行特征变换，将原始的g_dimFeat维特征变换为g_dimComp维特征
 */
void FeatureTranslation(short * pOrigFeat, short dimOrigFeat,short * pCompF,short dimCompFeat,short *pFeatureTransformVector,short LOG2MultiValue)
{
	short i,*pFeature,*pCompare,*pCompareAligned;
	WT_int32 sum;
	pCompare=pOrigFeat+dimOrigFeat;
	pCompareAligned=pOrigFeat+((dimOrigFeat>>3)<<3);
	for (i=0; i<dimCompFeat; i++)/*得到变换后的g_dimComp维特征*/
	{
		pFeature = pOrigFeat;
		sum=0;
		do
		{
			sum += ((*pFeatureTransformVector) * (*pFeature));
			pFeatureTransformVector++;
			pFeature++;
			sum += ((*pFeatureTransformVector) * (*pFeature));
			pFeatureTransformVector++;
			pFeature++;
			sum += ((*pFeatureTransformVector) * (*pFeature));
			pFeatureTransformVector++;
			pFeature++;
			sum += ((*pFeatureTransformVector) * (*pFeature));
			pFeatureTransformVector++;
			pFeature++;
			sum += ((*pFeatureTransformVector) * (*pFeature));
			pFeatureTransformVector++;
			pFeature++;
			sum += ((*pFeatureTransformVector) * (*pFeature));
			pFeatureTransformVector++;
			pFeature++;
			sum += ((*pFeatureTransformVector) * (*pFeature));
			pFeatureTransformVector++;
			pFeature++;
			sum += ((*pFeatureTransformVector) * (*pFeature));
			pFeatureTransformVector++;
			pFeature++;
		}while(pFeature<pCompareAligned);
		while(pFeature<pCompare)
		{	//补足余下的维数(维数有可能不是8的倍数
			sum += ((*pFeatureTransformVector) * (*pFeature));
			pFeatureTransformVector++;
			pFeature++;
		}
		/*
		 *	因为在生成PDA版本的识别库时，特征变换矩阵乘了256，而模板特征除了256，
		 *	所以这儿求出的特征值也必须进行同样的缩放，才能与模板特征间有可比性，
		 *	另外，为了查表的需要，生成的特征加了一个常数128
		 */
		sum=sum>>8;
		*pCompF=(short)(sum>>LOG2MultiValue);	/*即为/256/256*/
		if(*pCompF>127)
			*pCompF=127;
		if(*pCompF<-127)
			*pCompF=-127;
		(*pCompF)+=128;
		pCompF++;
	}
}

WTError WTAntiForge(short *pCompressedFeature,unsigned short *CandidateResult,unsigned short *CandiDistance,WTParameter *pGlobalParameter)
{
	char *pKeyCharacter,*pModelFeaturePos;
	short i,NumberOfKeyCharacter,DimensionOfKeyFeature,OneDimDist,*pFeatVector,*pCompareVector;
	unsigned short *pTableForMatching,*Tips;
	unsigned int distance;

	Tips=0;
	pTableForMatching=pGlobalParameter->TableForMatching;

	NumberOfKeyCharacter=*(short *)(pGlobalParameter->pAntiForgeSection);
	DimensionOfKeyFeature=*(short *)(pGlobalParameter->pAntiForgeSection+2);
	pKeyCharacter=(pGlobalParameter->pAntiForgeSection)+4;

	/*
	 *	逐一与KeyCharacter进行比较，看当前所写字是否是KeyCharacter,
	 *	如果是，则修改输出结果
	 */
	for(i=0;i<NumberOfKeyCharacter;i++)
	{
		pModelFeaturePos=pKeyCharacter+2;	// pass the two bytes which indicate the innercode

		pFeatVector=pCompressedFeature;
		pCompareVector=pFeatVector+pGlobalParameter->g_dimComp;
		distance=0;
		while(pFeatVector<pCompareVector)
		{
			STATIS_CORE_COUNT_4_DIM_DIST(pFeatVector,pModelFeaturePos)
			STATIS_CORE_COUNT_4_DIM_DIST(pFeatVector,pModelFeaturePos)
		}
		if(distance<=(*CandiDistance)&&distance<6000)
		{
			Tips=(unsigned short *)(pKeyCharacter+2+DimensionOfKeyFeature);
		}
		pKeyCharacter+=(2+DimensionOfKeyFeature+40);
	}
	if(Tips!=0)
	{
		for(i=0;i<pGlobalParameter->NumberOfCandidate;i++)
		{
			CandidateResult[i]=Tips[i];
			CandidateResult[i]+=1;
		}
	}
	return WTSuccess;
}
/*
 *	对结果进行后处理，当前的后处理是进字频修正
 */
WTError	WTPostProcess(unsigned short *pCandiDistance,unsigned short *pCandidateResult,short NumberOfCandidate,unsigned short *ModelIndex,short *pModelFrequency)
{
	short i,j;
	unsigned short tempUS;
	if(pCandiDistance[NumberOfCandidate-1]==STATIS_CORE_LARGE_DISTANCE)
	{
		for( i=0; i<NumberOfCandidate; i++ )
		{
			if(pCandiDistance[i]==STATIS_CORE_LARGE_DISTANCE)
				break;
		}
		NumberOfCandidate=i;	//有效的候选个数
	}
	for(i=0;i<NumberOfCandidate;i++)
	{
		pCandiDistance[i]+=pModelFrequency[ModelIndex[i]];
	}
	for(i=0;i<NumberOfCandidate;i++)
	{
		for(j=NumberOfCandidate-1;j>i;j--)
		{
			if(pCandiDistance[j]<pCandiDistance[j-1])
			{
				tempUS=pCandiDistance[j];
				pCandiDistance[j]=pCandiDistance[j-1];
				pCandiDistance[j-1]=tempUS;
				tempUS=pCandidateResult[j];
				pCandidateResult[j]=pCandidateResult[j-1];
				pCandidateResult[j-1]=tempUS;
			}
		}
	}
	return WTSuccess;

}

/*本函数功能是对特征进行变换，对变换后的特征寻找最佳匹配，并将结果输出。*/
void PDA_Classifier_Process(unsigned short *CandidateResult,char *pRamAddress)
{
	short i,nLeafNode,OneDimDist,NumberOfCandidate,*pFeatVector,*pCompareVector,*pCompressedFeature;
	unsigned short InnerCode,*pCandiDistance,*pLastDistance,*pCenterIndex,*pCenterIndexCompare,*pModelIdx; 
	signed char *pModelFeaturePos,*pSaveModelFeaturePos;	/*	采用明式的signed char 是为了防止不同编译器的差别 */
	unsigned short *p1,*p2,*p3,*p4,*p5,*p6,*pCompare,*pSave;
	register unsigned short *pTableForMatching;
	unsigned int distance;
	WTParameter *pGlobalParameter;

	pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);
	pTableForMatching=pGlobalParameter->TableForMatching;

	pFeatVector=(short *)pRamAddress;	/*	原始特征	*/
	if(pGlobalParameter->g_dimFeat>=512)	/*这样处理的目的是为了避免在原始特征维数较小时与后面的pCenterIndex，pSave等发生内存冲突*/
		pCompressedFeature=(short *)(pRamAddress+2*pGlobalParameter->g_dimFeat);	/*	变换后特征	*/
	else
		pCompressedFeature=(short *)(pRamAddress+2*512);							/*	变换后特征	*/
	FeatureTranslation(pFeatVector,pGlobalParameter->g_dimFeat,pCompressedFeature,pGlobalParameter->g_dimComp,pGlobalParameter->g_pFeatureTranslationVector,pGlobalParameter->LDA_LOG2MultiValue);

	pCandiDistance=(unsigned short *)pRamAddress;
	pCenterIndex=(unsigned short *)(pRamAddress+256);	/* 这样最多可以容忍 128个侯选类中心 */
	pSave=(unsigned short *)(pRamAddress+512);			//存放候选结果的模板序号

	NumberOfCandidate=pGlobalParameter->Speed_const1;

	for( i=0; i<NumberOfCandidate; i++ )
		pCandiDistance[i]=STATIS_CORE_LARGE_DISTANCE;
	pLastDistance=pCandiDistance+(NumberOfCandidate-1);

	if(pGlobalParameter->g_dimComp>=48)
		pCompareVector=pCompressedFeature+48;
	else
		pCompareVector=pCompressedFeature+pGlobalParameter->g_dimComp;

	pModelFeaturePos=(signed char *)(pGlobalParameter->pCenterFeature);
	/*	
	 *	选择最近的NumberOfCandidate个分类树树节点
	 */
	for(i=0; i<pGlobalParameter->g_nTreeNodes; i++)
	{
		pSaveModelFeaturePos=pModelFeaturePos;
		pFeatVector=pCompressedFeature;
		distance=0;
		do
		{
			STATIS_CORE_COUNT_4_DIM_DIST(pFeatVector,pModelFeaturePos)
			STATIS_CORE_COUNT_4_DIM_DIST(pFeatVector,pModelFeaturePos)

			if(distance>=(*pLastDistance))
				break;
		}while(pFeatVector<pCompareVector);
		pModelFeaturePos=pSaveModelFeaturePos+pGlobalParameter->g_dimComp; /*挪到后一个模板位*/
		if(distance>=(*pLastDistance))
			continue;
		else	/*如果距离小于最后一个候选，则插入进候选的NumberOfCandidate个中心中*/
		{
			p1=pCandiDistance;
			while(p1<pLastDistance)
			{	
				if(distance<*p1)
					break;
				p1++;
			}
			/*待识别字与当前模板的距离大于k-1，小于k;*/
			pCompare=p1;
			p2=pLastDistance;
			p1=p2-1;
			p4=pCenterIndex+(NumberOfCandidate-1);
			p3=p4-1;
			while(p2>pCompare)
			{
				*p2=*p1;
				p1--;
				p2--;
				*p4=*p3;
				p3--;
				p4--;
			}
			*p2=distance;
			*p4=i;
		}
	}
	/*
	 *	为了防止出现在特殊情况下，该字与任何类中心的距离都大于预设值，或小于预设值的候选类中心数
	 *	没有达到NumberOfCandidate时可能导致的错误，将距离过大的舍弃，得到实际真实有效的候选树节点个数
	 */
	if(pCandiDistance[NumberOfCandidate-1]==STATIS_CORE_LARGE_DISTANCE)
	{
		for( i=0; i<NumberOfCandidate; i++ )
		{
			if(pCandiDistance[i]==STATIS_CORE_LARGE_DISTANCE)
				break;
		}
		NumberOfCandidate=i;	//有效的候选个数
	}
	pCenterIndexCompare=pCenterIndex+NumberOfCandidate;

	/*	
	 *	实际要求的候选个数
	 */
	NumberOfCandidate=pGlobalParameter->NumberOfCandidate;
	for(i=0;i<NumberOfCandidate;i++)
	{
		pCandiDistance[i]=STATIS_CORE_LARGE_DISTANCE;
		CandidateResult[i]=0xffff;	
	}	
	pLastDistance=pCandiDistance+(NumberOfCandidate-1);
	/*
	 *	对分类树对应的叶节点进行处理
	 */
	pCompareVector=pCompressedFeature+pGlobalParameter->g_dimComp;
	nLeafNode=0;

	while(pCenterIndex<pCenterIndexCompare)
	{
		unsigned short * pCurrentTableIndex=pGlobalParameter->pTableIndex+(*pCenterIndex);
		unsigned short * pModelIndexCompare;
		pModelIdx=pGlobalParameter->pTreeTable+(*pCurrentTableIndex);
		if((*pCenterIndex)>=pGlobalParameter->OutRangeTreeNode)
			pModelIdx+=65536;
		pCurrentTableIndex++;
		pModelIndexCompare=pGlobalParameter->pTreeTable+(*pCurrentTableIndex);
		if((*pCenterIndex)+1>=pGlobalParameter->OutRangeTreeNode)
			pModelIndexCompare+=65536;
		pCenterIndex++;
//		nLeafNode+=(pModelIndexCompare-pModelIdx);
		while( pModelIdx <pModelIndexCompare)
		{
			if(pGlobalParameter->Use32BitCharset)
			{
#ifdef WINCE_PLAT
				//由于在WINCE平台下，这儿的pBelongCharset32可能不是4字节对齐，引起错误，因此进行下面的处理
				WT_int32 CurrentBelongCharset;
				unsigned short *pCurrentBelongCharset=(unsigned short *)pGlobalParameter->pBelongCharset32;
				pCurrentBelongCharset+=(*pModelIdx);
				pCurrentBelongCharset+=(*pModelIdx);
				CurrentBelongCharset=((*pCurrentBelongCharset)&0x0000ffff)+(((*(pCurrentBelongCharset+1))<<16)&0xffff0000);

				if(!(pGlobalParameter->RequiredCharset&CurrentBelongCharset))
#else
				if(!(pGlobalParameter->RequiredCharset&pGlobalParameter->pBelongCharset32[*pModelIdx]))
#endif
				{
					pModelIdx++;
					continue;
				}
			}
			else
			{
				if(!(pGlobalParameter->RequiredCharset&pGlobalParameter->pBelongCharset[*pModelIdx]))
				{
					pModelIdx++;
					continue;
				}
			}

			pModelFeaturePos=(signed char *)(pGlobalParameter->pModelFeature+((*pModelIdx)*pGlobalParameter->g_dimComp));
			distance=0;
			pFeatVector=pCompressedFeature;
			STATIS_CORE_COUNT_16_DIM_DIST(pFeatVector,pModelFeaturePos)			//先计算前面的16维
			do
			{
				STATIS_CORE_COUNT_4_DIM_DIST(pFeatVector,pModelFeaturePos)		
				STATIS_CORE_COUNT_4_DIM_DIST(pFeatVector,pModelFeaturePos)		
				if(distance>=(*pLastDistance))				//每几维进行一个判断，如果距离较大，不再进行下面的计算	
					break;
			}while(pFeatVector<pCompareVector);
			if(distance>=(*pLastDistance))
			{
				pModelIdx++;
				continue;
			}
			else	/*如果距离小于最后一个候选，则插入进候选的十个字中*/
			{
				p1=pCandiDistance;
				while(p1<pLastDistance)/*因为最后一个候选已经进行过比较，所以只要比较到倒数第二个就可以了*/
				{	
					if(distance<*p1)
						break;
					p1++;
				}
				/* 
				 * 按照距离，当前的模板应插入到p1的位置，但在多模板的情况下，
				 * 如果p1前已有该类的模板，则保留原模板作为候选
				 * 如果p1处也是该类的模板，则将与该模板的距离值改为当前距离
				 * 如果该类的模板在p1后的p2处，或在候选中还不存在该类的模板
				 * 则在p1处插入当前模板，并将p1--p2-1处的模板向后移一位
				 */
				InnerCode=(pGlobalParameter->pActiveModelSequenceToInnerCode)[*pModelIdx];
				if(InnerCode==0)
				{
					pModelIdx++;
					continue;
				}
				p4=CandidateResult;
				pCompare=CandidateResult+(NumberOfCandidate-1);
				while(p4<pCompare)
				{
					if(InnerCode==(*p4))
						break;
					p4++;
				}
				p2=pCandiDistance+(p4-CandidateResult);
				p6=pSave+(p4-CandidateResult);

				if(p2>p1)	
				{
					pCompare=p1;	
					p1=p2-1;
					p3=p4-1;
					p5=p6-1;
					while(p2>pCompare)
					{
						*p2=*p1;
						p1--;
						p2--;
						*p4=*p3;
						p3--;
						p4--;
						*p6=*p5;
						p5--;
						p6--;
					}
					*p2=distance;
					*p4=InnerCode;
					*p6=*pModelIdx;		
				}
				else if(p2==p1)
				{
					*p2=distance;
					*p4=InnerCode;
					*p6=*pModelIdx;
				}
				else
				{
					/*前面已有该字的另一模板*/
				}
				pModelIdx++;
			}
		}
//		if( nLeafNode > pGlobalParameter->Speed_const2)
//			break;
	}

	/*
	 *	调用下面的这个函数进行后处理
	 *	当前后处理的内容是进行字频修正，即对字频较高的字向前调整
	 */
	WTPostProcess(pCandiDistance,CandidateResult,NumberOfCandidate,pSave,pGlobalParameter->pModelFrequency);
	/*
	 *	如果某一候选在用户库中，则由于不必再在用户库中写入多内码，因此，与该候选对应的
	 *	模板序号可以不必考虑
	 */	
	if(pGlobalParameter->SelfAdaptEnabled)
	{	
		//	样本与自适应模板及未鉴别的初始模板进行比较，如果自适应模板距离更小的话，则认为自适应模板有效
		unsigned short minDistOfModel=STATIS_CORE_LARGE_DISTANCE,ValidUserModel=0xffff;
		for( i=0;i<NumberOfCandidate;i++)
		{
			pModelFeaturePos=(signed char *)(pGlobalParameter->pOrignModelFeature+(pSave[i]*pGlobalParameter->g_dimComp));
			distance=0;
			pFeatVector=pCompressedFeature;
			do
			{
				OneDimDist=(*pFeatVector) - (short)(*pModelFeaturePos);
				if(OneDimDist&0xff00)
				{
					distance=minDistOfModel+1;
					break;
				}
				distance+=*(pTableForMatching+OneDimDist);
				pModelFeaturePos++;
				pFeatVector++;
			}while(pFeatVector<pCompareVector);
			if(distance<minDistOfModel)
			{
				minDistOfModel=distance;
			}
		}
		for(i=0;i<(*(pGlobalParameter->pNumberOfUsrModel));i++)
		{
			pModelFeaturePos=(signed char *)((pGlobalParameter->pFeatureOfUsrModel)+i*(pGlobalParameter->g_dimComp));
			distance=0;
			pFeatVector=pCompressedFeature;
			pCompareVector=pFeatVector+pGlobalParameter->g_dimComp;

			while(pFeatVector<pCompareVector)
			{
				OneDimDist=(*pFeatVector)-(short)(*pModelFeaturePos);
				if(OneDimDist&0xff00)
				{
					distance=minDistOfModel+1;
					break;
				}
				distance+=*(pTableForMatching+OneDimDist);
				pFeatVector++;
				pModelFeaturePos++;
			} 
			if(distance>=minDistOfModel)
			{
				continue;
			}
			else	/*如果距离小于最后一个候选，则插入进候选的十个字中*/
			{
				minDistOfModel=distance;
				ValidUserModel=i;
			}
		}
		if(ValidUserModel!=0xffff)
		{	//	识别首选修改为用户模板对应的类
			p1=pCandiDistance;
			/* 
			 * 按照距离，当前的模板应插入到p1的位置，但在多模板的情况下，
			 * 如果p1前已有该类的模板，则保留原模板作为候选
			 * 如果p1处也是该类的模板，则将与该模板的距离值改为当前距离
			 * 如果该类的模板在p1后的p2处，或在候选中还不存在该类的模板
			 * 则在p1处插入当前模板，并将p1-p2-1处的模板向后移一位
			 */
			InnerCode=pGlobalParameter->pActiveCodeTableOfUsrModel[ValidUserModel];
			if(InnerCode!=0)
			{
				p4=CandidateResult;
				pCompare=CandidateResult+(NumberOfCandidate-1);
				while(p4<pCompare)
				{	
					if(InnerCode==(*p4))
						break;
					p4++;
				}
				p2=pCandiDistance+(p4-CandidateResult);
				p6=pSave+(p4-CandidateResult);

				if(p2>p1)	
				{
					pCompare=p1;	
					p1=p2-1;
					p3=p4-1;
					p5=p6-1;
					while(p2>pCompare)
					{
						*p2=*p1;
						p1--;
						p2--;
						*p4=*p3;
						p3--;
						p4--;
						*p6=*p5;
						p5--;
						p6--;
					}
					*p2=distance;
					*p4=InnerCode;
				}
				else if(p2==p1)
				{
					*p2=distance;
					*p4=InnerCode;
				}
				else
				{
					/*前面已有该字的另一模板*/
				}
			}
		}
	}
	/*
	 *	保存最终的识别结果以便用于用户自适应库的生成处理
	 */
	pSave=pSave+NumberOfCandidate;		
	for(i=0;i<NumberOfCandidate;i++)
	{
		pSave[i]=CandidateResult[i];
	}
	pSave-=NumberOfCandidate;

#ifdef __FOR_DEMO_SHOW_IN_RESULT__
	CandidateResult[5]='_ ';
	CandidateResult[6]='D ';
	CandidateResult[7]='E ';
	CandidateResult[8]='M ';
	CandidateResult[9]='O ';
#endif

	/*
	 *	调用下面的这个函数来进行反盗版
	 */
//	if(pGlobalParameter->RequiredCharset&0x3E00)
//		WTAntiForge(pCompressedFeature,CandidateResult,pCandiDistance,pGlobalParameter);
	return ;
}
TH_EXPORT_API WTError WTSetCandidateNumber(unsigned short NumberOfCandidate,char *pRamAddress)
{
	WTParameter *pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);
	/*
	 *	设置最大可能的候选数为30，更多的候选数(只要小于127)也是可以的，但程序速度会太慢
	 */
	if(NumberOfCandidate>30)
		NumberOfCandidate=30;
	pGlobalParameter->NumberOfCandidate=NumberOfCandidate;

	return WTSuccess;
}
// original interface using script type of "unsigned char"
TH_EXPORT_API WTError WTRecognize(unsigned char * PointData,short PointsNumber,unsigned short *CandidateResult,char *pRamAddress)
{
	WTParameter *pGlobalParameter;
	if(*PointData==0xff&&*(PointData+1)==0xff)
		return WTFail;
	pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);	
	if(pGlobalParameter->FeatureType==0x101)
	{
		//	使用RamAddress的前512个short型空间来存放特征
		ExtractOnlineFourDirectionFeature(PointData,0,(short *)pRamAddress,pRamAddress);
	}
	else if(pGlobalParameter->FeatureType==0x103)
	{
		ExtractOnlineEightDirectionFeature(PointData,0,(short *)pRamAddress,pRamAddress);
	}
	PDA_Classifier_Process(CandidateResult,pRamAddress);
	return WTSuccess;	
}
//	unsigned short格式->unsigned char格式
//	因为存在溢出问题，所以在需要时进行线性归一化
void POT_UnsignedShort2UnsignedChar(unsigned short *PointData,short PointsNumber,unsigned char *NormedPointData)
{
	unsigned short start_x,end_x,start_y,end_y,*pAxis_X;
	unsigned char *pNormedData;
	int x,y;
	
	pAxis_X=PointData;
	end_x=start_x=*pAxis_X;
	end_y=start_y=*(pAxis_X+1);
	/*得到最大和最小值，也就是边框大小*/
	while(*pAxis_X!=0xffff||*(pAxis_X+1)!=0xffff)
	{
		if(*pAxis_X!=0xffff||*(pAxis_X+1)!=0)
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
	/*如果过于狭长，则保持原长宽比放大到0~255，否则将长宽均规一化到0~255*/
	pAxis_X=PointData;
	pNormedData=NormedPointData;

	if(x<255&&y<255)
	{
		while(*pAxis_X!=0xffff||*(pAxis_X+1)!=0xffff)
		{
			if(*pAxis_X!=0xffff||*(pAxis_X+1)!=0)
			{
				*pNormedData=((*pAxis_X-start_x));
				*(pNormedData+1)=((*(pAxis_X+1)-start_y));
			}
			else
			{
				*pNormedData=0xff;
				*(pNormedData+1)=0;
			}
			pAxis_X+=2;
			pNormedData+=2;
		}
	}
	else
	{
		while(*pAxis_X!=0xffff||*(pAxis_X+1)!=0xffff)
		{
			if(*pAxis_X!=0xffff||*(pAxis_X+1)!=0)
			{
				*pNormedData=((*pAxis_X-start_x)*255/x);
				*(pNormedData+1)=((*(pAxis_X+1)-start_y)*255/y);
			}
			else
			{
				*pNormedData=0xff;
				*(pNormedData+1)=0;
			}
			pAxis_X+=2;
			pNormedData+=2;
		}
	}
	*pNormedData=0xff;
	*(pNormedData+1)=0xff;
	return;
}
//	a new inferface using script type of "unsigned short"
TH_EXPORT_API WTError WTRecognizeUShort(unsigned short * PointData,short PointsNumber,unsigned short *CandidateResult,char *pRamAddress)
{
	if( PointsNumber== 0 )
		return WTFail;	//如果没有数据点，直接返回
	if(*PointData==0xffff&&*(PointData+1)==0xffff)
		return WTFail;
	//	检查是否要进行归一化处理
	POT_UnsignedShort2UnsignedChar(PointData,PointsNumber,(unsigned char *)PointData);
	//	调用"unsigned char"型接口进行处理
	return WTRecognize((unsigned char *)PointData,PointsNumber,CandidateResult,pRamAddress);
}
TH_EXPORT_API WTError WTRecognizeEx(unsigned char * PointData,short PointsNumber,short NumberOfCandidate,unsigned short *CandidateResult,unsigned short *CandidateDistance,char *pRamAddress)
{
	WTSetCandidateNumber(NumberOfCandidate,pRamAddress);
	WTRecognize(PointData,PointsNumber,CandidateResult,pRamAddress);
	if(CandidateDistance!=0)
	{	/*	复制距离到指定的变量中 */
		short i;
		unsigned short *pTempUS=(unsigned short *)pRamAddress;
		for(i=0;i<NumberOfCandidate;i++)
			CandidateDistance[i]=pTempUS[i];
	}
#ifdef RECORD_POT

	unsigned short BlockSize,code,NumberOfByteOccupied;

	NumberOfByteOccupied = PointsNumber*2;
    BlockSize = NumberOfByteOccupied+6;
	code = CandidateResult[0];

	if(fpPot!=NULL)
	{
	fwrite(&BlockSize,sizeof(unsigned short),1,fpPot);
	fwrite(&code,sizeof(unsigned short),1,fpPot);
	unsigned short a=0;
	fwrite(&a,sizeof(unsigned short),1,fpPot);

	fwrite(PointData,sizeof(unsigned char),NumberOfByteOccupied,fpPot);
	}
#endif


	return WTSuccess;	
}
TH_EXPORT_API WTError WTRecognizeUShortEx(unsigned short * PointData,short PointsNumber,short NumberOfCandidate,unsigned short *CandidateResult,unsigned short *CandidateDistance,char *pRamAddress)
{
	if( PointsNumber== 0 )
		return WTFail;	//如果没有数据点，直接返回
	if(*PointData==0xffff&&*(PointData+1)==0xffff)
		return WTFail;
	//	检查是否要进行归一化处理
	POT_UnsignedShort2UnsignedChar(PointData,PointsNumber,(unsigned char *)PointData);
	//	调用"unsigned char"型接口进行处理
	return WTRecognizeEx((unsigned char *)PointData,PointsNumber,NumberOfCandidate,CandidateResult,CandidateDistance,pRamAddress);	
}
TH_EXPORT_API WT_int32 WTGetRamSize()
{
	return __PDA_GLOBAL_RAM_SIZE;
}

/*==========================================================================
*	Prototype:	WTError WTRecognizeInit(char *pRamAddress,short RamSize,char *LibStartAddress);
*	Purpose:	进行识别系统的初始化，包括将各个参量指向正确的内存地址或赋值
*	Parameters:	->pRamAddress	动态RAM地址，该块区域可以用于程序中的数据交换
*				->RamSize		动态RAM区域的大小，程序中所用空间不能超出其边界
*				->LibStartAddress	识别库的首地址
*	Return:		0 if success;1 else.
*	Comments:
===========================================================================*/
TH_EXPORT_API WTError WTRecognizeInit(char *pRamAddress,WT_int32 RamSize,char *LibStartAddress)
{
	char * pAddress;
	short i,Version,bContainModelFunctionalInfo;
	unsigned short g_nTotalClass;/*总的类别个数*/
	WT_int32 *pSectionPositionInLib;
	WTParameter *pGlobalParameter;
	pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);
	pGlobalParameter->pLibAddress=LibStartAddress;

	pSectionPositionInLib=(WT_int32 *)LibStartAddress;

	/*
	 *	设置一些缺省值
	 */
	pGlobalParameter->RequiredCharset=0x00000200;	/*	识别候选范围	*/
	pGlobalParameter->WTInnerCodeType=0x0004;		/*	输出内码类型	*/
	pGlobalParameter->Speed_const1=6;				/*	速度控制参数1	*/
	pGlobalParameter->Speed_const2=1400;			/*	速度控制参数2	*/
	pGlobalParameter->NumberOfCandidate=10;			/*	识别候选数目	*/
	pGlobalParameter->LDA_LOG2MultiValue=8;			/*	LDA变换阵的倍乘系数	*/
	pGlobalParameter->SelfAdaptEnabled=0;			/*	禁用用户自适应	*/
	pGlobalParameter->pOrignModelFeature=0;			/*	指向库中初始模板特征,如果没有，该值为0	*/
	pGlobalParameter->UsrLibAddress=0;				/*	指向用户库地址，如果没有初始化用户库，该值为0	*/
	pGlobalParameter->Use32BitCharset=0;			/*	不使用32bit字符集范围	*/
	pGlobalParameter->FeatureType=0;				/*	不知道特征类型	*/
	pGlobalParameter->pModelFunctionalInfo=0;

	bContainModelFunctionalInfo=0;
	/*
	 *	判断库的Endian类型是否正确及各个Section是否齐备
	 */
	if(pSectionPositionInLib[0]!=0x28)
		return STA_InvalidEndianType;	
	if(pSectionPositionInLib[3]>=pSectionPositionInLib[4])
		return STA_NoLookupTableSection;
	if(pSectionPositionInLib[4]>=pSectionPositionInLib[5])
		return STA_NoModelInfoSection;
///	if(pSectionPositionInLib[5]>=pSectionPositionInLib[6])
//		return STA_NoAntiForgeSection;
	if(pSectionPositionInLib[8]>=pSectionPositionInLib[9])
		return STA_NoInformationSection;

	/*
	 *	InformationSection
	 */
	pGlobalParameter->pGeneralInformationSection=(short *)(LibStartAddress+pSectionPositionInLib[8]);
	/*	版本号	*/
	Version=pGlobalParameter->pGeneralInformationSection[1];
	/*	计算出库所支持的输出内码种类	*/
	pGlobalParameter->NumberOfSupportedInnerCodeType=0;
	for(i=0;i<16;i++)
	{
		if(pGlobalParameter->pGeneralInformationSection[2]&(1<<i))
			pGlobalParameter->NumberOfSupportedInnerCodeType++;
	}
	pGlobalParameter->SupportedCharset=pGlobalParameter->pGeneralInformationSection[4];		/*	高16位	*/
	pGlobalParameter->SupportedCharset=pGlobalParameter->SupportedCharset<<16;
	pGlobalParameter->SupportedCharset+=pGlobalParameter->pGeneralInformationSection[3];	/*	低16位	*/
	
	i=((short)(pSectionPositionInLib[9]-pSectionPositionInLib[8]))>>1;	/*	有效的Information个数	*/
	if(i>=6)	/*	存在倍乘系数	*/
		pGlobalParameter->LDA_LOG2MultiValue=pGlobalParameter->pGeneralInformationSection[5]+8;

	if(i>=7)
	{
		if((pGlobalParameter->pGeneralInformationSection[6])&0x0002)	//support 32bit charset
			pGlobalParameter->Use32BitCharset=1;
		if((pGlobalParameter->pGeneralInformationSection[6])&0x0010)	//support self adapt
			pGlobalParameter->pOrignModelFeature=LibStartAddress+pSectionPositionInLib[6];
		if((pGlobalParameter->pGeneralInformationSection[6])&0x0020)	//support model functional info
			bContainModelFunctionalInfo=1;
	}
	if(i>=8)
		pGlobalParameter->FeatureType=pGlobalParameter->pGeneralInformationSection[7];

	/* 
	 *	Dictionary Section
	 */
	pGlobalParameter->g_nTotalModel=*((unsigned short *)(LibStartAddress+pSectionPositionInLib[0]));
	g_nTotalClass=*((unsigned short *)(LibStartAddress+pSectionPositionInLib[0]+2));
	pGlobalParameter->g_dimFeat=*((short *)(LibStartAddress+pSectionPositionInLib[0]+4));
	pGlobalParameter->g_dimComp=*((short *)(LibStartAddress+pSectionPositionInLib[0]+6));
	pGlobalParameter->g_pFeatureTranslationVector=(short *)(LibStartAddress+pSectionPositionInLib[0]+8);
	pAddress=LibStartAddress+pSectionPositionInLib[0]+8+2*pGlobalParameter->g_dimFeat*pGlobalParameter->g_dimComp;
	if(Version<0x0400)	//旧的版本2、3的库
		pAddress+=g_nTotalClass;
	else if(Version<0x0440)	//440以前的库这儿是表示每个字符集范围的起始与终了模板序号
		pAddress+=66;
	else
		pAddress+=64;	//这儿是表示每个字符集范围的ID
	pGlobalParameter->pModelFeature=(char *)(pAddress);
	
	if(pGlobalParameter->FeatureType==0)	//旧的库中没有设置特征类型，根据pGlobalParameter->g_dimFeat来决定缺省的特征类型
	{
		if(pGlobalParameter->g_dimFeat==512)
			pGlobalParameter->FeatureType=0x101;
		else if(pGlobalParameter->g_dimFeat==1416)
			pGlobalParameter->FeatureType=0x102;
		else if(pGlobalParameter->g_dimFeat==392)
			pGlobalParameter->FeatureType=0x201;
		else if(pGlobalParameter->g_dimFeat==200)
			pGlobalParameter->FeatureType=0x202;
		else 
			return STA_UnknownFeatureType;
	}
	if((pGlobalParameter->FeatureType&0xFF00)!=0x0100)
	{
		return STA_InvalidFeatureType;
	}
	if(pGlobalParameter->FeatureType!=0x101&&pGlobalParameter->FeatureType!=0x103)
	{	//当前程序中抽取特征的函数仅支持这两种特征类型
		return STA_InvalidFeatureType;
	}
	/* 
	 *	Tree Section
	 */
	pAddress=LibStartAddress+pSectionPositionInLib[1];
	pGlobalParameter->g_nTreeNodes=*((short *)pAddress);	
	pAddress+=2;
	pGlobalParameter->pCenterFeature=(char *)(pAddress);
	pAddress+=pGlobalParameter->g_nTreeNodes*pGlobalParameter->g_dimComp;
	pGlobalParameter->pTableIndex=(unsigned short *)(pAddress);
	//	寻找越界的pTableIndex
	pGlobalParameter->OutRangeTreeNode=pGlobalParameter->g_nTreeNodes+1;
	for(i=1;i<pGlobalParameter->g_nTreeNodes+1;i++)
	{
		if(pGlobalParameter->pTableIndex[i]<pGlobalParameter->pTableIndex[i-1])
		{
			pGlobalParameter->OutRangeTreeNode=i;	//如果后一个树节点Index比前一个小，则是unsigned short越界，需要加上65536
			break;
		}
	}
	pAddress+=2*(pGlobalParameter->g_nTreeNodes+1);
	pGlobalParameter->pTreeTable=(unsigned short *)(pAddress);
	
	/*
	 *	CodeTable Section
	 */
	pGlobalParameter->pModelSequenceToInnerCode=(unsigned short *)(LibStartAddress+pSectionPositionInLib[2]);
	pGlobalParameter->pActiveModelSequenceToInnerCode=pGlobalParameter->pModelSequenceToInnerCode;
	
	/*
	 *	LookupTableSection
	 */
	pAddress=LibStartAddress+pSectionPositionInLib[3];
	pGlobalParameter->CosValue=(unsigned char *)(pAddress);
	pAddress+=90;
	pGlobalParameter->TanTable=(unsigned short *)(pAddress);
	pAddress+=90;
	pGlobalParameter->SquareTable=(unsigned short *)(pAddress);
	pAddress+=364;	/*	364=2*182 */
	pGlobalParameter->TableForMatching=(unsigned short *)(pAddress);

	/*
	 *	Model Info Section
	 */
	pGlobalParameter->pModelFrequency=(short *)(LibStartAddress+pSectionPositionInLib[4]);
	if(pGlobalParameter->Use32BitCharset)
	{
		pGlobalParameter->pBelongCharset32=(WT_int32 *)(pGlobalParameter->pModelFrequency+pGlobalParameter->g_nTotalModel);
		if(bContainModelFunctionalInfo==1)
			pGlobalParameter->pModelFunctionalInfo=(short *)(pGlobalParameter->pBelongCharset32+pGlobalParameter->g_nTotalModel);
	}
	else
	{
		pGlobalParameter->pBelongCharset=(short *)(pGlobalParameter->pModelFrequency+pGlobalParameter->g_nTotalModel);
		if(bContainModelFunctionalInfo==1)
			pGlobalParameter->pModelFunctionalInfo=(short *)(pGlobalParameter->pBelongCharset+pGlobalParameter->g_nTotalModel);
	}
	
	/*
	 *	Anti-Forge Section
	 */
	pGlobalParameter->pAntiForgeSection=(char *)(LibStartAddress+pSectionPositionInLib[5]);


#ifdef RECORD_POT

	fpPot = fopen("c:\\yanghu\\recog.pot","ab");
#endif

	return WTSuccess;
}

TH_EXPORT_API WTError WTRecognizeEnd(char *pRamAddress)
{
	return WTSuccess;
}
/*
 *	根据给定的用户最大模板数目，算出需要开辟的用户库空间大小
 */
WT_int32 WTGetSelfAdaptLibSize(WT_int32 NumberOfModel,char *pRamAddress)
{
	WTParameter *pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);
	WT_int32 SelfAdaptRamSize=0;;

	SelfAdaptRamSize+=0x00000028;	//section 0
	SelfAdaptRamSize+=4+sizeof(unsigned short)*NumberOfModel	
			+sizeof(char)*NumberOfModel*pGlobalParameter->g_dimComp;	//section 1
	SelfAdaptRamSize+=sizeof(short)*(pGlobalParameter->g_nTreeNodes+1)+sizeof(short)*NumberOfModel;		//section 2 
	SelfAdaptRamSize+=sizeof(unsigned short)*NumberOfModel*pGlobalParameter->NumberOfSupportedInnerCodeType;	//section 3
	SelfAdaptRamSize+=2*sizeof(WT_int32)*NumberOfModel;	//section 4
	SelfAdaptRamSize+=8;	//section 9

	return SelfAdaptRamSize;
}
/*
 *	根据给定的空间大小，算出所支持的模板数目
 */	
WT_int32 WTGetSelfAdaptMaxAllowedModel(WT_int32 GivenLibSize,char *pRamAddress)
{
	WT_int32 RequiredLibSize,ModelNumber,minModelNumber=0,maxModelNumber=_MAX_NUMBER_OF_USR_MODEL;
	ModelNumber=maxModelNumber;
	do{
		RequiredLibSize=WTGetSelfAdaptLibSize(ModelNumber,pRamAddress);
		if(RequiredLibSize>GivenLibSize)
		{	//空间不够，减少模板数
			maxModelNumber=ModelNumber;
			ModelNumber=(minModelNumber+maxModelNumber)>>1;
		}
		else if(RequiredLibSize==GivenLibSize)
			break;
		else if(RequiredLibSize<GivenLibSize)
		{
			minModelNumber=ModelNumber;
			ModelNumber=(minModelNumber+maxModelNumber)>>1;
		}
	}while(minModelNumber!=ModelNumber);
	return ModelNumber;
}
//创建用户自适应库
WTError WTCreateSelfAdaptLib(char *UserLibAddress,WT_int32 UserLibSize,WT_int32 NumberOfUsrModel,char *pRamAddress)
{
	short i,*pUsrInformationSection;
	unsigned short *pTempUS;
	WT_int32 *pSectionPositionInLib,*pTempLong;
	WTParameter *pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);

	pGlobalParameter->UsrLibAddress=UserLibAddress;	/*	UsrLibAddress指向用户库的起始地址	 */
	pGlobalParameter->UsrRamSize=UserLibSize;		/*	用户库的大小	*/

	pSectionPositionInLib=(WT_int32 *)(pGlobalParameter->UsrLibAddress);

	pSectionPositionInLib[0]=0x00000028;
	pSectionPositionInLib[1]=pSectionPositionInLib[0]+4+sizeof(unsigned short)*NumberOfUsrModel
			+sizeof(char)*NumberOfUsrModel*pGlobalParameter->g_dimComp;
	pSectionPositionInLib[2]=pSectionPositionInLib[1]+sizeof(short)*(pGlobalParameter->g_nTreeNodes+1)
			+sizeof(short)*NumberOfUsrModel;
	pSectionPositionInLib[3]=pSectionPositionInLib[2]+sizeof(unsigned short)*NumberOfUsrModel*pGlobalParameter->NumberOfSupportedInnerCodeType;
	pSectionPositionInLib[4]=pSectionPositionInLib[3]+2*sizeof(WT_int32)*NumberOfUsrModel;
	for(i=5;i<9;i++)
		pSectionPositionInLib[i]=pSectionPositionInLib[3];
	pSectionPositionInLib[9]=pSectionPositionInLib[8]+8;

	/*
	 *	指向UsrDictionary Section
	 */
	pTempUS=(unsigned short *)(pGlobalParameter->UsrLibAddress+pSectionPositionInLib[0]);
	pTempUS[0]=0;					//	库中已有多少个用户模板
	pTempUS[1]=NumberOfUsrModel;	//	最多允许的用户模板数
	pGlobalParameter->pNumberOfUsrModel=pTempUS;
	/*
	 *	指向Model Usage Section
	 */
	pTempLong=(WT_int32 *)(pGlobalParameter->UsrLibAddress+pSectionPositionInLib[3]);
//	for(i=0;i<NumberOfUsrModel;i++)
//		pTempLong[i]=0;		//Model Usage

	/*
	 *	指向Informatioin Section
	 */
	pUsrInformationSection=(short *)(pGlobalParameter->UsrLibAddress+pSectionPositionInLib[8]);
	if(pGlobalParameter->pGeneralInformationSection!=0)
	{	//	如果通用库中存在Information Section，则以此内容为参照写入用户库的Information Section
		pUsrInformationSection[0]=pGlobalParameter->pGeneralInformationSection[0];
		pUsrInformationSection[1]=pGlobalParameter->pGeneralInformationSection[1];
		pUsrInformationSection[2]=pGlobalParameter->pGeneralInformationSection[2];
	}
	else	
	{	//	如果通用库中不存在Information Section,则用户库中的Information Section的创建可能有BUG
		pUsrInformationSection[0]=0;		/*应该是何值??? */
		pUsrInformationSection[1]=0x400;	/*应该是何值??? */
		pUsrInformationSection[2]=0x2;		/*应该是何值??? */
	}
	return WTSuccess;
}
/*
 *	初始化自适应库，这包括两个方面的操作，如果没有自适应库，则创建一个全新的自适应库；
 *	如果有自适应库，则进行自适应库的读取；
 */
WTError WTInitSelfAdaptLib(char *UserLibAddress,WT_int32 UserLibSize,char *pRamAddress)
{
	//	应该根据UserLibSize算出最多允许的模板数目
	short *pUsrInformationSection;
	unsigned short MaxNumberOfUsrModel;
	WT_int32 *pSectionPositionInLib;
	WTParameter *pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);

	pGlobalParameter->UsrLibAddress=UserLibAddress;	/*	UsrLibAddress指向用户库的起始地址	 */
	pGlobalParameter->UsrRamSize=UserLibSize;		/*	用户库的大小	*/

	pSectionPositionInLib=(WT_int32 *)(pGlobalParameter->UsrLibAddress);
	/*
	 *	校验用户库是否有效，如果无效，则创建新的用户库
	 */
	if(pSectionPositionInLib[0]!=0x00000028)
	{
		MaxNumberOfUsrModel=WTGetSelfAdaptMaxAllowedModel(UserLibSize,pRamAddress);	//算出允许的用户模板数
		WTCreateSelfAdaptLib(UserLibAddress,UserLibSize,MaxNumberOfUsrModel,pRamAddress);	//以此模板数创建用户初始库
	}
	else
	{
		/*
		 *	读取用户库参数
		 */
		pGlobalParameter->pNumberOfUsrModel=(unsigned short *)(pGlobalParameter->UsrLibAddress+pSectionPositionInLib[0]);
		MaxNumberOfUsrModel=*((unsigned short *)(pGlobalParameter->UsrLibAddress+pSectionPositionInLib[0]+2));
		pGlobalParameter->pFeatureOfUsrModel=(char *)(pGlobalParameter->UsrLibAddress+pSectionPositionInLib[0]+4+sizeof(unsigned short)*MaxNumberOfUsrModel);

		/*
		 *	for the compatible with the old version recognition libarary,
		 *	legal check are required.
		 */
		if(pSectionPositionInLib[8]==pSectionPositionInLib[9])	
		{
			/* the Information Section does not exist ,only one InnerCode type are supported */
			return STA_CanNotSetInnerCodeType;
		}
	}
	/*	point to Information Section */
	pUsrInformationSection=(short *)(pGlobalParameter->UsrLibAddress+pSectionPositionInLib[8]);
	/*
	 *	下面将设置用户模板序数到输出内码的转换表
	 */
	pGlobalParameter->pCodeTableOfUsrModel=(unsigned short *)(pGlobalParameter->UsrLibAddress+pSectionPositionInLib[2]);
	pGlobalParameter->pActiveCodeTableOfUsrModel=pGlobalParameter->pCodeTableOfUsrModel;

	if((pGlobalParameter->WTInnerCodeType)&0x0001)	/* require GBK */
	{
		if((pUsrInformationSection[2])&0x0001)
			return WTSuccess;	/* support GBK */
		else
			return STA_InnerCodeTypeNotSupported;
	}
	if((pGlobalParameter->WTInnerCodeType)&0x0002)	/* require BIG5 */
	{
		if(pUsrInformationSection[2]&0x0001)	/* support GBK */
		{
			pGlobalParameter->pActiveCodeTableOfUsrModel+=MaxNumberOfUsrModel;	/* jump the code table for GBK */
		}
		if(pUsrInformationSection[2]&0x0002) /* support BIG5 */
			return WTSuccess;
		else
			return STA_InnerCodeTypeNotSupported;
	}
	if((pGlobalParameter->WTInnerCodeType)&0x0004)	/* require Unicode (2-byte version )*/
	{
		if(pUsrInformationSection[2]&0x0001)	/*	support GBK */
		{
			pGlobalParameter->pActiveCodeTableOfUsrModel+=MaxNumberOfUsrModel;	/* jump the code table for GBK */
		}
		if(pUsrInformationSection[2]&0x0002)	/*	support BIG5 */
		{
			pGlobalParameter->pActiveCodeTableOfUsrModel+=MaxNumberOfUsrModel;	/* jump the code table for BIG5 */
		}
		if(pUsrInformationSection[2]&0x0004)	/*	support Unicode */
			return WTSuccess;
		else
			return STA_InnerCodeTypeNotSupported;
	}
	return WTSuccess;
}
/*
 *	设置是否允许自适应识别
 */
TH_EXPORT_API WTError WTEnableSelfAdapt(short SelfAdapt,char *UserLibAddress,WT_int32 UserLibSize,char *pRamAddress)
{
	WTError rt;

	WTParameter *pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);
	switch(SelfAdapt)
	{
	case SELF_ADAPT_DISABLE:		//不进行自适应识别，不进行用户自适应库的初始化
		pGlobalParameter->SelfAdaptEnabled=0;
		rt=WTSuccess;
		break;
	case SELF_ADAPT_RECOGNIZE_AND_INIT_LIB:	//进行自适应识别，进行用户自适应库的初始化
		pGlobalParameter->SelfAdaptEnabled=1;	
		rt=WTInitSelfAdaptLib(UserLibAddress,UserLibSize,pRamAddress);
		break;
	case SELF_ADAPT_INIT_LIB_ONLY:	//不进行自适应识别，但进行用户自适应库的初始化
		pGlobalParameter->SelfAdaptEnabled=0;
		rt=WTInitSelfAdaptLib(UserLibAddress,UserLibSize,pRamAddress);
		break;
	case SELF_ADAPT_RECOGNIZE_ONLY:	//进行自适应识别，但不进行用户自适应库的初始化(此时需要有其它方式给出自适应库的信息)
		pGlobalParameter->SelfAdaptEnabled=1;
		rt=WTSuccess;
		break;
	default:
		rt=WTFail;
		break;
	}
	if(pGlobalParameter->pOrignModelFeature==0)
	{
		if(pGlobalParameter->SelfAdaptEnabled==1)
		{
			pGlobalParameter->SelfAdaptEnabled=0;	//	如果当前库不支持自适应，则不能启用自适应功能	
			rt=STA_SelfAdaptNotSupported;
		}
	}
	return rt;
}
/*
 *	设置识别范围
 */
TH_EXPORT_API WTError WTSetRange(WT_int32 Range,char *pRamAddress)
{
	WTParameter *pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);
#ifdef __FOR_DEMO_CHN_LEVEL1__
	Range=0x00000200;	/*	如果是用于Demo，则强制识别候选范围为一级字	*/
#elif defined(__FOR_DEMO_CHN_LEVEL1_PLUS__)
	Range&=0x000002FF;
#elif defined(__FOR_DEMO_CHN_LEVEL2__)
	Range=0x00000600;
#elif defined(__FOR_DEMO_CHN_LEVEL2_PLUS__)
	Range&=0x000006FF;
#endif

	if(pGlobalParameter->Use32BitCharset)
		pGlobalParameter->RequiredCharset=Range;
	else
		pGlobalParameter->RequiredCharset=Range&0x0000ffff;

	return WTSuccess;
}
/*
 *	设置识别速度，这是通过设置两个可以控制速度的参数来完成的
 */
TH_EXPORT_API WTError WTSetSpeed(short Speed,char *pRamAddress)
{
	WTParameter *pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);
	switch(Speed)
	{
	case 0:		pGlobalParameter->Speed_const1=12;		pGlobalParameter->Speed_const2=2700;		break;		/*识别率最高*/
	case 1:		pGlobalParameter->Speed_const1=10;		pGlobalParameter->Speed_const2=2200;		break;
	case 2:		pGlobalParameter->Speed_const1=9;		pGlobalParameter->Speed_const2=2000;		break;
	case 3:		pGlobalParameter->Speed_const1=8;		pGlobalParameter->Speed_const2=1800;		break;
	case 4:		pGlobalParameter->Speed_const1=7;		pGlobalParameter->Speed_const2=1600;		break;
	case 5:		pGlobalParameter->Speed_const1=6;		pGlobalParameter->Speed_const2=1400;		break;
	case 6:		pGlobalParameter->Speed_const1=5;		pGlobalParameter->Speed_const2=1200;		break;
	case 7:		pGlobalParameter->Speed_const1=4;		pGlobalParameter->Speed_const2=1000;		break;
	case 8:		pGlobalParameter->Speed_const1=3;		pGlobalParameter->Speed_const2=800;			break;
	case 9:		pGlobalParameter->Speed_const1=2;		pGlobalParameter->Speed_const2=600;			break;
	case 10:	pGlobalParameter->Speed_const1=1;		pGlobalParameter->Speed_const2=200;			break;		/*速度最快*/
	case 999:	pGlobalParameter->Speed_const1=120;		pGlobalParameter->Speed_const2=20000;		break;		/*速度最慢,不带分类树*/
	default:	pGlobalParameter->Speed_const1=6;		pGlobalParameter->Speed_const2=1400;	
	}
	return WTSuccess;
}
#ifdef USE_OLD_INTERFACE
TH_EXPORT_API WTError WTSetInnerCode(short InnerCodeType,char *LibStartAddress,char *pRamAddress)
{
	WTParameter *pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);
	WT_int32 *pSectionPositionInLib;

	pGlobalParameter->WTInnerCodeType=InnerCodeType;
	pSectionPositionInLib=(WT_int32 *)LibStartAddress;

	/*
	 *	point to CodeTable Section 
	 */
	pGlobalParameter->pActiveModelSequenceToInnerCode=(unsigned short *)(LibStartAddress+pSectionPositionInLib[2]);
	/*
	 *	for the compatible with the old version recognition libarary,
	 *	legal check are required.
	 */
	if(pSectionPositionInLib[8]==pSectionPositionInLib[9])	
	{
		/* the Information Section does not exist ,only one InnerCode type are supported */
		return CanNotSetInnerCodeType;
	}
	/* 
	 *	point to Information Section 
	 */
	pGlobalParameter->pGeneralInformationSection=(short *)(LibStartAddress+pSectionPositionInLib[8]);
	
	if(pGlobalParameter->WTInnerCodeType&0x0001)	/* require GBK */
	{
		if(pGlobalParameter->pGeneralInformationSection[2]&0x0001)
			return WTSuccess;	/* support GBK */
		else
			return InnerCodeTypeNotSupported;
	}
	if(pGlobalParameter->WTInnerCodeType&0x0002)	/* require BIG5 */
	{
		if(pGlobalParameter->pGeneralInformationSection[2]&0x0001)	/* support GBK */
		{
			pGlobalParameter->pActiveModelSequenceToInnerCode+=pGlobalParameter->g_nTotalModel;	/* jump the code table for GBK */
		}
		if(pGlobalParameter->pGeneralInformationSection[2]&0x0002) /* support BIG5 */
			return WTSuccess;
		else
			return InnerCodeTypeNotSupported;
	}
	if(pGlobalParameter->WTInnerCodeType&0x0004)	/* require Unicode (2-byte version )*/
	{
		if(pGlobalParameter->pGeneralInformationSection[2]&0x0001)	/*	support GBK */
		{
			pGlobalParameter->pActiveModelSequenceToInnerCode+=pGlobalParameter->g_nTotalModel;	/* jump the code table for GBK */
		}
		if(pGlobalParameter->pGeneralInformationSection[2]&0x0002)	/*	support BIG5 */
		{
			pGlobalParameter->pActiveModelSequenceToInnerCode+=pGlobalParameter->g_nTotalModel;	/* jump the code table for BIG5 */
		}
		if(pGlobalParameter->pGeneralInformationSection[2]&0x0004)	/*	support Unicode */
			return WTSuccess;
		else
			return InnerCodeTypeNotSupported;
	}
	if(pGlobalParameter->SelfAdaptEnabled)
		WTEnableSelfAdapt(pGlobalParameter->SelfAdaptEnabled,pGlobalParameter->UsrLibAddress,pGlobalParameter->UsrRamSize,pRamAddress);
	return WTSuccess;
}
#else
TH_EXPORT_API WTError WTSetInnerCode(short InnerCodeType,char *pRamAddress)
{
	WTParameter *pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);

	pGlobalParameter->WTInnerCodeType=InnerCodeType;

	/*
	 *	point to CodeTable Section 
	 */
	pGlobalParameter->pActiveModelSequenceToInnerCode=pGlobalParameter->pModelSequenceToInnerCode;
	
	if(pGlobalParameter->WTInnerCodeType&0x0001)	/* require GBK */
	{
		if(pGlobalParameter->pGeneralInformationSection[2]&0x0001)
			return WTSuccess;	/* support GBK */
		else
			return STA_InnerCodeTypeNotSupported;
	}
	if(pGlobalParameter->WTInnerCodeType&0x0002)	/* require BIG5 */
	{
		if(pGlobalParameter->pGeneralInformationSection[2]&0x0001)	/* support GBK */
		{
			pGlobalParameter->pActiveModelSequenceToInnerCode+=pGlobalParameter->g_nTotalModel;	/* jump the code table for GBK */
		}
		if(pGlobalParameter->pGeneralInformationSection[2]&0x0002) /* support BIG5 */
			return WTSuccess;
		else
			return STA_InnerCodeTypeNotSupported;
	}
	if(pGlobalParameter->WTInnerCodeType&0x0004)	/* require Unicode (2-byte version )*/
	{
		if(pGlobalParameter->pGeneralInformationSection[2]&0x0001)	/*	support GBK */
		{
			pGlobalParameter->pActiveModelSequenceToInnerCode+=pGlobalParameter->g_nTotalModel;	/* jump the code table for GBK */
		}
		if(pGlobalParameter->pGeneralInformationSection[2]&0x0002)	/*	support BIG5 */
		{
			pGlobalParameter->pActiveModelSequenceToInnerCode+=pGlobalParameter->g_nTotalModel;	/* jump the code table for BIG5 */
		}
		if(pGlobalParameter->pGeneralInformationSection[2]&0x0004)	/*	support Unicode */
			return WTSuccess;
		else
			return STA_InnerCodeTypeNotSupported;
	}
	if(pGlobalParameter->SelfAdaptEnabled)
		WTEnableSelfAdapt(pGlobalParameter->SelfAdaptEnabled,pGlobalParameter->UsrLibAddress,pGlobalParameter->UsrRamSize,pRamAddress);
	return WTSuccess;
}
#endif
/*
 *	进行自适应库的处理
 */
TH_EXPORT_API WTError WTSelfAdapt(unsigned short CorrectResult,char *pRamAddress)
{
	char *pUsrModelFeature;
	unsigned short i,MaxNumberOfUsrModel,*pFIFOTable,tempUS,*pSave,CorrectModelSequence;
	WT_int32 *pSectionPositionInLib,*pModelLearningTimes,tempLong;
	short  NumberOfCandidate,*pCompressedFeature;		/*变换后的特征向量*/
	WTParameter *pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);

	if(pGlobalParameter->UsrLibAddress==0)
		return STA_UsrLibFileNotFound;
	if(pGlobalParameter->g_dimFeat>=512)	/*这样处理的目的是为了避免在原始特征维数较小时与后面的pCenterIndex，pSave等发生内存冲突*/
		pCompressedFeature=(short *)(pRamAddress+2*pGlobalParameter->g_dimFeat);	/*	变换后特征	*/
	else
		pCompressedFeature=(short *)(pRamAddress+2*512);	
	pSectionPositionInLib=(WT_int32 *)pGlobalParameter->UsrLibAddress;
	pSave=(unsigned short *)(pRamAddress+512);	/*	现在pSave指向所保存的候选集模板序数 */
	NumberOfCandidate=pGlobalParameter->NumberOfCandidate;
	/*
	 *	判断正确内码应是候选中的第几个
	 */
	for(i=0;i<NumberOfCandidate;i++)	
	{
		if(CorrectResult==pSave[i+NumberOfCandidate])
		{
			CorrectModelSequence=pSave[i];
			break;
		}
	}
	if(i==NumberOfCandidate)	/*	当前的正确内码并不在候选中，无法进行多内码添加，需要作另处的处理 */
	{
#ifdef WTDEBUG
//		return NotSupportedFunction1;
#else
//		return WTSuccess;
#endif
	}

	/*
	 *	读取用户库参数
	 */
	pGlobalParameter->pNumberOfUsrModel=(unsigned short *)(pGlobalParameter->UsrLibAddress+pSectionPositionInLib[0]);
	MaxNumberOfUsrModel=*((unsigned short *)(pGlobalParameter->UsrLibAddress+pSectionPositionInLib[0]+2));
	pFIFOTable=(unsigned short *)(pGlobalParameter->UsrLibAddress+pSectionPositionInLib[0]+4);
	pGlobalParameter->pFeatureOfUsrModel=(char *)(pGlobalParameter->UsrLibAddress+pSectionPositionInLib[0]+4+sizeof(unsigned short)*MaxNumberOfUsrModel);
	pUsrModelFeature=pGlobalParameter->pFeatureOfUsrModel;
	pModelLearningTimes=(WT_int32 *)(pGlobalParameter->UsrLibAddress+pSectionPositionInLib[3]+MaxNumberOfUsrModel*sizeof(WT_int32));
	/*
	 *	搜寻用户库，看当前类是否已在用户库中
	 */
	for(i=0;i<*(pGlobalParameter->pNumberOfUsrModel);i++)
	{
		if(CorrectResult==pGlobalParameter->pActiveCodeTableOfUsrModel[i])
			break;
	}
	/*	 如果当前的类不在用户库中	 */
	if(i==*(pGlobalParameter->pNumberOfUsrModel))
	{	
		if(*(pGlobalParameter->pNumberOfUsrModel)<MaxNumberOfUsrModel)
		{	/*	当前用户库中已有模板数小于最大类别数,直接添加该模板进去	 */
			tempUS=*(pGlobalParameter->pNumberOfUsrModel);
			(*(pGlobalParameter->pNumberOfUsrModel))++;
		}
		else		
		{	/*	当前用户库中已有模板数已等于最大模板数,则替换掉最旧的模板	 */
			tempUS=pFIFOTable[MaxNumberOfUsrModel-1];	//根据FIFO表找到最旧的模板序号,替换最旧的模板
		}

		/*	添加方法：将模板压缩特征加入第tempUS个模板特征处，并相应的更新FIFOTable */
		pUsrModelFeature+=tempUS*pGlobalParameter->g_dimComp;
		for(i=0;i<pGlobalParameter->g_dimComp;i++)
			pUsrModelFeature[i]=pCompressedFeature[i]-128;
		for(i=0;i<pGlobalParameter->NumberOfSupportedInnerCodeType;i++)
		{	/*	将多种内码都加上 */
			pGlobalParameter->pCodeTableOfUsrModel[i*MaxNumberOfUsrModel+tempUS]=CorrectResult;
//			pGlobalParameter->pModelSequenceToInnerCode[i*pGlobalParameter->g_nTotalModel+CorrectModelSequence];
		}			
		/*	更新FIFO表	 */
		for(i=(*(pGlobalParameter->pNumberOfUsrModel))-1;i>=1;i--)
			pFIFOTable[i]=pFIFOTable[i-1];
		pFIFOTable[0]=tempUS;

		pModelLearningTimes[tempUS]=1;
	}
	/*
	 *	如果当前的类已在用户库中，则将用户库中的模板特征与当前模板特征进行加权，然后
	 *	以加权出的特征替换用户库的原特征
	 */
	else
	{
		tempUS=i;
		pUsrModelFeature+=tempUS*pGlobalParameter->g_dimComp;
		for(i=0;i<pGlobalParameter->g_dimComp;i++)
		{	//计算出加权的特征
			tempLong=pModelLearningTimes[tempUS]*pUsrModelFeature[i]+(pCompressedFeature[i]-128);
			pUsrModelFeature[i]=(char)(tempLong/(pModelLearningTimes[tempUS]+1));
		}
		/*	更新FIFO表	 */
		for(i=0;i<*(pGlobalParameter->pNumberOfUsrModel);i++)
		{
			if(pFIFOTable[i]==tempUS)
				break;
		}
		for(;i>=1;i--)
			pFIFOTable[i]=pFIFOTable[i-1];
		pFIFOTable[0]=tempUS;

		pModelLearningTimes[tempUS]++;
	}
	return WTSuccess;
}
/*
 *	得到识别库所支持的内码种类
 */
WTError WTGetSupportedInnerCodeType(short *pSupportedInnerCodeType,char *pRamAddress)
{
	WTParameter *pGlobalParameter=(WTParameter *)(pRamAddress+GLOBAL_PARAMETER_POSITION);
	*pSupportedInnerCodeType=pGlobalParameter->pGeneralInformationSection[2];
	return WTSuccess;
}
