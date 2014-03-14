#ifndef __HMC_SGMNT_GMM_H__
#define __HMC_SGMNT_GMM_H__

#include "hmcCommon.h"

#define OLXCR_O_FEATURE_DIM		512
#define HCR_GMM_STRU_FEA_DIM_1	0
#define HCR_GMM_STRU_FEA_DIM_2	24
#define HCR_GMM_STRU_FEA_DIM	(HCR_GMM_STRU_FEA_DIM_1 * 2 + HCR_GMM_STRU_FEA_DIM_2)

#define HCR_GMM_BASE_FEA_DIM	26

#define HCR_GMM_DIM				(HCR_GMM_BASE_FEA_DIM + HCR_GMM_STRU_FEA_DIM) 

#define HCR_GMM_MIX				32

void hmcLoadGmmModel(char *pModelRam, PGMMMODELHCR pModel);
void hmcReleaseGmmModel(PGMMMODELHCR pModel);
//void hcrGmmSegment(PSEGGLOBALINFOHCR pSegGlobal);

#endif // __HMC_SGMNT_GMM_H__
