/************************************************************
类 型 名：CAnalyser
版权所有：THOCR
创 建 人：胡阳
创建日期：2013-08-13
版 本 号：
功能描述：
	执行公式的空间、逻辑结构分析，
	得到三叉树和字符串两种表达。
公有成员：
私有成员：
保护成员：
继承父类：
修改历史：
备注信息：
/************************************************************/

#ifndef __HMC_ANALYSE_H__
#define __HMC_ANALYSE_H__

#include "hmcCommon.h"

void hmcAnalysis(PSPLITPATHHCR pSplitPath, const short nCandPathNum,
			 PSTROKEINFOHCR pStrokeInfo, const short nStrokeNum,
			 unsigned short uCandidateResult[]);
void m_Analysis(PSPLITPATHHCR pSplitPath, const short nCandPathNum,	PSTROKEINFOHCR pStrokeInfo, const short nStrokeNum,	unsigned short uCandidateResult[]);

#endif // __HMC_ANALYSE_H__
