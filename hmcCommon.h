
#ifndef __HMC_COMMON_H__
#define __HMC_COMMON_H__

// used only for DEBUG VERSION
// comment it in RELEASE VERSION
#define HMC_DEBUG

#ifdef HMC_DEBUG
#include <stdio.h>
#include <assert.h>
#endif

// hide warning
#pragma warning(disable: 4996)

typedef unsigned char	UINT8,*PUINT8;
typedef char			SINT8,*PSINT8;
typedef unsigned short	UINT16,*PUINT16;
typedef short			SINT16,*PSINT16;
typedef unsigned int	UINT32,*PUINT32;
typedef int				SINT32,*PSINT32;

typedef unsigned short	TEXTCODE;

typedef long			WT_int32;


#define HCR_UINT16_MAX					0xffffffff
#define HCR_INT16_MAX					2147483647
#define HCR_FMIN						(-1.0E10)

#define HC_MAX_GROUP					128
#define HC_MAX_STROKE_PER_GROUP			3
#define HCR_MAX_STROKE					(HC_MAX_GROUP * HC_MAX_STROKE_PER_GROUP)

/******************************************************************
* Definition: writing area
******************************************************************/
#define HC_MIN_RECT_WIDTH				32
#define HC_MIN_RECT_HEIGHT				32
#define HC_MAX_RECT_WIDTH				1600         //800  huyang
#define HC_MAX_RECT_HEIGHT				1600         //800  huyang

#define NUM_OF_RELATIONS                8
#define BUF_SIZE	4096					// 缓冲区尺寸

#define SPLITSPOT_MAXSPLITCFD	0xFFFF	//最大切分置信度，表示肯定切开

#define FP_CHNCAND_NUM		10				//中文组最大候选字符数
//#define FP_MAXCAND_NUM		12				//最大候选个数

/******************************************************************
* Definition: segmentation & recognition struct
******************************************************************/
#define RECT_WIDTH(rc)	((rc)->nRight - (rc)->nLeft + 1)
#define RECT_HEIGHT(rc)	((rc)->nBottom - (rc)->nTop + 1)

typedef struct stRectHcr
{
	short nLeft;
	short nRight;
	short nTop;
	short nBottom;
	

} RECTHCR, *PRECTHCR;

typedef struct stStrokeInfoHcr
{
	long nDistance1[FP_CHNCAND_NUM];
	long nDistance2[FP_CHNCAND_NUM];
	long nDistance3[FP_CHNCAND_NUM];
	unsigned short uText1[FP_CHNCAND_NUM];
	unsigned short uText2[FP_CHNCAND_NUM];
	unsigned short uText3[FP_CHNCAND_NUM];
	RECTHCR	rect;
	short	nEndX;
	short	nEndY;
	short	nStartX;
	short	nStartY;
	short	nGravX;
	short	nGravY;
	unsigned short	uStartPt;	// 开始点的索引 
	unsigned short	uPntNum;	// 点数，不包括0xffff 0x0000
	short nCandCount1;
	short nCandCount2;
	short nCandCount3;
} STROKEINFOHCR, *PSTROKEINFOHCR;

typedef struct stGroupInfoHcr
{
	//RECTHCR rect;
	long	nDistance[FP_CHNCAND_NUM];	//候选距离
	unsigned short uText[FP_CHNCAND_NUM];
	short	nStrokeBe;	//起始比划序号
	short	nStrokeEn;	//终止笔划序号
	short	nGravX;		//重心
	short	nGravY;		//重心
	short	nCandCount;		//候选个数
} GROUPINFOHCR, *PGROUPINFOHCR;

typedef struct stSplitPathHcr
{
	GROUPINFOHCR	group[HC_MAX_GROUP];
	float	fScore;
	short	nGroupNum;
} SPLITPATHHCR, *PSPLITPATHHCR;


enum
{
	BBO_HORZ=	1,	//水平重叠
	BBO_VERT=	2,	//垂直重叠
	BBO_AREA=	3,	//面积
	BBO_MGWD=	4,	//合并后宽
	BBO_MGHT=	5,	//合并后高
};

#define FPExchange(Type,v1,v2) {Type tmpV=(v1);(v1)=(v2);(v2)=tmpV;}
//#define FPExchange(Type,v1,v2) {Type tmpV=v1;v1=v2;v2=tmpV;}

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef abs
#define abs(a) ((a)<0?(-(a)):(a))
#endif
#define HMC_SWAP(a,b)	((a)^=(b)^=(a)^=(b))

// Stroke related definitions

typedef struct tagRECTANGLE{
	unsigned short unXTop;//左上角的x坐标
	unsigned short unYTop;
	unsigned short unXBottom;
	unsigned short unYBottom;//右下角的y坐标
} RectInfo;//定义矩形变量

typedef struct tagEVERYSTROKEINFO //每个笔画的信息
{
	RectInfo rectOneStroke;
	unsigned short unStrokePtNum; //笔画包含的点数，不包含笔终码xff,0x00
	unsigned short unStrokeNo;   //笔画的原始序号
	unsigned short unStrokeStartPt; //笔划开始点
	short nCenterX;
	short nCenterY;
}StrokeInfo2;//记录每个笔画的信息

// Thresholds and other constants

#define MAX_SP_CAND_NUM						15
#define FINAL_SP_CAND_NUM					5

// Unicode
#define SQRT_SYM_CODE						0x221A

#define IMPOSSIBLE_X_DISTANCE				30000

#define DOUBLE_RATE				2048
#define HCR_GMM_F2I_MULTIPLY    DOUBLE_RATE

#define OLXCR_LDA_SCALE			2048

typedef struct stGmmModelHcr  // from xyzhou
{
	// original features' global mean & var
	float *pFeatureMean; 
	float *pFeatureVar;

	// segment model
	float *pSegWeight;
	float *pSegMean; 
	float *pSegVar;
	float *pSegConst;

	// none-segment model
	float *pConjWeight;
	float *pConjMean; 
	float *pConjVar;
	float *pConjConst;

	// LDA
	short *pLDA;

	// lower and upper score thresh for confidence
	long nLowerThresh;
	long nUpperThresh;
} GMMMODELHCR, *PGMMMODELHCR;

/******************************************************************
* Definition: global parameter table
******************************************************************/
typedef struct stGlobalParamHcr
{
	// template database
	char *pOlxcrLib;
	// template database parameter & RAM
	char *pOlxcrRam;
	// GMM model parameter
	GMMMODELHCR gmmModel;

	unsigned short uWidth;
	unsigned short uHeight;

} GLOBALPARAMHCR, *PGLOBALPARAMHCR;

/******************************************************************
* Definition: point data
******************************************************************/
#define HCR_MAX_POINT_COUNT				4000
#define HCR_MAX_POINT_DATA_COUNT		((HCR_MAX_POINT_COUNT) * 2)

/******************************************************************
* Definition: distribution of global RAM
*	NOTE that each offset should be 4-byte alignment
******************************************************************/
// global parameter
#define HCR_RAM_OFFSET_FOR_GLOBAL_PARAM		0
#define HCR_RAM_SIZE_FOR_GLOBAL_PARAM		256

// RAM for old OLCCR ram
#define HCR_RAM_OFFSET_FOR_OLCCR_RAM		(HCR_RAM_OFFSET_FOR_GLOBAL_PARAM + HCR_RAM_SIZE_FOR_GLOBAL_PARAM)
#define HCR_RAM_SIZE_FOR_OLCCR_RAM			2048

// RAM for segmentation
//	size = sizeof(SPLITPATHHCR) * max_candidate = (2560) * 5
#define HCR_RAM_OFFSET_FOR_SEG				(HCR_RAM_OFFSET_FOR_OLCCR_RAM + HCR_RAM_SIZE_FOR_OLCCR_RAM)
#define HCR_RAM_SIZE_FOR_SEG				(2560 * FINAL_SP_CAND_NUM)

// RAM for stroke information
//	size = sizeof(STROKEINFOHCR) * max_stroke = (224) * 3 * 128
#define HCR_RAM_OFFSET_FOR_STROKE			(HCR_RAM_OFFSET_FOR_SEG + HCR_RAM_SIZE_FOR_SEG)
#define HCR_RAM_SIZE_FOR_STROKE				(224 * HCR_MAX_STROKE)

// temporary 8-bit point data
#define HCR_RAM_OFFSET_FOR_U8_PNT			(HCR_RAM_OFFSET_FOR_STROKE + HCR_RAM_SIZE_FOR_STROKE)
#define HCR_RAM_SIZE_FOR_U8_PNT				(HCR_MAX_POINT_DATA_COUNT)

// total RAM size
#define HCR_RAM_SIZE						(HCR_RAM_OFFSET_FOR_U8_PNT + HCR_RAM_SIZE_FOR_U8_PNT)

#endif // __HMC_COMMON_H__
