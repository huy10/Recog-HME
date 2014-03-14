/******************************************************************
* Function: HmcInit
* Description: load database
* Commit: Yang Hu
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
#define DEBUG_MODE 1
#ifdef __cplusplus
	extern "C"
#endif

#include <stdlib.h>
#include "math.h"

#include <iomanip>
#include <fstream>
#include "HmcInterface.h"
#include "hmcCommon.h"
#include "hmcSegment.h"
#include "hmcAnalyse.h"
#include "hmcSgmntGmm.h"
#include "wtpen.h"
//#include "wtpen_V4.h"

short HmcInit(char *pDatabase,
			  char *pGlobalRam)
{
	PGLOBALPARAMHCR pGlobalParam = (PGLOBALPARAMHCR)(
		pGlobalRam + HCR_RAM_OFFSET_FOR_GLOBAL_PARAM);
	long *pHeader = (long *)pDatabase;
	short nRetVal;

	if (!pDatabase)
	{
		return HMCERROR_NO_DB;
	}
	if (!pGlobalRam)
	{
		return HMCERROR_NULL_ADDRESS;
	}

	// recognition module initialization
	if (pHeader[0] != 0x28)  
	{
		return HMCERROR_ERROR_DB;
	}
	pGlobalParam->pOlxcrLib = pDatabase;// + pHeader[0];
	pGlobalParam->pOlxcrRam = pGlobalRam + HCR_RAM_OFFSET_FOR_OLCCR_RAM;
/*	nRetVal = WTRecognizeInitEx(pGlobalParam->pOlxcrRam, 
		HCR_RAM_SIZE_FOR_OLCCR_RAM, 
		pGlobalParam->pOlxcrLib, 
		0, 0, 254, 254);
*///plr2013-08-06
 	nRetVal = WTRecognizeInit(pGlobalParam->pOlxcrRam, 	HCR_RAM_SIZE_FOR_OLCCR_RAM, pGlobalParam->pOlxcrLib);


	if (nRetVal != HMCERROR_SUCCESS)
	{
		return nRetVal;
	}

	nRetVal = WTSetRange(0x000006FF, pGlobalParam->pOlxcrRam); //plr2013-08-06
	nRetVal = WTSetSpeed(0,pGlobalParam->pOlxcrRam);
	if (nRetVal != HMCERROR_SUCCESS)
	{
		return nRetVal;
	}

	// GMM model initialization
	if (pHeader[1] != 0)
	{
		hmcLoadGmmModel(pDatabase + pHeader[1], &pGlobalParam->gmmModel);
	}
	else
	{
		hmcReleaseGmmModel(&pGlobalParam->gmmModel);
	}

	return HMCERROR_SUCCESS;
}

/******************************************************************
* Function: HmcSetRect
* Description: set writing area
* Call:
* Call By:
* General Variables:
* Input & Output:
*	->uWidht:	right X value, if left X=0
*	->uHeight:	bottom Y value, if top Y=0
*	->pGlobalRam:	global ram
* Return:
*	0	success
*	else	fail
* Others:
******************************************************************/
short HmcSetRect(unsigned short uWidth,
				 unsigned short uHeight,
				 char *pGlobalRam)
{
	PGLOBALPARAMHCR pGlobalParam = (PGLOBALPARAMHCR)(
		pGlobalRam + HCR_RAM_OFFSET_FOR_GLOBAL_PARAM);
	unsigned short uBase = 0;
	unsigned short uMedium = 0;
	short nRetVal;

	if (!pGlobalRam)
	{
		return HMCERROR_NULL_ADDRESS;
	}
	if (uWidth < HC_MIN_RECT_WIDTH || uHeight < HC_MIN_RECT_HEIGHT)
	{
		return HMCERROR_TOO_SMALL_RECT;
	}
	if (uWidth > HC_MAX_RECT_WIDTH || uHeight > HC_MAX_RECT_HEIGHT)
	{
		return HMCERROR_TOO_BIG_RECT;
	}

	//nRetVal = WTSetWriteArea(0, 0, uWidth, uHeight, uBase, uMedium, 	pGlobalParam->pOlxcrRam); //plr2013-08-06

	nRetVal = 0;
	if (HMCERROR_SUCCESS == nRetVal)
	{
		pGlobalParam->uWidth = uWidth;
		pGlobalParam->uHeight = uHeight;
	}

	return nRetVal;
}

short HmcRecog(unsigned short uPointData[],
			   unsigned short uPointNum,
			   unsigned short uCandidateNum,
			   unsigned short uEndInput,
			   char *pGlobalRam,
			   unsigned short uCandidateResult[])
{
	PSPLITPATHHCR pSplitPath = (PSPLITPATHHCR)(
		pGlobalRam + HCR_RAM_OFFSET_FOR_SEG);
	short nCandPathNum = 0;
	PSTROKEINFOHCR pStrokeInfo = (PSTROKEINFOHCR)(
		pGlobalRam + HCR_RAM_OFFSET_FOR_STROKE);
	short nStrokeNum = 0;

	if (!pGlobalRam)
	{
		return HMCERROR_NULL_ADDRESS;
	}
	if (uPointNum == 0)
	{
		return HMCERROR_ERROR_POINTS;
	}
	if (uPointNum > HCR_MAX_POINT_COUNT)
	{
		return HMCERROR_TOO_MANY_POINTS;
	}

	nCandPathNum = hmcSegAndRecog(uPointData, uPointNum, pGlobalRam, pSplitPath, pStrokeInfo, &nStrokeNum);

//	m_Analysis(pSplitPath, nCandPathNum, pStrokeInfo, nStrokeNum, uCandidateResult);


	hmcAnalysis(pSplitPath, nCandPathNum, 	pStrokeInfo, nStrokeNum, uCandidateResult);

	return HMCERROR_SUCCESS;
}

char * HmcFinal(unsigned short uOriginalResult[])
{
	
	
	//short nErrCode = HmcRecog(m_pPenDraw->m_pPntData, m_pPenDraw->m_unPntNum, 5, 1, m_pGlobalRam, uOriginalResult);
	char *pszResult = (char *)&uOriginalResult[0];
	int nFactWordNum = 5;
	
	int i = 0;
	while (pszResult[i])
				{
					if (pszResult[i] == 0xFFFF)
					{
						pszResult[i] = 0x0000;
						nFactWordNum++;
					}
					i++;
				}

	//char *  m_csCurrentCandidate = pszResult;
	return pszResult;
}
