/******************************************************************


#include "hmcSgmntGmm.h"
//#include "hcrStruFea.h"

/******************************************************************
* Function:
* Description: load GMM model
* Call:
* Call By:
* General Variables:
* Input & Output:
* Return:
******************************************************************/
void hmcLoadGmmModel(char *pModelRam, PGMMMODELHCR pModel)
{
	int totalDim = HCR_GMM_DIM * HCR_GMM_MIX;
	long *pnTmp = (long *)pModelRam;
	float *pfTmp;

	// score threshold for go to segmentation rule set
	pModel->nLowerThresh = *pnTmp;
	pnTmp++;
	pModel->nUpperThresh = *pnTmp;
	pnTmp++;

	pfTmp = (float *)pnTmp;

	// feature normalization
	pModel->pFeatureMean = pfTmp;
	pfTmp += HCR_GMM_DIM;
	pModel->pFeatureVar = pfTmp;
	pfTmp += HCR_GMM_DIM;

	// coalition model
	pModel->pConjWeight = pfTmp;
	pfTmp += HCR_GMM_MIX;
	pModel->pConjMean = pfTmp;
	pfTmp += totalDim;
	pModel->pConjVar = pfTmp;
	pfTmp += totalDim;
	pModel->pConjConst = pfTmp;
	pfTmp += HCR_GMM_MIX;

	// segmentation model
	pModel->pSegWeight = pfTmp;
	pfTmp += HCR_GMM_MIX;
	pModel->pSegMean = pfTmp;
	pfTmp += totalDim;
	pModel->pSegVar = pfTmp;
	pfTmp += totalDim;
	pModel->pSegConst = pfTmp;
	pfTmp += HCR_GMM_MIX;

	// LDA for OLXCR feature
	pModel->pLDA = (short *)pfTmp;
}

/******************************************************************
* Function:
* Description: release GMM model, reset all.
* Call:
* Call By:
* General Variables:
* Input & Output:
* Return:
* Others:
******************************************************************/
void hmcReleaseGmmModel(PGMMMODELHCR pModel)
{
	pModel->nLowerThresh = pModel->nUpperThresh = 0;

	pModel->pFeatureMean = 0;
	pModel->pFeatureVar = 0;

	pModel->pSegWeight = 0;
	pModel->pSegMean = 0;
	pModel->pSegVar = 0;
	pModel->pSegConst = 0;

	pModel->pConjWeight = 0;
	pModel->pConjMean = 0;
	pModel->pConjVar = 0;
	pModel->pConjConst = 0;

	pModel->pLDA = 0;
}
