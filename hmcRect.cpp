// BoundBox.cpp: implementation of the CBoundBox class.
//
//////////////////////////////////////////////////////////////////////

#include "hmcCommon.h"
#include "hmcRect.h"

SINT32 Overlap2(RECTHCR *b1, RECTHCR *b2, SINT32 nType)
{
	if (!b1 || !b2) return -1;
	switch(nType)
	{
	case BBO_HORZ: return max(RECT_WIDTH(b1)+RECT_WIDTH(b2)-(max(b1->nRight,b2->nRight)-min(b1->nLeft,b2->nLeft)), 0);
	case BBO_VERT: return max(RECT_HEIGHT(b1)+RECT_HEIGHT(b2)-(max(b1->nBottom,b2->nBottom)-min(b1->nTop,b2->nTop)),0);
	case BBO_AREA: return Overlap2(b1,b2,BBO_HORZ)*Overlap2(b1,b2,BBO_VERT);
	case BBO_MGWD: return max(b1->nRight,b2->nRight)-min(b1->nLeft,b2->nLeft);
	case BBO_MGHT: return max(b1->nBottom,b2->nBottom)-min(b1->nTop,b2->nTop);
	}
	return 0;
}

SINT32 Area2(RECTHCR* Src)
{
	if (!Src) return -1;
	return RECT_WIDTH(Src) * RECT_HEIGHT(Src);
}

void Union2(RECTHCR* bbx, RECTHCR* Src)
{
	if (bbx && Src)
	{
		Src->nLeft = min(Src->nLeft, bbx->nLeft);
		Src->nRight = max(Src->nRight, bbx->nRight);
		Src->nTop = min(Src->nTop, bbx->nTop);
		Src->nBottom = max(Src->nBottom, bbx->nBottom);
	}
}

void CenterPoint2(PSINT32 ps4X, PSINT32 ps4Y, RECTHCR* Src)
{
	if (!Src) return;
	if(ps4X) *ps4X = (Src->nLeft + Src->nRight)/2;
	if(ps4Y) *ps4Y = (Src->nTop + Src->nBottom)/2;
}
