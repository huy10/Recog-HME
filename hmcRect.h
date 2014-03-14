
#ifndef __HMC_RECT_H__
#define __HMC_RECT_H__

#include "hmcCommon.h"

SINT32 Overlap2(RECTHCR *b1, RECTHCR *b2, SINT32 nType);
SINT32 Area2(RECTHCR* Src);
void Union2(RECTHCR* bbx, RECTHCR* Src);
void CenterPoint2(PSINT32 ps4X, PSINT32 ps4Y, RECTHCR* Src);

#endif // __HMC_RECT_H__
