/************************************************************
类 型 名：CRecognizer
版权所有：THOCR
创 建 人：胡阳
创建日期：2013-07-29
版 本 号：
功能描述：
	公式识别中用于笔画分组(切分)及单个字符识别的类。
公有成员：
私有成员：
保护成员：
继承父类：
修改历史：
备注信息：
/************************************************************/

#ifndef __HMC_SEGMENT_H__
#define __HMC_SEGMENT_H__

#include "hmcCommon.h"
static unsigned short CalCrossPoint(
									unsigned short *pRawPntData,
									PSTROKEINFOHCR pStrokeInfo,
									unsigned short FirstStroke,
									unsigned short SecondStroke
									);
short hmcSegAndRecog(unsigned short uPntData[],
					const unsigned short uPntNum,
					char *pGlobalRam, 
					PSPLITPATHHCR pSplitPath,
					PSTROKEINFOHCR pStrokeInfo,
					short *pStrokeNum);

#endif // __HMC_SEGMENT_H__
