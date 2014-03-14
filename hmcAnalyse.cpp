/******************************************************************
* Function: HmcInit
* Description: load database
* Commit: Yang Hu, Yejun Tang
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
//#include  <afxpriv.h>
#include "hmcCommon.h"
#include "hmcAnalyse.h"
#include "hmcRect.h"
#include <stdio.h>
//#include <stack>

//#include <list>
#define MAXNODE 32

typedef struct CHILDNODE
{
	TEXTCODE *fullRecogResult;
	short symbol;         // UNICODE 全角 e.g. 0xff41
	short ChildeNode;
    struct CHILDNODE * NextChild;
	//List _children;

}ChildNODE;
typedef struct UNITREE{
	
	TEXTCODE *fullRecogResult;
	unsigned short symbol;         // UNICODE 全角 e.g. 0xff41
	struct UNITREE * Children[MAXNODE];
	int parent;   //存放父亲结点的ID
	int NodeID;
}UniTree;

typedef struct StructAnalyzerHcr
{
	TEXTCODE* m_pRecResult;				// 用于存放单个符号识别结果
	int		m_pnType[HC_MAX_GROUP];	// 用于分配符号类型数组
	int**	m_ppnSpacialRelation;			// 用于存放空间位置关系矩阵
	int		m_pnRank[HC_MAX_GROUP];	// 用于分配符号级别数组
	int		m_ppnDomRelation[HC_MAX_GROUP][HC_MAX_GROUP];		// 用于分配直接统领关系矩阵
	int		m_pnBoxWidth[HC_MAX_GROUP];// 用于存放每个符号的宽度
	int		m_pnBoxLeft[HC_MAX_GROUP];	// 用于存放每个符号的左边界
	int		m_nMaxInRank;					// 数组m_pnRank中的最大值，即符号的最高级别
	int		m_nSymbolNum;					// 符号个数	
	UniTree	treeNode[HC_MAX_GROUP];// 用于存放每个符号对应的三叉树结点地址
	short	nNodeCreated[HC_MAX_GROUP];	// this node (ID) has been created
	short	nRootNodeID;				// 生成表达式三叉树的头指针
}  UniStruct; //, *pUniStruct;


// tribinary tree
typedef struct stTrinaryTreeHcr
{
	TEXTCODE *fullRecogResult;	// 对应笔划组的全部识别结果
	short	next;			// next node's ID
	short	leftChild;		// nLeft child node's ID
	short	rightChild;		// nRight child node's ID
	int		nRefCount;		// 引用计数
	unsigned short	symbol;
} TRINARYTREEHCR, *PTRINARYTREEHCR;

typedef struct stStructAnalyzerHcr
{
	TEXTCODE* m_pRecResult;				// 用于存放单个符号识别结果
	int		m_pnType[HC_MAX_GROUP];	// 用于分配符号类型数组
	int**	m_ppnSpacialRelation;			// 用于存放空间位置关系矩阵
	int		m_pnRank[HC_MAX_GROUP];	// 用于分配符号级别数组
	int		m_ppnDomRelation[HC_MAX_GROUP][HC_MAX_GROUP];		// 用于分配直接统领关系矩阵
	int		m_pnBoxWidth[HC_MAX_GROUP];// 用于存放每个符号的宽度
	int		m_pnBoxLeft[HC_MAX_GROUP];	// 用于存放每个符号的左边界
	int		m_nMaxInRank;					// 数组m_pnRank中的最大值，即符号的最高级别
	int		m_nSymbolNum;					// 符号个数	
	// xyzhou
	TRINARYTREEHCR	treeNode[HC_MAX_GROUP];// 用于存放每个符号对应的三叉树结点地址
	short	nNodeCreated[HC_MAX_GROUP];	// this node (ID) has been created
	short	nRootNodeID;				// 生成表达式三叉树的头指针
}  STRUCTANALYZERHCR, *PSTRUCTANALYZERHCR;

/********
Here is my implementation. Coded by Hu Yang.
/************/


void AnalysisInit(UniStruct * pUniStruct)
{
	pUniStruct->nRootNodeID = 0;   // define ROOTNODE 0
//	pUniStruct->treeNode->Children->symbol = 0;   // define ROOT 0


}
void	CalculatePositionRelationship(int symbolcount, TEXTCODE* presult, int** point,  PRECTHCR pRect,  int ppnEachOther[HC_MAX_GROUP][HC_MAX_GROUP])
{
	int i, j;
	enum {NONE, RIGHT, LEFT, ABOVE, UNDER, ENCLOSE, BE_ENCLOSED, UPPER_RIGHT, LOWER_LEFT};

	TEXTCODE ptcResult[HC_MAX_GROUP];
	RECTHCR pbbxEach[HC_MAX_GROUP];
	int pnCenterX[HC_MAX_GROUP];
	int pnCenterY[HC_MAX_GROUP];

	

	for (i = 0; i < symbolcount; ++i)
	{
		for (j = 0; j < symbolcount; ++j)
		{
			ppnEachOther[i][j] = NONE;
		}
	}
	// 取得首选识别结果及各字符外接框、中心等
	for (i = 0; i != symbolcount; ++i)
	{
		ptcResult[i] = presult[i*FP_CHNCAND_NUM];
		pbbxEach[i] = pRect[i];
		CenterPoint2(pnCenterX + i, pnCenterY + i, pbbxEach + i);
	}

	// Calculate relationship now
	for (i = 0; i < symbolcount - 1; ++i)
	{
		for (j = i + 1; j < symbolcount; ++j)
		{
			// i 是根号，j 被 i 包围情形
			if (ptcResult[i] == SQRT_SYM_CODE)
			{
				if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_AREA) > 0.5 * Area2(pbbxEach + j))
				{
					ppnEachOther[i][j] = ENCLOSE;
					ppnEachOther[j][i] = BE_ENCLOSED;
					continue;
				}
				else if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_AREA) > 0.1f * Area2((pbbxEach + j)))  //  			&&  ptcResult[j] >= '0' && ptcResult[j] <= '9'
				{
					ppnEachOther[i][j] = ENCLOSE;
					ppnEachOther[j][i] = BE_ENCLOSED;
					continue;
				}
			}

			// j 是根号，i 被 j 包围情形
			if (ptcResult[j] == SQRT_SYM_CODE)
			{
				if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_AREA) > 0.5 * Area2(pbbxEach + i))
				{
					ppnEachOther[i][j] = BE_ENCLOSED;
					ppnEachOther[j][i] = ENCLOSE;
					continue;
				}
				else if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_AREA) > 0.1f * Area2((pbbxEach + i)) )        // &&  ptcResult[i] >= '0' && ptcResult[i] <= '9
					
				{
					ppnEachOther[i][j] = BE_ENCLOSED;
					ppnEachOther[j][i] = ENCLOSE;
					continue;
				}
			}

			// i 为分数线(横线), 上/下符号为 j
			if ((ptcResult[i] == '-' || ptcResult[i] == '/') &&	RECT_WIDTH(pbbxEach + i) >= 1.5f * RECT_HEIGHT(pbbxEach + i))
			{
				if ( Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) >= 0.1f * RECT_WIDTH(pbbxEach + j)
					|| Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) >= 0.8f * RECT_WIDTH(pbbxEach + j))
				{
					// 上符号为 j
					if (pnCenterY[j] < pnCenterY[i] &&
						pnCenterY[i] - pnCenterY[j] < 1.1f * max(RECT_WIDTH(pbbxEach + i), RECT_HEIGHT(pbbxEach + j)))
					{
						ptcResult[i] = '-';
						presult[i * FP_CHNCAND_NUM] = '-';
						ppnEachOther[i][j] = ABOVE;
						ppnEachOther[j][i] = UNDER;
						continue;
					}
					// 下符号为 j
					else if (pnCenterY[j] > pnCenterY[i] &&
						pnCenterY[j] - pnCenterY[i] < 1.1f * max(RECT_WIDTH(pbbxEach + i), RECT_HEIGHT(pbbxEach + j)))
					{
						ptcResult[i] = '-';
						presult[i * FP_CHNCAND_NUM] = '-';
						ppnEachOther[i][j] = UNDER;
						ppnEachOther[j][i] = ABOVE;
						continue;
					}
				}
			}

			// j 为分数线(横线), 上/下符号为 i
			if ((ptcResult[j] == '-' || ptcResult[j] == '/') &&
				RECT_WIDTH(pbbxEach + j) >= 1.5f * RECT_HEIGHT(pbbxEach + j))
			{
				if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) >= 0.1f * RECT_WIDTH(pbbxEach + i)
					|| Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) >= 0.8f * RECT_WIDTH(pbbxEach + i))
				{
					// 上符号为 i
					if (pnCenterY[i] < pnCenterY[j] &&
						pnCenterY[j] - pnCenterY[i] < 1.1f * max(RECT_WIDTH(pbbxEach + j), RECT_HEIGHT(pbbxEach + i)))
					{
						ptcResult[j] = '-';
						presult[j * FP_CHNCAND_NUM] = '-';
						ppnEachOther[j][i] = ABOVE;
						ppnEachOther[i][j] = UNDER;
						continue;
					}
					// 下符号为 i
					else if (pnCenterY[i] > pnCenterY[j] &&
						pnCenterY[i] - pnCenterY[j] < 1.1f * max(RECT_WIDTH(pbbxEach + j), RECT_HEIGHT(pbbxEach + i)))
					{
						ptcResult[j] = '-';
						presult[j * FP_CHNCAND_NUM] = '-';
						ppnEachOther[j][i] = UNDER;
						ppnEachOther[i][j] = ABOVE;
						continue;
					}
				}
			}

			// i 的左下符号为 j，即可能为乘幂形式的情形
			if (pbbxEach[i].nTop < pbbxEach[j].nTop + 0.25f * RECT_HEIGHT(pbbxEach + j) &&
				pbbxEach[i].nBottom < pbbxEach[j].nBottom - 0.5f * RECT_HEIGHT(pbbxEach + j) &&
				pbbxEach[j].nRight < pnCenterX[i] &&
				pnCenterX[j] < pbbxEach[i].nLeft &&
				pbbxEach[i].nLeft - pbbxEach[j].nRight < 1.0 * max(RECT_WIDTH(pbbxEach + j), RECT_HEIGHT(pbbxEach + j)) &&
				ptcResult[i] != '.' &&
				ptcResult[j] != '.' &&
				((ptcResult[i] == '-' && ptcResult[j] != '-') || (ptcResult[i] != '-' && pbbxEach[j].nTop >= pbbxEach[i].nTop)))
			{
				if (ptcResult[i] == '-' && 
					(pbbxEach[j].nTop + 0.33f * RECT_HEIGHT(pbbxEach + j) <= pnCenterY[i] || 
					pbbxEach[j].nTop + 0.2f * RECT_HEIGHT(pbbxEach + j) < pbbxEach[i].nBottom))
				{
					;
				}
				else if (ptcResult[j] == ')')
				{
					ppnEachOther[i][j] = LOWER_LEFT;
					ppnEachOther[j][i] = UPPER_RIGHT;
					continue;
				}
				else if (ptcResult[j] != '-')
				{
					ppnEachOther[i][j] = LEFT;
					ppnEachOther[j][i] = RIGHT;
					continue;
				}
			}
			// i 的右上符号为 j，即可能为乘幂形式的情形
			else if ((pbbxEach[j].nTop < pbbxEach[i].nTop + 0.25f * RECT_HEIGHT(pbbxEach + i) &&
				pbbxEach[j].nBottom < pbbxEach[i].nBottom - 0.5f * RECT_HEIGHT(pbbxEach + i)) &&
				pbbxEach[i].nRight < pnCenterX[j] &&
				pnCenterX[i] < pbbxEach[j].nLeft &&
				pbbxEach[j].nLeft - pbbxEach[i].nRight < 1.0 * max(RECT_WIDTH(pbbxEach + i), RECT_HEIGHT(pbbxEach + i)) &&
				ptcResult[j] != '.' &&
				ptcResult[i] != '.' &&
				((ptcResult[j] == '-' && ptcResult[i] != '-') ||( ptcResult[j] != '-' && pbbxEach[i].nTop >= pbbxEach[j].nTop)))
			{
				if (ptcResult[j] == '-' && 
					(pbbxEach[i].nTop + 0.33f * RECT_HEIGHT(pbbxEach + i) <= pnCenterY[j] ||
					pbbxEach[i].nTop + 0.2f * RECT_HEIGHT(pbbxEach + i) < pbbxEach[j].nBottom))
				{
					;
				}
				else if ( ptcResult[i] == ')')
				{
					ppnEachOther[j][i] = LOWER_LEFT;
					ppnEachOther[i][j] = UPPER_RIGHT;
					continue;
				}
				else if (ptcResult[i] != '-')
				{
					ppnEachOther[j][i] = LEFT;
					ppnEachOther[i][j] = RIGHT;
					continue;
				}
			}
			// i j 为左右关系的一般判定
			if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_VERT) > 	0.5 * min(RECT_HEIGHT(pbbxEach + i), RECT_HEIGHT(pbbxEach + j)) &&
				abs(pnCenterY[i] - pnCenterY[j]) < 	0.5 * max(RECT_HEIGHT(pbbxEach + i), RECT_HEIGHT(pbbxEach + j)))
			{
				// i 左 j 右
				if (pbbxEach[j].nLeft > pnCenterX[i] - 0.2 * RECT_WIDTH(pbbxEach + j) &&
					pbbxEach[i].nRight < pnCenterX[j] + 0.2 * RECT_WIDTH(pbbxEach + j) &&
					pnCenterY[i] >= pbbxEach[j].nTop - 0.1 * RECT_HEIGHT(pbbxEach + j) &&
					pnCenterY[i] <= pbbxEach[j].nBottom + 0.1 * RECT_HEIGHT(pbbxEach + j) &&
					pnCenterY[j] >= pbbxEach[i].nTop - 0.1 * RECT_HEIGHT(pbbxEach + i) &&
					pnCenterY[j] <= pbbxEach[i].nBottom + 0.1 * RECT_HEIGHT(pbbxEach + i) &&
					RECT_WIDTH(pbbxEach + i) <= 2 * RECT_HEIGHT(pbbxEach + i) &&
					RECT_WIDTH(pbbxEach + j) <= 2 * RECT_HEIGHT(pbbxEach + j) &&
					pnCenterX[i] < pnCenterX[j])
				{
					ppnEachOther[i][j] = RIGHT;
					ppnEachOther[j][i] = LEFT;
					continue;
				}
				// j 左 i 右
				else if (pbbxEach[i].nLeft > pnCenterX[j] - 0.2 * RECT_WIDTH(pbbxEach + i) &&
					pbbxEach[j].nRight < pnCenterX[i] + 0.2 * RECT_WIDTH(pbbxEach + i) &&
					pnCenterY[j] >= pbbxEach[i].nTop - 0.1 * RECT_HEIGHT(pbbxEach + i) &&
					pnCenterY[j] <= pbbxEach[i].nBottom + 0.1 * RECT_HEIGHT(pbbxEach + i) &&
					pnCenterY[i] >= pbbxEach[j].nTop - 0.1 * RECT_HEIGHT(pbbxEach + j) &&
					pnCenterY[i] <= pbbxEach[j].nBottom + 0.1 * RECT_HEIGHT(pbbxEach + j) &&
					RECT_WIDTH(pbbxEach + j) <= 2 * RECT_HEIGHT(pbbxEach + j) &&
					RECT_WIDTH(pbbxEach + i) <= 2 * RECT_HEIGHT(pbbxEach + i) &&
					pnCenterX[j] < pnCenterX[i])
				{
					ppnEachOther[i][j] = LEFT;
					ppnEachOther[j][i] = RIGHT;
					continue;
				}
			}

			// i 的右符号为 j 的其他情形，针对横线之间判断关系
			 if (
				(abs(pnCenterY[i] - pnCenterY[j]) 	< 0.4 * max(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j))
				|| (ptcResult[i] == '-' && ptcResult[j] == '-' && abs(pnCenterY[i] - pnCenterY[j]) < 0.8 * max(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j))))
				&&	pbbxEach[j].nLeft > pnCenterX[i]
				&&	pbbxEach[i].nRight < pnCenterX[j]
				&&  (RECT_WIDTH(pbbxEach + i) > 2 * RECT_HEIGHT(pbbxEach + i) || RECT_WIDTH(pbbxEach + j) > 2 * RECT_HEIGHT(pbbxEach + j))
				&&  abs(pnCenterX[i] - pnCenterX[j]) < 5 * max(max(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j)),max(RECT_HEIGHT(pbbxEach + i), RECT_HEIGHT(pbbxEach + j)))
				&& (Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) <= 0.2 * max(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j)))
				&& pnCenterX[i] < pnCenterX[j]
			   )
			{

				ppnEachOther[i][j] = RIGHT;
				ppnEachOther[j][i] = LEFT;
			}
			// j 的右符号为 i 的其他情形，针对横线之间判断关系
			else if (
				(abs(pnCenterY[j] - pnCenterY[i]) 	< 0.4 * max(RECT_WIDTH(pbbxEach + j), RECT_WIDTH(pbbxEach + i))
				|| (ptcResult[j] == '-' && ptcResult[i] == '-' && abs(pnCenterY[j] - pnCenterY[i]) < 0.8 * max(RECT_WIDTH(pbbxEach + j), RECT_WIDTH(pbbxEach + i))))
				&&	pbbxEach[i].nLeft > pnCenterX[j]
				&&	pbbxEach[j].nRight < pnCenterX[i]
				&&  (RECT_WIDTH(pbbxEach + j) > 2 * RECT_HEIGHT(pbbxEach + j) || RECT_WIDTH(pbbxEach + i) > 2 * RECT_HEIGHT(pbbxEach + i))
				&&  abs(pnCenterX[j] - pnCenterX[i])< 5 * max(max(RECT_WIDTH(pbbxEach + j), RECT_WIDTH(pbbxEach + i)),max(RECT_HEIGHT(pbbxEach + j), RECT_HEIGHT(pbbxEach + i)))
				&& (Overlap2(pbbxEach + j, pbbxEach + i, BBO_HORZ) <= 0.2 * max(RECT_WIDTH(pbbxEach + j), RECT_WIDTH(pbbxEach + i)))
				&& pnCenterX[j] < pnCenterX[i]
			)
			{
				ppnEachOther[i][j] = LEFT;
				ppnEachOther[j][i] = RIGHT;
			}
	
			// 左右结构的另一判断, 针对 '1'
			else if (
				ptcResult[i] == '1'
				&&	Overlap2(pbbxEach + i, pbbxEach + j, BBO_VERT) > 0.6 * RECT_HEIGHT(pbbxEach + i)
				&&  Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) < 0.2 * RECT_WIDTH(pbbxEach + j)
				&&  3 * RECT_WIDTH(pbbxEach + i) < RECT_HEIGHT(pbbxEach + j)
				&&  pbbxEach[i].nRight < pnCenterX[j]
			)
			{
				ppnEachOther[i][j] = RIGHT;
				ppnEachOther[j][i] = LEFT;
			}
			
			// 左右结构的另一判断, 针对 '1'
			else if (
				ptcResult[j] == '1'
				&&	Overlap2(pbbxEach + j, pbbxEach + i, BBO_VERT) > 0.6 * RECT_HEIGHT(pbbxEach + j)
				&&  Overlap2(pbbxEach + j, pbbxEach + i, BBO_HORZ) < 0.2 * RECT_WIDTH(pbbxEach + i)
				&&  3 * RECT_WIDTH(pbbxEach + j) < RECT_HEIGHT(pbbxEach + i)
				&&  pbbxEach[j].nRight < pnCenterX[i]
			)
			{
				ppnEachOther[i][j] = LEFT;
				ppnEachOther[j][i] = RIGHT;
			}
			
			// 左右结构的另一判断，规则较松，为查缺补漏
			else if ((abs(pnCenterY[i] - pnCenterY[j]) < 0.5 * max(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j)) ||
				abs(pnCenterY[i] - pnCenterY[j]) < 0.5 * max(RECT_HEIGHT(pbbxEach + i), RECT_HEIGHT(pbbxEach + j))) &&
				(abs(pnCenterX[i] - pnCenterX[j]) < 2 * max(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j)) ||
				abs(pnCenterX[i] - pnCenterX[j]) < 2 * max(RECT_HEIGHT(pbbxEach + i), RECT_HEIGHT(pbbxEach + j))) &&
				(Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) <= 0.3 * min(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j))) &&
				pbbxEach[i].nTop < pbbxEach[j].nBottom &&
				pbbxEach[j].nTop < pbbxEach[i].nBottom && 
				(ptcResult[j] == '-' ||ptcResult[i] == '-'))
			{
				if (pnCenterX[i] > pnCenterX[j])
				{
					ppnEachOther[i][j] = LEFT;
					ppnEachOther[j][i] = RIGHT;
				}
				else
				{
					ppnEachOther[i][j] = RIGHT;
					ppnEachOther[j][i] = LEFT;
				}
			}
		}
	}


	// get the Confidence matrix
	for (i = 0; i < symbolcount; ++i)
	{
		int nMinIndex;
		unsigned int uMinDist;

		// 左下
		nMinIndex = -1;
		uMinDist = HCR_UINT16_MAX;
		for (j = 0; j < symbolcount; ++j)
		{
			unsigned int uDist;

			if (i == j || ppnEachOther[i][j] != LOWER_LEFT)
			{
				continue;
			}

			uDist = (pnCenterX[i] - pnCenterX[j]) * (pnCenterX[i] - pnCenterX[j])
				+ (pnCenterY[i] - pnCenterY[j]) * (pnCenterY[i] - pnCenterY[j]);
			if (uDist < uMinDist)
			{
				uMinDist = uDist;
				nMinIndex = j;
			}
		}
		if (nMinIndex >= 0)
		{
			point[i][5] = nMinIndex;
		}

		// 被包围
		nMinIndex = -1;
		uMinDist = HCR_UINT16_MAX;
		for (j = 0; j < symbolcount; ++j)
		{
			unsigned int uDist;

			if (i == j || ppnEachOther[i][j] != BE_ENCLOSED)
			{
				continue;
			}

			// 上面过程保证了 j 为根号
			uDist = (pnCenterX[i] - pbbxEach[j].nLeft) 
				* (pnCenterX[i] - pbbxEach[j].nLeft)
				+ (pnCenterY[i] - pbbxEach[j].nTop)
				* (pnCenterY[i] - pbbxEach[j].nTop);

			if (uDist < uMinDist && point[i][5] != j)
			{
				uMinDist = uDist;
				nMinIndex = j;
			}
		}
		if (nMinIndex >= 0)
		{
			point[i][2] = nMinIndex;
		}

		// 上
		nMinIndex = -1;
		uMinDist = HCR_UINT16_MAX;
		for (j = 0; j < symbolcount; ++j)
		{
			unsigned int uDist;

			if (i == j || ppnEachOther[i][j] != ABOVE)
			{
				continue;
			}

			uDist = (pnCenterY[i] - pnCenterY[j]) * (pnCenterY[i] - pnCenterY[j]);
			if (uDist < uMinDist && point[i][2] != j && point[i][5] != j)
			{
				uMinDist = uDist;
				nMinIndex = j;
			}
		}
		if (nMinIndex >= 0)
		{
			point[i][1] = nMinIndex;
			point[nMinIndex][6] = i;
		}

		// 下
		nMinIndex = -1;
		uMinDist = HCR_UINT16_MAX;
		for (j = 0; j < symbolcount; ++j)
		{
			unsigned int uDist;

			if (i == j || ppnEachOther[i][j] != UNDER)
			{
				continue;
			}

			uDist = (pnCenterY[i] - pnCenterY[j]) * (pnCenterY[i] - pnCenterY[j]);
			if (uDist < uMinDist && point[i][2] != j && point[i][5] != j)
			{
				uMinDist = uDist;
				nMinIndex = j;
			}
		}
		if (nMinIndex >= 0)
		{
			point[i][6] = nMinIndex;
			point[nMinIndex][1] = i;
		}

		// 左/右
		nMinIndex = -1;
		uMinDist = HCR_UINT16_MAX;
		for (j = 0; j < symbolcount; ++j)
		{
			unsigned int uDist;

			if (i == j || ppnEachOther[i][j] != RIGHT)
			{
				continue;
			}

			if (ptcResult[i] != '.' && ptcResult[j] != '.' && ptcResult[i] != '-' && ptcResult[j] != '-')
			{
				uDist = (pbbxEach[i].nRight - pbbxEach[j].nLeft) 
					* (pbbxEach[i].nRight - pbbxEach[j].nLeft)
					+ 3 * (pnCenterY[i] - pnCenterY[j]) * (pnCenterY[i] - pnCenterY[j]);
			}
			else if (ptcResult[i] == '-' || ptcResult[i] == '-')
			{
				uDist = (pbbxEach[i].nRight - pbbxEach[j].nLeft) 
					* (pbbxEach[i].nRight - pbbxEach[j].nLeft)
					+ (pnCenterY[i] - pnCenterY[j]) * (pnCenterY[i] - pnCenterY[j]) / 5;
			}
			else
			{
				uDist = (pbbxEach[i].nRight - pbbxEach[j].nLeft) 
					* (pbbxEach[i].nRight - pbbxEach[j].nLeft)
					+ 0 * (pnCenterY[i] - pnCenterY[j]) * (pnCenterY[i] - pnCenterY[j]);
			}
			if (uDist < uMinDist)
			{
				uMinDist = uDist;
				nMinIndex = j;
			}
		}
		if (nMinIndex >= 0)
		{
			if (point[nMinIndex][3] >= 0)
			{
				int m = point[nMinIndex][3];
				if ( (int)uMinDist < (pbbxEach[m].nRight - pbbxEach[nMinIndex].nLeft) 
					* (pbbxEach[m].nRight - pbbxEach[nMinIndex].nLeft)
					+ 3 * (pnCenterY[nMinIndex] - pnCenterY[m]) * (pnCenterY[nMinIndex] - pnCenterY[m]) )
				{
					point[i][4] = nMinIndex;
					point[nMinIndex][3] = i;
				}
			}
			else
			{
				point[i][4] = nMinIndex;
				point[nMinIndex][3] = i;
			}
		}
	}
	for (i = 0; i < symbolcount; ++i)
	{
		for (j = 0; j < symbolcount; ++j)
		{
			if (point[i][4] == j && point[j][4] == i)
			{
				point[j][4] = -1;
				point[i][3] = -1;
				point[j][3] = i;
			}

			if (point[i][3] == j && point[j][3] == i)
			{
				point[j][3] = -1;
				point[i][4] = -1;
				point[j][4] = i;
			}
		}
	}
	//SlashPostprocess(ptcResult, pbbxEach, pnCenterX, pnCenterY, point, symbolcount);

	for (i = 0; i < symbolcount; ++i)
	{
		int rootInd, rightInd;
		if (point[i][2] < 0)
			continue;
		rootInd = point[i][2];
		if (point[i][4] < 0)
			continue;
		rightInd = point[i][4];
		if (point[rightInd][2] == rootInd )
			continue;
		point[rightInd][3] = rootInd;
		point[rootInd][4] = point[rightInd][4];
		point[rightInd][4] = -1;
		point[rightInd][2] = rootInd;
	}

}
void	BuildTree(PSTRUCTANALYZERHCR pThis, PSPLITPATHHCR pCurPatht)
{
	//pUniStruct->treeNode->Children->symbol = 0xff41;  // a
	//pUniStruct->treeN ode->Children++;
	//pUniStruct->treeNode->Children->symbol = 0xff43;  // c
	pThis->treeNode;
	pCurPatht->group->uText[0];
}
void	PruningTree(){}
void	Tree2Expr(){}

// this is my implementation  Hu Yang
void m_Analysis(PSPLITPATHHCR pSplitPath, const short nCandPathNum,	PSTROKEINFOHCR pStrokeInfo, const short nStrokeNum,	unsigned short uCandidateResult[])
{

	
	//Calculate Position Relationship
	enum {NONE, RIGHT, LEFT, ABOVE, UNDER, ENCLOSE, BE_ENCLOSED, UPPER_RIGHT, LOWER_LEFT};
	int *Confidence[MAXNODE];
	TEXTCODE  * pTempText;
	TEXTCODE RecognizeResult[HC_MAX_GROUP * FP_CHNCAND_NUM];
	PSPLITPATHHCR pCurPath;
	PGROUPINFOHCR pGroupInfo;
	RECTHCR GroupRect[HC_MAX_GROUP], CurrentGroupRect;
	int nPathID = 0,nGroupID = 0,nCandID = 0;
	UniTree _t[MAXNODE];
	for ( nPathID = 0, pCurPath = pSplitPath; nPathID < nCandPathNum; nPathID++, pCurPath++)
	{
		int nGroupNum  = pCurPath->nGroupNum;
		pTempText = RecognizeResult;
		// get the recognize candidates
		for (nGroupID = 0; nGroupID < nGroupNum; nGroupID++)
		{
			for (nCandID = 0; nCandID < pCurPath->group[nGroupID].nCandCount; nCandID++)
			{
				pTempText[nCandID] = pCurPath->group[nGroupID].uText[nCandID];
			}
			pTempText += FP_CHNCAND_NUM;   // 10 candidates
		}
		// get the bounding box information
		for (nGroupID = 0, pGroupInfo = &pCurPath->group[0]; nGroupID < nGroupNum; nGroupID++, pGroupInfo++)
		{
			int nStrokeID = pGroupInfo->nStrokeBe;
			CurrentGroupRect = pStrokeInfo[nStrokeID].rect;
			nStrokeID++;
			while (nStrokeID <= pGroupInfo->nStrokeEn)
			{
				Union2(&pStrokeInfo[nStrokeID].rect, &CurrentGroupRect);
				nStrokeID++;
			}
			GroupRect[nGroupID] = CurrentGroupRect;	
		}
		// initialize confidence matrix
		int nPointBuf[MAXNODE * 8];
		Confidence[0] = & nPointBuf[0];
		for (nGroupID = 1; nGroupID < nGroupNum; nGroupID++)
			Confidence[nGroupID] = Confidence[nGroupID-1] + 8;
		for (int i = 0; i < nGroupNum; i++)
			for (int j = 0; j < 8; j++)
				Confidence[i][j] = -1;
		int ppnEachOther[HC_MAX_GROUP][HC_MAX_GROUP];
		CalculatePositionRelationship(nGroupNum,RecognizeResult, Confidence, GroupRect, ppnEachOther);
	
		
	//  Build Tree
		// initialize tree
		_t->NodeID = -1;
			for (int i = 0; i < MAXNODE; i++)
			{
				_t->Children[i] = new UniTree;
				_t->Children[i]->symbol = 0;
			}
		for (int i = 0; i < nGroupNum; i++)
		{
			// 左右
			if (Confidence[i][4] == i+1)
			{
				_t->Children[i]->symbol = RecognizeResult[i * FP_CHNCAND_NUM];
			}
	
			if (i == nGroupNum -1 )
			{
				_t->Children[i]->symbol = RecognizeResult[i * FP_CHNCAND_NUM];
			}

			// 包围
			if (Confidence[i][2] == i+1)
			{
				

			}
			// 指数
			for (int j = 0; j < nGroupNum; j++)
			{
				if (Confidence[i][5] == j)
				{
					_t->Children[j]->symbol = RecognizeResult[j * FP_CHNCAND_NUM];	
					//_t->Children[j]->parent = _t;
					_t->Children[i]->symbol = RecognizeResult[i * FP_CHNCAND_NUM];

				}
			}
			// 分数
			if (Confidence[i][1] == i+1)
			{
				
				_t->Children[i] = _t;
			}
		}

	unsigned short Final[MAXNODE];
	int CurrentNode = 0, TotalNode = 6;
	while (CurrentNode < TotalNode)
	{
	Final[CurrentNode] = _t->Children[CurrentNode]->symbol;
	CurrentNode++;
	}
	printf("%2x %2x %2x %2x %2x %2x",Final[0],Final[1],Final[2],Final[3],Final[4],Final[5]);
	}

}
/********
Below is the implementation by Jiang Han.
/************/

void SlashPostprocess(TEXTCODE *ptcResult, 
					  RECTHCR *pbbxEach, 
					  int *pnCenterX,
					  int *pnCenterY,
					  int **ppnPoint, 
					  int nSymbolCount)
{
	int i, j;
	unsigned uAboveMark, uUnderMark;
	int nAboveIndex, nUnderIndex;
	int nAboveXDist, nUnderXDist;

	assert(ptcResult && pbbxEach && ppnPoint && nSymbolCount > 0);

	for (i = 1; i < nSymbolCount - 1; ++i)
	{
		// 找出斜线除号
		if (ptcResult[i] != (TEXTCODE)('/'))
		{
			continue;
		}

		// 标记包含符号
		uAboveMark = 0;
		uUnderMark = 0;
		for (j = 0; j < nSymbolCount; ++j)
		{
			if (
				i != j
				&& (TEXTCODE)('0') <= ptcResult[j]
			&& ptcResult[j] <= (TEXTCODE)('9')
				&& Overlap2(pbbxEach + i, pbbxEach + j, BBO_AREA) > 0.5 * Area2(pbbxEach + j)
				)
			{
				if (pnCenterY[j] < pnCenterY[i])
				{
					uAboveMark |= 1 << j;
				}
				else
				{
					uUnderMark |= 1 << j;
				}
			}
		}

		// 取得最靠近的符号索引
		nAboveIndex = -1;
		nUnderIndex = -1;
		nAboveXDist = 0;
		nUnderXDist = 0xFFFF;
		for (j = 0; j < nSymbolCount; ++j)
		{
			if ( (uAboveMark >> j) & 1 )
			{
				if ( pnCenterX[j] > nAboveXDist )
				{
					nAboveIndex = j;
					nAboveXDist = pnCenterX[j];
				}
			}
			if ( (uUnderMark >> j) & 1 )
			{
				if ( pnCenterX[j] < nUnderXDist )
				{
					nUnderIndex = j;
					nUnderXDist = pnCenterX[j];
				}
			}
		}

		// 修改 point 矩阵
		if (nAboveIndex > -1)
		{
			int count = 0;
			int k;
			if (ppnPoint[i][3] > -1 && ppnPoint[ppnPoint[i][3]][4] == i)
			{
				ppnPoint[ppnPoint[i][3]][4] = -1;
				ppnPoint[i][3] = nAboveIndex;
			}
			for (k = nAboveIndex; k > -1; k = ppnPoint[k][3])
			{
				++count;
				if (count > 100) break;
				if (
					ppnPoint[k][5] > -1
					&& (ppnPoint[k][3] == -1 || ppnPoint[ppnPoint[k][3]][5] != ppnPoint[k][5])
					&&
					(ptcResult[ppnPoint[k][5]] == (TEXTCODE)('+')
					|| ptcResult[ppnPoint[k][5]] == (TEXTCODE)('-')
					|| ptcResult[ppnPoint[k][5]] == (TEXTCODE)('*')
					|| ptcResult[ppnPoint[k][5]] == (TEXTCODE)('/'))
					)
				{
					int l;
					if (ppnPoint[i][3] > -1)
					{
						ppnPoint[ppnPoint[i][3]][4] = -1;
					}
					ppnPoint[ppnPoint[k][5]][4] = k;
					ppnPoint[i][3] = nAboveIndex;
					ppnPoint[nAboveIndex][4] = i;
					for (l = ppnPoint[k][4]; l > -1; l = ppnPoint[l][4])
					{
						if (ppnPoint[l][5] == ppnPoint[k][5])
						{
							ppnPoint[l][5] = -1;						
						}
					}
					ppnPoint[k][5] = -1;

					break;
				}
				if (k == 0)
				{
					if (ppnPoint[i][3] > -1)
					{
						ppnPoint[ppnPoint[i][3]][4] = -1;
					}
					ppnPoint[i][3] = nAboveIndex;
					ppnPoint[nAboveIndex][4] = i;
					ppnPoint[k][5] = -1;
				}
			}
		}
		if (nUnderIndex > -1)
		{
			int count = 0;
			int k;
			if (ppnPoint[i][4] > -1 && ppnPoint[ppnPoint[i][4]][3] == i)
			{
				ppnPoint[ppnPoint[i][4]][3] = -1;
				if (ppnPoint[nUnderIndex][4] != i)
				{
					ppnPoint[i][4] = nUnderIndex;
				}
				else
				{
					//ppnPoint[i][4] = -1;
				}

			}
			for (k = nUnderIndex; k > -1; k = ppnPoint[k][4])
			{
				++count;
				if (count > 100) break;
				if (
					k < nSymbolCount - 1
					&& ppnPoint[k + 1][5] == k
					&&
					(ptcResult[k + 1] == (TEXTCODE)('+')
					|| ptcResult[k + 1] == (TEXTCODE)('-')
					|| ptcResult[k + 1] == (TEXTCODE)('*')
					|| ptcResult[k + 1] == (TEXTCODE)('/'))
					)
				{
					if (ppnPoint[i][4] > -1)
					{
						ppnPoint[ppnPoint[i][4]][3] = -1;
					}
					ppnPoint[i][4] = nUnderIndex;
					ppnPoint[k + 1][3] = ppnPoint[k + 1][5];
					ppnPoint[k + 1][5] = -1;
					ppnPoint[nUnderIndex][3] = i;
					ppnPoint[k][4] = k + 1;
					break;
				}
				if (k == nSymbolCount - 1)
				{
					if (ppnPoint[i][4] > -1)
					{
						ppnPoint[ppnPoint[i][4]][3] = -1;
					}
					ppnPoint[i][4] = nUnderIndex;
					ppnPoint[nUnderIndex][3] = i;
					ppnPoint[k][4] = -1;
					break;
				}	
			}
		}
	}
}


// 空间位置关系的判断
static void JudgePositionRelations(int symbolcount, TEXTCODE* presult, int** point,
								   PRECTHCR pRect)
{
	int i, j;
	enum {NONE, RIGHT, LEFT, ABOVE, UNDER, ENCLOSE, BE_ENCLOSED, UPPER_RIGHT, LOWER_LEFT,UPPER_LEFT,LOWER_RIGHT};

	TEXTCODE ptcResult[HC_MAX_GROUP];
	RECTHCR pbbxEach[HC_MAX_GROUP];
	int pnCenterX[HC_MAX_GROUP];
	int pnCenterY[HC_MAX_GROUP];

	int ppnEachOther[HC_MAX_GROUP][HC_MAX_GROUP];

	for (i = 0; i < symbolcount; ++i)
	{
		for (j = 0; j < symbolcount; ++j)
		{
			ppnEachOther[i][j] = NONE;
		}
	}
	// 取得首选识别结果及各字符外接框、中心等
	for (i = 0; i != symbolcount; ++i)
	{
		ptcResult[i] = presult[i*FP_CHNCAND_NUM];
		pbbxEach[i] = pRect[i];
		CenterPoint2(pnCenterX + i, pnCenterY + i, pbbxEach + i);
	}

	// 下面判断每个ppnEachOther[i][j]应取何值
	for (i = 0; i < symbolcount - 1; ++i)
	{
		for (j = i + 1; j < symbolcount; ++j)
		{
			// i 是根号，j 被 i 包围情形
			if (ptcResult[i] == SQRT_SYM_CODE)
			{
				if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_AREA) > 0.5 * Area2(pbbxEach + j))
				{
					ppnEachOther[i][j] = ENCLOSE;
					ppnEachOther[j][i] = BE_ENCLOSED;
					continue;
				}
				else if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_AREA) > 0.1f * Area2((pbbxEach + j))
					&&  ptcResult[j] >= '0' && ptcResult[j] <= '9')
				{
					ppnEachOther[i][j] = ENCLOSE;
					ppnEachOther[j][i] = BE_ENCLOSED;
					continue;
				}
			}

			// j 是根号，i 被 j 包围情形
			if (ptcResult[j] == SQRT_SYM_CODE)
			{
				if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_AREA) > 0.5 * Area2(pbbxEach + i))
				{
					ppnEachOther[i][j] = BE_ENCLOSED;
					ppnEachOther[j][i] = ENCLOSE;
					continue;
				}
				else if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_AREA) > 0.1f * Area2((pbbxEach + i))
					&&  ptcResult[i] >= '0' && ptcResult[i] <= '9')
				{
					ppnEachOther[i][j] = BE_ENCLOSED;
					ppnEachOther[j][i] = ENCLOSE;
					continue;
				}
			}

			// i 为分数线(横线), 上/下符号为 j
			if ((ptcResult[i] == 0xff0d || ptcResult[i] == 0x2014 ||ptcResult[i] == 0x2215) &&
				RECT_WIDTH(pbbxEach + i) >= 1.5f * RECT_HEIGHT(pbbxEach + i))
			{
				if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) >= 0.1f * RECT_WIDTH(pbbxEach + j)
					|| Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) >= 0.8f * RECT_WIDTH(pbbxEach + j))
				{
					// 上符号为 j
					if (pnCenterY[j] < pnCenterY[i] &&
						pnCenterY[i] - pnCenterY[j] < 1.1f * max(RECT_WIDTH(pbbxEach + i), RECT_HEIGHT(pbbxEach + j)))
					{
						ptcResult[i] =0x2215;
						presult[i * FP_CHNCAND_NUM] = 0x2215;
						ppnEachOther[i][j] = ABOVE;
						ppnEachOther[j][i] = UNDER;
						continue;
					}
					// 下符号为 j
					else if (pnCenterY[j] > pnCenterY[i] &&
						pnCenterY[j] - pnCenterY[i] < 1.1f * max(RECT_WIDTH(pbbxEach + i), RECT_HEIGHT(pbbxEach + j)))
					{
						ptcResult[i] = 0x2215;
						presult[i * FP_CHNCAND_NUM] = 0x2215;
						ppnEachOther[i][j] = UNDER;
						ppnEachOther[j][i] = ABOVE;
						continue;
					}
				}
			}

			// j 为分数线(横线), 上/下符号为 i
			if ((ptcResult[j] == 0xff0d || ptcResult[j] ==0x2215) &&
				RECT_WIDTH(pbbxEach + j) >= 1.5f * RECT_HEIGHT(pbbxEach + j))
			{
				if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) >= 0.1f * RECT_WIDTH(pbbxEach + i)
					|| Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) >= 0.8f * RECT_WIDTH(pbbxEach + i))
				{
					// 上符号为 i
					if (pnCenterY[i] < pnCenterY[j] &&
						pnCenterY[j] - pnCenterY[i] < 1.1f * max(RECT_WIDTH(pbbxEach + j), RECT_HEIGHT(pbbxEach + i)))
					{
						ptcResult[j] = 0x2215;
						presult[j * FP_CHNCAND_NUM] = 0x2215;
						ppnEachOther[j][i] = ABOVE;
						ppnEachOther[i][j] = UNDER;
						continue;
					}
					// 下符号为 i
					else if (pnCenterY[i] > pnCenterY[j] &&
						pnCenterY[i] - pnCenterY[j] < 1.1f * max(RECT_WIDTH(pbbxEach + j), RECT_HEIGHT(pbbxEach + i)))
					{
						ptcResult[j] = 0x2215;
						presult[j * FP_CHNCAND_NUM] = 0x2215;
						ppnEachOther[j][i] = UNDER;
						ppnEachOther[i][j] = ABOVE;
						continue;
					}
				}
			}
			// i 为累加号sigma, 上/下符号为 j
			if (ptcResult[i] == 0x2211  &&
				RECT_WIDTH(pbbxEach + i) >= 1.5f * RECT_HEIGHT(pbbxEach + i))
			{
				if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) >= 0.1f * RECT_WIDTH(pbbxEach + i)
					|| Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) >= 0.8f * RECT_WIDTH(pbbxEach + i))
				{
					// 上符号为 j
					if (pnCenterY[j] < pnCenterY[i] &&
						pnCenterY[i] - pnCenterY[j] < 1.1f * max(RECT_WIDTH(pbbxEach + i), RECT_HEIGHT(pbbxEach + j)))
					{
						ptcResult[i] =0x2211;
						presult[i * FP_CHNCAND_NUM] = 0x2211;
						ppnEachOther[i][j] = ABOVE;
						ppnEachOther[j][i] = UNDER;
						continue;
					}
					// 下符号为 j
					else if (pnCenterY[j] > pnCenterY[i] &&
						pnCenterY[j] - pnCenterY[i] < 1.1f * max(RECT_WIDTH(pbbxEach + i), RECT_HEIGHT(pbbxEach + j)))
					{
						ptcResult[i] = 0x2211;
						presult[i * FP_CHNCAND_NUM] = 0x2211;
						ppnEachOther[i][j] = UNDER;
						ppnEachOther[j][i] = ABOVE;
						continue;				
					}
				}
			}
			// j 为累加号sigma, 上/下符号为 i
			if ( ptcResult[j] ==0x2211 &&
				RECT_WIDTH(pbbxEach + j) >= 1.5f * RECT_HEIGHT(pbbxEach + j))
			{
				if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) >= 0.1f * RECT_WIDTH(pbbxEach + i)
					|| Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) >= 0.8f * RECT_WIDTH(pbbxEach + i))
				{
					// 上符号为 i
					if (pnCenterY[i] < pnCenterY[j] &&
						pnCenterY[j] - pnCenterY[i] < 1.1f * max(RECT_WIDTH(pbbxEach + j), RECT_HEIGHT(pbbxEach + i)))
					{
						ptcResult[j] = 0x2211;
						presult[j * FP_CHNCAND_NUM] = 0x2211;
						ppnEachOther[j][i] = ABOVE;
						ppnEachOther[i][j] = UNDER;
						continue;
					}
					// 下符号为 i
					else if (pnCenterY[i] > pnCenterY[j] &&
						pnCenterY[i] - pnCenterY[j] < 1.1f * max(RECT_WIDTH(pbbxEach + j), RECT_HEIGHT(pbbxEach + i)))
					{
						ptcResult[j] = 0x2211;
						presult[j * FP_CHNCAND_NUM] = 0x2211;
						ppnEachOther[j][i] = UNDER;
						ppnEachOther[i][j] = ABOVE;
						continue;
					}
				}
			}
	

			// i 的左上符号为 j，即可能为积分的情形
			if (pbbxEach[i].nTop < pbbxEach[j].nTop + 0.25f * RECT_HEIGHT(pbbxEach + j) &&
				pbbxEach[i].nBottom < pbbxEach[j].nBottom - 0.5f * RECT_HEIGHT(pbbxEach + j) &&
				pbbxEach[j].nLeft > pnCenterX[i] &&
				pnCenterX[j] > pbbxEach[i].nRight &&
				pbbxEach[j].nLeft - pbbxEach[i].nRight < 1.0 * max(RECT_WIDTH(pbbxEach + j), RECT_HEIGHT(pbbxEach + j)) &&
				((ptcResult[i] == 0xff0d && ptcResult[j] != 0xff0d) || (ptcResult[i] != 0xff0d && pbbxEach[j].nTop >= pbbxEach[i].nTop)))
			{
				if (ptcResult[i] == 0xff0d && 
					(pbbxEach[j].nTop + 0.33f * RECT_HEIGHT(pbbxEach + j) <= pnCenterY[i] || 
					pbbxEach[j].nTop + 0.2f * RECT_HEIGHT(pbbxEach + j) < pbbxEach[i].nBottom))
				{
					;
				}
				else if ((ptcResult[j] >= '0' && ptcResult[j] <= '9') || ptcResult[j] == ')')
				{
					ppnEachOther[i][j] = LOWER_RIGHT;
					ppnEachOther[j][i] = UPPER_LEFT;
					continue;
				}
				else if (ptcResult[j] != 0xff0d)
				{
					ppnEachOther[i][j] = LOWER_RIGHT;
					ppnEachOther[j][i] = UPPER_LEFT;
					continue;
				}
			}
			// j 的左上符号为 i，即可能为积分的情形
			else if ((pbbxEach[j].nTop < pbbxEach[i].nTop + 0.25f * RECT_HEIGHT(pbbxEach + i) &&
				pbbxEach[j].nBottom < pbbxEach[i].nBottom - 0.5f * RECT_HEIGHT(pbbxEach + i)) &&
				pbbxEach[j].nRight < pnCenterX[i] &&
				pnCenterX[j] < pbbxEach[i].nLeft &&
				pbbxEach[i].nLeft - pbbxEach[j].nRight < 1.0 * max(RECT_WIDTH(pbbxEach + i), RECT_HEIGHT(pbbxEach + i)) &&
				((ptcResult[j] == 0xff0d && ptcResult[i] != 0xff0d) ||( ptcResult[j] != 0xff0d && pbbxEach[i].nTop >= pbbxEach[j].nTop)))
			{
				if (ptcResult[j] == 0xff0d && 
					(pbbxEach[i].nTop + 0.33f * RECT_HEIGHT(pbbxEach + i) <= pnCenterY[j] ||
					pbbxEach[i].nTop + 0.2f * RECT_HEIGHT(pbbxEach + i) < pbbxEach[j].nBottom))
				{
					;
				}
				else if ((ptcResult[i] >= '0' && ptcResult[i] <= '9') || ptcResult[i] == ')')
				{
					ppnEachOther[i][j] = LOWER_RIGHT;
					ppnEachOther[j][i] = UPPER_LEFT;
					continue;
				}
				else if (ptcResult[i] != 0xff0d)
				{
					ppnEachOther[i][j] = LOWER_RIGHT;
					ppnEachOther[j][i] = UPPER_LEFT;
					continue;
				}
			}


			// i 的左下符号为 j，即可能为乘幂形式的情形
			if (pbbxEach[i].nTop < pbbxEach[j].nTop + 0.25f * RECT_HEIGHT(pbbxEach + j) &&
				pbbxEach[i].nBottom < pbbxEach[j].nBottom - 0.5f * RECT_HEIGHT(pbbxEach + j) &&
				pbbxEach[j].nRight < pnCenterX[i] &&
				pnCenterX[j] < pbbxEach[i].nLeft &&
				pbbxEach[i].nLeft - pbbxEach[j].nRight < 1.0 * max(RECT_WIDTH(pbbxEach + j), RECT_HEIGHT(pbbxEach + j)) &&
				((ptcResult[i] == 0xff0d && ptcResult[j] != 0xff0d) || (ptcResult[i] != 0xff0d && pbbxEach[j].nTop >= pbbxEach[i].nTop)))

			{
				if (ptcResult[i] == 0xff0d && 
					(pbbxEach[j].nTop + 0.33f * RECT_HEIGHT(pbbxEach + j) <= pnCenterY[i] || 
					pbbxEach[j].nTop + 0.2f * RECT_HEIGHT(pbbxEach + j) < pbbxEach[i].nBottom))
				{
					;
				}
				else if ((ptcResult[j] >= 0xff10 && ptcResult[j] <= 0xff19) || ptcResult[j] == 0xff09 )
				{
					ppnEachOther[i][j] = LOWER_LEFT;
					ppnEachOther[j][i] = UPPER_RIGHT;
					continue;
				}
				else if (ptcResult[j] != 0xff0d)
				{
					ppnEachOther[i][j] = LOWER_LEFT;
					ppnEachOther[j][i] = UPPER_RIGHT;
					continue;
				}
			}
			// i 的右上符号为 j，即可能为乘幂形式的情形
			else if ((pbbxEach[j].nTop < pbbxEach[i].nTop + 0.25f * RECT_HEIGHT(pbbxEach + i) &&
				pbbxEach[j].nBottom < pbbxEach[i].nBottom - 0.5f * RECT_HEIGHT(pbbxEach + i)) &&
				pbbxEach[i].nRight < pnCenterX[j] &&
				pnCenterX[i] < pbbxEach[j].nLeft &&
				pbbxEach[j].nLeft - pbbxEach[i].nRight < 1.0 * max(RECT_WIDTH(pbbxEach + i), RECT_HEIGHT(pbbxEach + i)) &&
				//ptcResult[j] != '.' &&
				//ptcResult[i] != '.' &&
				((ptcResult[j] == 0xff0d && ptcResult[i] != 0xff0d) ||( ptcResult[j] != 0xff0d && pbbxEach[i].nTop >= pbbxEach[j].nTop)))
			{
				if (ptcResult[j] == 0xff0d && 
					(pbbxEach[i].nTop + 0.33f * RECT_HEIGHT(pbbxEach + i) <= pnCenterY[j] ||
					pbbxEach[i].nTop + 0.2f * RECT_HEIGHT(pbbxEach + i) < pbbxEach[j].nBottom))
				{
					;
				}
				else if ((ptcResult[i] >= '0' && ptcResult[i] <= '9') || ptcResult[i] == ')')
				{
					ppnEachOther[j][i] = LOWER_LEFT;
					ppnEachOther[i][j] = UPPER_RIGHT;
					continue;
				}
				else if (ptcResult[i] != 0xff0d)
				{
					ppnEachOther[j][i] = LOWER_LEFT;
					ppnEachOther[i][j] = UPPER_RIGHT;
					continue;
				}
			}

			// i j 为左右关系的一般判定
			if (Overlap2(pbbxEach + i, pbbxEach + j, BBO_VERT) > 
				0.5 * min(RECT_HEIGHT(pbbxEach + i), RECT_HEIGHT(pbbxEach + j)) &&
				abs(pnCenterY[i] - pnCenterY[j]) < 
				0.5 * max(RECT_HEIGHT(pbbxEach + i), RECT_HEIGHT(pbbxEach + j)))
			{
				// i 左 j 右
				if (pbbxEach[j].nLeft > pnCenterX[i] - 0.2 * RECT_WIDTH(pbbxEach + j) &&
					pbbxEach[i].nRight < pnCenterX[j] + 0.2 * RECT_WIDTH(pbbxEach + j) &&
					pnCenterY[i] >= pbbxEach[j].nTop - 0.1 * RECT_HEIGHT(pbbxEach + j) &&
					pnCenterY[i] <= pbbxEach[j].nBottom + 0.1 * RECT_HEIGHT(pbbxEach + j) &&
					pnCenterY[j] >= pbbxEach[i].nTop - 0.1 * RECT_HEIGHT(pbbxEach + i) &&
					pnCenterY[j] <= pbbxEach[i].nBottom + 0.1 * RECT_HEIGHT(pbbxEach + i) &&
					RECT_WIDTH(pbbxEach + i) <= 2 * RECT_HEIGHT(pbbxEach + i) &&
					RECT_WIDTH(pbbxEach + j) <= 2 * RECT_HEIGHT(pbbxEach + j) &&
					pnCenterX[i] < pnCenterX[j])
				{
					ppnEachOther[i][j] = RIGHT;
					ppnEachOther[j][i] = LEFT;
					continue;
				}
				// j 左 i 右
				else if (pbbxEach[i].nLeft > pnCenterX[j] - 0.2 * RECT_WIDTH(pbbxEach + i) &&
					pbbxEach[j].nRight < pnCenterX[i] + 0.2 * RECT_WIDTH(pbbxEach + i) &&
					pnCenterY[j] >= pbbxEach[i].nTop - 0.1 * RECT_HEIGHT(pbbxEach + i) &&
					pnCenterY[j] <= pbbxEach[i].nBottom + 0.1 * RECT_HEIGHT(pbbxEach + i) &&
					pnCenterY[i] >= pbbxEach[j].nTop - 0.1 * RECT_HEIGHT(pbbxEach + j) &&
					pnCenterY[i] <= pbbxEach[j].nBottom + 0.1 * RECT_HEIGHT(pbbxEach + j) &&
					RECT_WIDTH(pbbxEach + j) <= 2 * RECT_HEIGHT(pbbxEach + j) &&
					RECT_WIDTH(pbbxEach + i) <= 2 * RECT_HEIGHT(pbbxEach + i) &&
					pnCenterX[j] < pnCenterX[i])
				{
					ppnEachOther[i][j] = LEFT;
					ppnEachOther[j][i] = RIGHT;
					continue;
				}
			}

			// i 的右符号为 j 的其他情形，针对横线之间判断关系
			if (  //else if  huyang
				(abs(pnCenterY[i] - pnCenterY[j]) 
				< 0.4 * max(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j))
				|| (ptcResult[i] == 0xff0d && ptcResult[j] == 0xff0d && abs(pnCenterY[i] - pnCenterY[j]) 
				< 0.8 * max(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j)))
				)
				&&	pbbxEach[j].nLeft > pnCenterX[i]
			&&	pbbxEach[i].nRight < pnCenterX[j]
			&&  (RECT_WIDTH(pbbxEach + i) > 2 * RECT_HEIGHT(pbbxEach + i) || RECT_WIDTH(pbbxEach + j) > 2 * RECT_HEIGHT(pbbxEach + j))
				&&  abs(pnCenterX[i] - pnCenterX[j])
				< 5 * max(max(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j)),max(RECT_HEIGHT(pbbxEach + i), RECT_HEIGHT(pbbxEach + j)))
				&& (Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) <= 0.2 * max(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j)))
				&& pnCenterX[i] < pnCenterX[j]
			)
			{

				ppnEachOther[i][j] = RIGHT;
				ppnEachOther[j][i] = LEFT;
			}
			// j 的右符号为 i 的其他情形，针对横线之间判断关系
			else if (
				(abs(pnCenterY[j] - pnCenterY[i]) 
				< 0.4 * max(RECT_WIDTH(pbbxEach + j), RECT_WIDTH(pbbxEach + i))
				|| (ptcResult[j] == 0xff0d && ptcResult[i] == 0xff0d && abs(pnCenterY[j] - pnCenterY[i]) 
				< 0.8 * max(RECT_WIDTH(pbbxEach + j), RECT_WIDTH(pbbxEach + i)))
				)
				&&	pbbxEach[i].nLeft > pnCenterX[j]
			&&	pbbxEach[j].nRight < pnCenterX[i]
			&&  (RECT_WIDTH(pbbxEach + j) > 2 * RECT_HEIGHT(pbbxEach + j) || RECT_WIDTH(pbbxEach + i) > 2 * RECT_HEIGHT(pbbxEach + i))
				&&  abs(pnCenterX[j] - pnCenterX[i])
				< 5 * max(max(RECT_WIDTH(pbbxEach + j), RECT_WIDTH(pbbxEach + i)),max(RECT_HEIGHT(pbbxEach + j), RECT_HEIGHT(pbbxEach + i)))
				&& (Overlap2(pbbxEach + j, pbbxEach + i, BBO_HORZ) <= 0.2 * max(RECT_WIDTH(pbbxEach + j), RECT_WIDTH(pbbxEach + i)))
				&& pnCenterX[j] < pnCenterX[i]
			)
			{
				ppnEachOther[i][j] = LEFT;
				ppnEachOther[j][i] = RIGHT;
			}
			// added by Yang Hu on 2013-10-03
			// 左右结构的另一判断, 针对 '1'
			else if (
				ptcResult[i] == '1'
				&&	Overlap2(pbbxEach + i, pbbxEach + j, BBO_VERT) > 0.6 * RECT_HEIGHT(pbbxEach + i)
				&&  Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) < 0.2 * RECT_WIDTH(pbbxEach + j)
				&&  3 * RECT_WIDTH(pbbxEach + i) < RECT_HEIGHT(pbbxEach + j)
				&&  pbbxEach[i].nRight < pnCenterX[j]
			)
			{
				ppnEachOther[i][j] = RIGHT;
				ppnEachOther[j][i] = LEFT;
			}
			// added by Yang Hu on 2013-10-03
			// 左右结构的另一判断, 针对 '1'
			else if (
				ptcResult[j] == '1'
				&&	Overlap2(pbbxEach + j, pbbxEach + i, BBO_VERT) > 0.6 * RECT_HEIGHT(pbbxEach + j)
				&&  Overlap2(pbbxEach + j, pbbxEach + i, BBO_HORZ) < 0.2 * RECT_WIDTH(pbbxEach + i)
				&&  3 * RECT_WIDTH(pbbxEach + j) < RECT_HEIGHT(pbbxEach + i)
				&&  pbbxEach[j].nRight < pnCenterX[i]
			)
			{
				ppnEachOther[i][j] = LEFT;
				ppnEachOther[j][i] = RIGHT;
			}
			// added by Yang Hu on 2013-10-03
			// 左右结构的另一判断，规则较松，为查缺补漏
			else if ((abs(pnCenterY[i] - pnCenterY[j]) < 0.5 * max(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j)) ||
				abs(pnCenterY[i] - pnCenterY[j]) < 0.5 * max(RECT_HEIGHT(pbbxEach + i), RECT_HEIGHT(pbbxEach + j))) &&
				(abs(pnCenterX[i] - pnCenterX[j]) < 2 * max(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j)) ||
				abs(pnCenterX[i] - pnCenterX[j]) < 2 * max(RECT_HEIGHT(pbbxEach + i), RECT_HEIGHT(pbbxEach + j))) &&
				(Overlap2(pbbxEach + i, pbbxEach + j, BBO_HORZ) <= 0.3 * min(RECT_WIDTH(pbbxEach + i), RECT_WIDTH(pbbxEach + j))) &&
				pbbxEach[i].nTop < pbbxEach[j].nBottom &&
				pbbxEach[j].nTop < pbbxEach[i].nBottom && 
				(ptcResult[j] == 0xff0d ||ptcResult[i] == 0xff0d))
			{
				if (pnCenterX[i] > pnCenterX[j])
				{
					ppnEachOther[i][j] = LEFT;
					ppnEachOther[j][i] = RIGHT;
				}
				else
				{
					ppnEachOther[i][j] = RIGHT;
					ppnEachOther[j][i] = LEFT;
				}
			}
		}
	}

	// 利用每两个符号间的关系得到 point 矩阵
	for (i = 0; i < symbolcount; ++i)
	{
		int nMinIndex;
		unsigned int uMinDist;

		// 左上
		nMinIndex = -1;
		uMinDist = HCR_UINT16_MAX;
		for (j = 0; j < symbolcount; ++j)
		{
			unsigned int uDist;

			if (i == j || ppnEachOther[i][j] != UPPER_LEFT)
			{
				continue;
			}

			uDist = (pnCenterX[i] - pnCenterX[j]) * (pnCenterX[i] - pnCenterX[j])
				+ (pnCenterY[i] - pnCenterY[j]) * (pnCenterY[i] - pnCenterY[j]);   //Euclid
			if (uDist < uMinDist)
			{
				uMinDist = uDist;
				nMinIndex = j;
			}
		}
		if (nMinIndex >= 0)
		{
			point[i][7] = nMinIndex;
		}

		// 左下
		nMinIndex = -1;
		uMinDist = HCR_UINT16_MAX;
		for (j = 0; j < symbolcount; ++j)
		{
			unsigned int uDist;

			if (i == j || ppnEachOther[i][j] != LOWER_LEFT)
			{
				continue;
			}

			uDist = (pnCenterX[i] - pnCenterX[j]) * (pnCenterX[i] - pnCenterX[j])
				+ (pnCenterY[i] - pnCenterY[j]) * (pnCenterY[i] - pnCenterY[j]);
			if (uDist < uMinDist)
			{
				uMinDist = uDist;
				nMinIndex = j;
			}
		}
		if (nMinIndex >= 0)
		{
			point[i][5] = nMinIndex;
		}

		// 被包围
		nMinIndex = -1;
		uMinDist = HCR_UINT16_MAX;
		for (j = 0; j < symbolcount; ++j)
		{
			unsigned int uDist;

			if (i == j || ppnEachOther[i][j] != BE_ENCLOSED)
			{
				continue;
			}

			// 上面过程保证了 j 为根号
			uDist = (pnCenterX[i] - pbbxEach[j].nLeft) 
				* (pnCenterX[i] - pbbxEach[j].nLeft)
				+ (pnCenterY[i] - pbbxEach[j].nTop)
				* (pnCenterY[i] - pbbxEach[j].nTop);

			if (uDist < uMinDist && point[i][5] != j)
			{
				uMinDist = uDist;
				nMinIndex = j;
			}
		}
		if (nMinIndex >= 0)
		{
			point[i][2] = nMinIndex;
		}

		// 上
		nMinIndex = -1;
		uMinDist = HCR_UINT16_MAX;
		for (j = 0; j < symbolcount; ++j)
		{
			unsigned int uDist;

			if (i == j || ppnEachOther[i][j] != ABOVE)
			{
				continue;
			}

			uDist = (pnCenterY[i] - pnCenterY[j]) * (pnCenterY[i] - pnCenterY[j]);
			if (uDist < uMinDist && point[i][2] != j && point[i][5] != j)
			{
				uMinDist = uDist;
				nMinIndex = j;
			}
		}
		if (nMinIndex >= 0)
		{
			point[i][1] = nMinIndex;
			point[nMinIndex][6] = i;
		}

		// 下
		nMinIndex = -1;
		uMinDist = HCR_UINT16_MAX;
		for (j = 0; j < symbolcount; ++j)
		{
			unsigned int uDist;

			if (i == j || ppnEachOther[i][j] != UNDER)
			{
				continue;
			}

			uDist = (pnCenterY[i] - pnCenterY[j]) * (pnCenterY[i] - pnCenterY[j]);
			if (uDist < uMinDist && point[i][2] != j && point[i][5] != j)
			{
				uMinDist = uDist;
				nMinIndex = j;
			}
		}
		if (nMinIndex >= 0)
		{
			point[i][6] = nMinIndex;
			point[nMinIndex][1] = i;
		}

		// 左/右
		nMinIndex = -1;
		uMinDist = HCR_UINT16_MAX;
		for (j = 0; j < symbolcount; ++j)
		{
			unsigned int uDist;

			if (i == j || ppnEachOther[i][j] != RIGHT)
			{
				continue;
			}

			if (ptcResult[i] != '.' && ptcResult[j] != '.' && ptcResult[i] != '-' && ptcResult[j] != '-')
			{
				uDist = (pbbxEach[i].nRight - pbbxEach[j].nLeft) 
					* (pbbxEach[i].nRight - pbbxEach[j].nLeft)
					+ 3 * (pnCenterY[i] - pnCenterY[j]) * (pnCenterY[i] - pnCenterY[j]);
			}
			else if (ptcResult[i] == '-' || ptcResult[i] == '-')
			{
				uDist = (pbbxEach[i].nRight - pbbxEach[j].nLeft) 
					* (pbbxEach[i].nRight - pbbxEach[j].nLeft)
					+ (pnCenterY[i] - pnCenterY[j]) * (pnCenterY[i] - pnCenterY[j]) / 5;
			}
			else
			{
				uDist = (pbbxEach[i].nRight - pbbxEach[j].nLeft) 
					* (pbbxEach[i].nRight - pbbxEach[j].nLeft)
					+ 0 * (pnCenterY[i] - pnCenterY[j]) * (pnCenterY[i] - pnCenterY[j]);
			}
			if (uDist < uMinDist)
			{
				uMinDist = uDist;
				nMinIndex = j;
			}
		}
		if (nMinIndex >= 0)
		{
			if (point[nMinIndex][3] >= 0)
			{
				int m = point[nMinIndex][3];
				if ( (int)uMinDist < (pbbxEach[m].nRight - pbbxEach[nMinIndex].nLeft) 
					* (pbbxEach[m].nRight - pbbxEach[nMinIndex].nLeft)
					+ 3 * (pnCenterY[nMinIndex] - pnCenterY[m]) * (pnCenterY[nMinIndex] - pnCenterY[m]) )
				{
					point[i][4] = nMinIndex;
					point[nMinIndex][3] = i;
				}
			}
			else
			{
				point[i][4] = nMinIndex;
				point[nMinIndex][3] = i;
			}
		}
	}
	for (i = 0; i < symbolcount; ++i)
	{
		for (j = 0; j < symbolcount; ++j)
		{
			if (point[i][4] == j && point[j][4] == i)
			{
				point[j][4] = -1;
				point[i][3] = -1;
				point[j][3] = i;
			}

			if (point[i][3] == j && point[j][3] == i)
			{
				point[j][3] = -1;
				point[i][4] = -1;
				point[j][4] = i;
			}
		}
	}
	SlashPostprocess(ptcResult, pbbxEach, pnCenterX, pnCenterY, point, symbolcount);

	for (i = 0; i < symbolcount; ++i)
	{
		int rootInd, rightInd;
		if (point[i][2] < 0)
			continue;
		rootInd = point[i][2];
		if (point[i][4] < 0)
			continue;
		rightInd = point[i][4];
		if (point[rightInd][2] == rootInd || ptcResult[rightInd] >= '0' || ptcResult[rightInd] <= '9')
			continue;
		point[rightInd][3] = rootInd;
		point[rootInd][4] = point[rightInd][4];
		point[rightInd][4] = -1;
		point[rightInd][2] = rootInd;
	}

	// point 矩阵的后处理
	for (i = 0; i < symbolcount; ++i)
	{
		for (j = 0; j < symbolcount; ++j)
		{
			if (i == j)
			{
				continue;
			}
			if (point[i][4] >= 0 && point[j][4] == point[i][4])
			{
				int myth = point[j][4];
				if (point[j][5] == i)
				{
					point[j][4] = -1;
					point[myth][3] = i;
				}
				else if (point[j][1] != i && point[j][2] != i && point[j][3] != i
					&& point[j][4] != i && point[j][5] != i && point[j][6] != i
					&& pnCenterX[i] > pnCenterX[j]
				&& point[i][1] != j && point[i][2] != j && point[i][3] != j
					&& point[i][4] != j && point[i][5] != j && point[i][6] != j
					)
				{
					point[j][4] = i;
					point[i][3] = j;
				}
			}
		}
	}
}

void SACatagorizeSymbols(PSTRUCTANALYZERHCR pThis)
// 对符号进行分类
// 0 -- 根号，1 -- 分数线，2 -- 数字和小数点，3 -- 小写字母, 4 -- 运算符
{
	int nIter;
	for (nIter = 0; nIter < pThis->m_nSymbolNum; ++nIter)
	{
		// printf("%x\n ",pThis-> m_pRecResult[nIter]);
		if (pThis->m_pRecResult[nIter] == 0x221A)	// 0x221A为根号的内码
		{
			pThis->m_pnType[nIter] = 0;
		}
		else if ((pThis->m_pRecResult[nIter] == 0xff0d || pThis->m_pRecResult[nIter] == 0x2014 || pThis->m_pRecResult[nIter] == 0x2215)
			&& pThis->m_ppnSpacialRelation[nIter][1] > -1
			&& pThis->m_ppnSpacialRelation[nIter][6] > -1)	// 如果是横线且上下均有符号，认为是分数线
		{
			pThis->m_pnType[nIter] = 1;
			pThis->m_pRecResult[nIter] = 0xff0d;
		}
		else if ((pThis->m_pRecResult[nIter] == 0xff0d || pThis->m_pRecResult[nIter] == 0x2014)) // 减号
		{
			pThis->m_pnType[nIter] = 3;
			pThis->m_pRecResult[nIter] = 0xff0d;
		}
		else if ((0xff10 <= pThis->m_pRecResult[nIter] && pThis->m_pRecResult[nIter] <= 0xff19)	// 数字和小数点
			|| pThis->m_pRecResult[nIter] == 0xff0e)
		{
			pThis->m_pnType[nIter] = 2;
		}
		else if ((0xff41 <= pThis->m_pRecResult[nIter] && pThis->m_pRecResult[nIter] <= 0xff5a) || 
			(0xff21 <= pThis->m_pRecResult[nIter] && pThis->m_pRecResult[nIter] <= 0xff3a)||
			(0x03b1 <= pThis->m_pRecResult[nIter] && pThis->m_pRecResult[nIter] <= 0x03c9)||
			(0x0391 <= pThis->m_pRecResult[nIter] && pThis->m_pRecResult[nIter] <= 0x03a9)
			)// 小写字母，大写字母，希腊字母
			{
				pThis->m_pnType[nIter] = 2;
		}
			else //其他情况视作运算符
			{
				pThis->m_pnType[nIter] = 3;
		}
	}
}

static void SASetDominantRelations(PSTRUCTANALYZERHCR pThis)
// 设定（直接）统领关系矩阵
{
	int nIter;

	for (nIter = 0; nIter < pThis->m_nSymbolNum; ++nIter)
	{
		// 上方符号为分数线
		if (pThis->m_ppnSpacialRelation[nIter][1] >= 0
			&& pThis->m_pnType[pThis->m_ppnSpacialRelation[nIter][1]] == 1)		
		{
			pThis->m_ppnDomRelation[nIter][pThis->m_ppnSpacialRelation[nIter][1]] = -1;
			pThis->m_ppnDomRelation[pThis->m_ppnSpacialRelation[nIter][1]][nIter] = 1;
		}

		// 下方符号为分数线
		if (pThis->m_ppnSpacialRelation[nIter][6] >= 0
			&& pThis->m_pnType[pThis->m_ppnSpacialRelation[nIter][6]] == 1)		
		{
			pThis->m_ppnDomRelation[nIter][pThis->m_ppnSpacialRelation[nIter][6]] = -2;
			pThis->m_ppnDomRelation[pThis->m_ppnSpacialRelation[nIter][6]][nIter] = 2;
		}

		// 被根号包围
		if (pThis->m_ppnSpacialRelation[nIter][2] >= 0
			&& pThis->m_pnType[pThis->m_ppnSpacialRelation[nIter][2]] == 0)		
		{
			pThis->m_ppnDomRelation[nIter][pThis->m_ppnSpacialRelation[nIter][2]] = -1;
			pThis->m_ppnDomRelation[pThis->m_ppnSpacialRelation[nIter][2]][nIter] = 1;
		}

		// 上下都有分数线，比较长短确定统领关系
		if (pThis->m_ppnSpacialRelation[nIter][1] >= 0
			&& pThis->m_ppnSpacialRelation[nIter][6] >= 0
			&& pThis->m_pnType[pThis->m_ppnSpacialRelation[nIter][1]] == 1
			&& pThis->m_pnType[pThis->m_ppnSpacialRelation[nIter][6]] == 1
			&& pThis->m_pnType[nIter] != 1)		
		{
			int nIndex1 = pThis->m_ppnSpacialRelation[nIter][1];
			int nIndex2 = pThis->m_ppnSpacialRelation[nIter][6];
			pThis->m_ppnDomRelation[nIndex1][nIndex2]
			= (pThis->m_pnBoxWidth[nIndex1] > pThis->m_pnBoxWidth[nIndex2] ? 1 : -2);
			pThis->m_ppnDomRelation[nIndex2][nIndex1]
			= (pThis->m_pnBoxWidth[nIndex1] > pThis->m_pnBoxWidth[nIndex2] ? -1 : 2);
		}
	}
}

int SAFindFirst(int nValue, int *pnBegin, int *pnEnd)
// 在数组[pnBegin, pnEnd)中查找第一个值为nValue的元素，返回索引。若没有找到，返回-1。
{
	int* pnIter;
	for (pnIter = pnBegin; pnIter != pnEnd; ++pnIter)
	{
		if (*pnIter == nValue)
			return (int)(pnIter - pnBegin);
	}
	return -1;
}

static int SAGetMaxValue2(int *pnBegin, int *pnEnd)
// 求数组[pnBegin, pnEnd)的最大值
{
	int nMax = *pnBegin;
	int *pnIter;
	for (pnIter = pnBegin; pnIter != pnEnd; ++pnIter)
	{
		if (*pnIter > nMax)
			nMax = *pnIter;
	}
	return nMax;
}

static void SASetSymbolRanks2(PSTRUCTANALYZERHCR pThis)
// 设定符号级别
{
	int nIter, nRow, nCol;
	short nBeingDomFlag, nAddOneFlag, nEmptyFlag;

	// 数组m_pnRank初始化
	for (nIter = 0; nIter < pThis->m_nSymbolNum; ++nIter)
		pThis->m_pnRank[nIter] = -1;

	// 0级符号的设定
	for (nRow = 0; nRow != pThis->m_nSymbolNum; ++nRow)
	{
		nBeingDomFlag = 0;		// 标记当前符号是(true)否(false)被统领
		for (nCol = 0; nCol != pThis->m_nSymbolNum && !nBeingDomFlag; ++nCol)
		{
			if (pThis->m_ppnDomRelation[nRow][nCol] < 0)	// 当前符号nRow被nCol统领
				nBeingDomFlag = 1;
		}
		if (!nBeingDomFlag)
			pThis->m_pnRank[nRow] = 0;
	}

	// added by Yang Hu on 2013-10-03
	nEmptyFlag = 1;
	for (nRow = 0; nRow != pThis->m_nSymbolNum; ++nRow)
	{
		if (pThis->m_pnRank[nRow] == 0)
		{
			nEmptyFlag = 0;
			break;
		}
	}
	if (nEmptyFlag)
	{
		for (nRow = 0; nRow != pThis->m_nSymbolNum; ++nRow)
			pThis->m_pnRank[nRow] = 0;
		return;
	}

	// 只要m_pnRank中还有值为-1的元素，就需要继续给符号分级
	for (/*null*/; SAFindFirst(-1, pThis->m_pnRank, pThis->m_pnRank + pThis->m_nSymbolNum) > -1; /*null*/)
	{
		int nMax = SAGetMaxValue2(pThis->m_pnRank, pThis->m_pnRank + pThis->m_nSymbolNum);
		for (nRow = 0; nRow < pThis->m_nSymbolNum; ++nRow)
		{
			if (pThis->m_pnRank[nRow] == -1)
			{
				nAddOneFlag = 1;	// 标记当前符号是否为(nMax + 1)级
				for (nCol = 0; nCol < pThis->m_nSymbolNum; ++nCol)
				{
					// 如果被未知级别的符号统领，或者为(nMax + 1)级符号统领，则不可能同为(nMax + 1)级

					if (pThis->m_ppnDomRelation[nRow][nCol] < 0 
						&& (pThis->m_pnRank[nCol] < 0 || pThis->m_pnRank[nCol] == nMax + 1))
					{
						nAddOneFlag = 0;
						//bBreakFlag = true;
						break;
					}

					// 如果被nMax级符号统领，才有可能是(nMax + 1)级
					/*if (m_ppnDomRelation[nRow][nCol] < 0 && m_pnRank[nCol] == nMax)
					{
					nAddOneFlag = true;
					}*/
				}
				//if (bBreakFlag)
				//	break;
				if (nAddOneFlag)
					pThis->m_pnRank[nRow] = nMax + 1;
			}
		} // for (int nRow = 0; nRow < m_nSymbolNum; ++nRow)
	} // for (/*null*/; FindFirst(-1, m_pnRank, m_pnRank + m_nSymbolNum) > -1; /*null*/)
}

static void SAAnalyseBasicEqn(int nRank,
							  int nIndex, 
							  int nPosition, 
							  PSTRUCTANALYZERHCR pThis,
							  PSPLITPATHHCR pCurPath)
							  // nPosition主要是给分数线预备的，因为分数线有两种统领方式：向上和向下
{
	short i, j, nCount, nIter, nTemp;
	short nCurSymbol, nNextSymbol, nRightSymbol;
	short pnSymIndex[HC_MAX_GROUP];
	short nFirstSymFakeIndex, nFirstSymRealIndex;
	PTRINARYTREEHCR pChiefNode;
	// center x of symbol
	int nCenter[HC_MAX_GROUP];

	if (!pThis) return;
	if (nPosition != 1 && nPosition != 2) return;
	if (nIndex < -1) return;

	//============================================================
	// get symbol array
	nCount = 0;	// 符号个数计数
	if (nRank == 0)
	{
		for (nIter = 0; nIter < pThis->m_nSymbolNum; ++nIter)
		{
			if (pThis->m_pnRank[nIter] == 0)
			{
				pnSymIndex[nCount] = nIter;
				// center x
				nCenter[nCount] = pThis->m_pnBoxLeft[nIter] + 
					pThis->m_pnBoxWidth[nIter] / 2;
				nCount++;
			}
		}
	}
	else
	{
		for (nIter = 0; nIter < pThis->m_nSymbolNum; ++nIter)
		{
			if (pThis->m_pnRank[nIter] == nRank && 
				pThis->m_ppnDomRelation[nIter][nIndex] == -nPosition)
			{
				pnSymIndex[nCount] = nIter;
				// center x
				nCenter[nCount] = pThis->m_pnBoxLeft[nIter] + 
					pThis->m_pnBoxWidth[nIter] / 2;
				nCount++;
			}
		}
	}
	if (!nCount) return;

	// sort symbols, from left to right
	for (i = nCount; i > 1; i--)
	{
		// move the biggest element in [0: i-1] to the right, using bubble method
		for (j = 0; j < i - 1; j++)
		{
			if (nCenter[j] > nCenter[j+1])
			{
				HMC_SWAP(pnSymIndex[j], pnSymIndex[j+1]);
				HMC_SWAP(nCenter[j], nCenter[j+1]);
			}
		}
	}

	//============================================================
	// fill tree
	for (nIter = 0; nIter < nCount; ++nIter)
	{
		nTemp = pnSymIndex[nIter];
		pThis->nNodeCreated[nTemp] = 1;
		pThis->treeNode[nTemp].symbol = (unsigned short)(pThis->m_pRecResult[nTemp]);
		pThis->treeNode[nTemp].fullRecogResult = pCurPath->group[nTemp].uText;
		pThis->treeNode[nTemp].leftChild = -1;
		pThis->treeNode[nTemp].rightChild = -1;
		pThis->treeNode[nTemp].next = -1;
		pThis->treeNode[nTemp].nRefCount = 0;
	}

	// 寻找首符号，生成主节点
	nFirstSymFakeIndex = 0;
	nFirstSymRealIndex = pnSymIndex[0];
	pChiefNode = &pThis->treeNode[nFirstSymRealIndex];
	if (nRank == 0)
	{
		pThis->nRootNodeID = nFirstSymRealIndex;
		pChiefNode->nRefCount = 1;
	}
	else
	{
		// 非0级符号挂在nIndex符号节点之后
		if ((pThis->m_pRecResult[nIndex] == 0xff0d || pThis->m_pRecResult[nIndex] == 0x2014) && 
			pThis->m_ppnDomRelation[nIndex][nFirstSymRealIndex] == 1)
		{
			pThis->treeNode[nIndex].rightChild = nFirstSymRealIndex;
			++(pChiefNode->nRefCount);
		}
		if ((pThis->m_pRecResult[nIndex] == 0xff0d || pThis->m_pRecResult[nIndex] == 0x2014) && 
			pThis->m_ppnDomRelation[nIndex][nFirstSymRealIndex] == 2)
		{
			pThis->treeNode[nIndex].leftChild = nFirstSymRealIndex;
			++(pChiefNode->nRefCount);
		}
		if (pThis->m_pRecResult[nIndex] == 0x221A && 
			pThis->m_ppnDomRelation[nIndex][nFirstSymRealIndex] == 1)
		{
			pThis->treeNode[nIndex].leftChild = nFirstSymRealIndex;
			++(pChiefNode->nRefCount);
		}
	}

	//============================================================
	// 穿线成树
	for (nIter = 0; nIter + 1 < nCount; ++nIter)
	{
		nCurSymbol = pnSymIndex[nIter];
		nNextSymbol = pnSymIndex[nIter+1];
		pChiefNode = &pThis->treeNode[nCurSymbol];

		if (pThis->treeNode[nNextSymbol].nRefCount > 0)
		{
			continue;
		}

		if (pThis->m_ppnSpacialRelation[nCurSymbol][4] == nNextSymbol)      // 同一基线
		{
			pChiefNode->next = nNextSymbol;
			pThis->treeNode[nNextSymbol].nRefCount++;
		}
		else if (pThis->m_ppnSpacialRelation[nNextSymbol][5] == nCurSymbol)   // 幂指数
		{
			pChiefNode->rightChild = nNextSymbol;
			pThis->treeNode[nNextSymbol].nRefCount++;

			for (i = nIter + 2; i < nCount; i++)
			{
				nRightSymbol = pnSymIndex[i];
				if (pThis->m_ppnSpacialRelation[nCurSymbol][4] == nRightSymbol &&
					pThis->treeNode[nRightSymbol].nRefCount < 1)
				{
					pChiefNode->next = nRightSymbol;
					pThis->treeNode[nRightSymbol].nRefCount++;
				}
			}
		}
		else if (pThis->m_ppnSpacialRelation[nNextSymbol][7] == nCurSymbol)   //右下/左上   huyang
		{
			pChiefNode->rightChild = nNextSymbol;
			pThis->treeNode[nNextSymbol].nRefCount++;

			for (i = nIter + 2; i < nCount; i++)
			{
				nRightSymbol = pnSymIndex[i];
				if (pThis->m_ppnSpacialRelation[nCurSymbol][4] == nRightSymbol &&
					pThis->treeNode[nRightSymbol].nRefCount < 1)
				{
					pChiefNode->next = nRightSymbol;
					pThis->treeNode[nRightSymbol].nRefCount++;
				}
			}
		}
		else
		{
			

			pChiefNode->next = nNextSymbol;
			pThis->treeNode[nNextSymbol].nRefCount++;
		}
	}
}

static void SABuildTree(PSTRUCTANALYZERHCR pThis, PSPLITPATHHCR pCurPath)
// 构造三叉树
{
	int nMax = SAGetMaxValue2(pThis->m_pnRank, pThis->m_pnRank + pThis->m_nSymbolNum);
	int nRank, nIndex;

	SAAnalyseBasicEqn(0, -1, 1, pThis, pCurPath);	// 对0级符号进行基本表达式分析

	// 对1 -- nMax级符号进行基本表达式分析
	for (nRank = 1; nRank <= nMax; ++nRank)
	{
		// 对nRank - 1级的每个符号nIndex
		for (nIndex = 0; nIndex != pThis->m_nSymbolNum; ++nIndex)
		{
			if (pThis->m_pnRank[nIndex] == nRank - 1)
			{
				SAAnalyseBasicEqn(nRank, nIndex, 1, pThis, pCurPath);
				SAAnalyseBasicEqn(nRank, nIndex, 2, pThis, pCurPath);
			}// if (m_pnRank[nRow] == nRank)
		}// for (int nRow = 0; nRow < m_nSymbolNum; ++nRow)
	}// for (int nRank = 1; nRank < nMax; ++nRank)
}

// 分析表达式并生成三叉树的主过程
static short SAAnalyseAndGetTree(PSTRUCTANALYZERHCR pThis, PSPLITPATHHCR pCurPath)
{
	int nIter;

	if (!pThis) 
	{
		return 0;
	}
	if (!(pThis->m_ppnSpacialRelation) || !(pThis->m_pRecResult))		
	{
		return 0;
	}
	for (nIter = 0; nIter != pThis->m_nSymbolNum; ++nIter)
	{
		if (!(pThis->m_ppnSpacialRelation[nIter]))
		{
			return 0;
		}
	}
	SACatagorizeSymbols(pThis);
	SASetDominantRelations(pThis);
	SASetSymbolRanks2(pThis);
	SABuildTree(pThis, pCurPath);
	return pThis->nRootNodeID;
}

// 处理原则，尽量不丢弃符号
void RefineTree(PTRINARYTREEHCR TreeNodes, short dataHead)
{
	short i, baseLineCount;
	short baseLineHead[HC_MAX_GROUP];

	for (i = 0; i < HC_MAX_GROUP; ++i)
	{
		baseLineHead[i] = -1;
	}
	baseLineHead[0] = dataHead;
	baseLineCount = 1;
	assert(dataHead >= 0);

	for (i = 0; baseLineHead[i] >= 0 && i < HC_MAX_GROUP; ++i)
		// 处理第 i 个基线，并将新的基线存入数组 baseLineHead
	{
		short treeNodeInd;
		short head = baseLineHead[i];
		short k, baseLineLen = 0;
		short parenthStack[HC_MAX_GROUP];
		short top = 0;

		for (treeNodeInd = baseLineHead[i]; 
			treeNodeInd != -1; 
			treeNodeInd = TreeNodes[treeNodeInd].next)
		{
			if (baseLineCount == HC_MAX_GROUP)
				break;
			if (TreeNodes[treeNodeInd].leftChild >= 0)
			{
				baseLineHead[baseLineCount++] = TreeNodes[treeNodeInd].leftChild;
			}
			if (baseLineCount == HC_MAX_GROUP)
				break;
			if (TreeNodes[treeNodeInd].rightChild >= 0)
			{
				baseLineHead[baseLineCount++] = TreeNodes[treeNodeInd].rightChild;
			}
		}

		while (head >= 0)
		{
			if (TreeNodes[head].symbol == (TEXTCODE)('('))
			{
				parenthStack[top++] = head;
			}
			else if (TreeNodes[head].symbol == (TEXTCODE)(')'))
			{
				if (top == 0)	// empty stack
				{
					parenthStack[top++] = head;
				}
				else
				{
					--top;
				}
			}
			head = TreeNodes[head].next;
		}
		for (k = 0; k < top; ++k)
		{
			short l;
			for (l = 0; l < FP_CHNCAND_NUM; ++l)
			{
				TEXTCODE ch = TreeNodes[parenthStack[k]].fullRecogResult[l] ;
				if (ch >= '0' && ch <= '9')
				{
					TreeNodes[parenthStack[k]].symbol = (char)ch;
					break;
				}
			}
		}

	}
}

// 将三叉树dataHead转换为表达式strExpr
short TreeToExpr(STRUCTANALYZERHCR *pAnalyzer, const short nHeadNodeID, unsigned short* strExpr, int* score)
{
	unsigned short strSubExpr[BUF_SIZE] = {0};
	FILE *fp;
	short nStrLen, nSubStrLen;
	int i;
	PTRINARYTREEHCR dataHead;

	if (pAnalyzer->nNodeCreated[nHeadNodeID] < 0)
	{
		return 0;
	}
	dataHead = &pAnalyzer->treeNode[nHeadNodeID];

	for (i = 0; i < BUF_SIZE; ++i)
		strExpr[i] = 0;
	nStrLen = 0;
	nSubStrLen = 0;

	// 分数线
	if (dataHead->symbol == 0xff0d ||dataHead->symbol == 0x2215 && dataHead->leftChild >= 0 && dataHead->rightChild >= 0)
	{
		strSubExpr[0] = 0;
		nSubStrLen = 0;
		nSubStrLen = TreeToExpr(pAnalyzer, dataHead->leftChild, strSubExpr, score);	// 分子
	
		for (i = 0; i < nSubStrLen; i++)
		{
			strExpr[nStrLen++] = strSubExpr[i];
		}
		
		strExpr[nStrLen++] = 0x2215;   // ? 0xff15
		++*score;
		nSubStrLen = TreeToExpr(pAnalyzer, dataHead->rightChild, strSubExpr, score);	// 分母

		for (i = 0; i < nSubStrLen; i++)
		{
			strExpr[nStrLen++] = strSubExpr[i];
		}
	}
	// 根号
	else if (dataHead->symbol == 0x221a && dataHead->leftChild >= 0)
	{
		strSubExpr[0] = 0;
		nSubStrLen = 0;
		strExpr[nStrLen++] = 0xff53;//'s';
		strExpr[nStrLen++] = 0xff51;//'q';
		strExpr[nStrLen++] = 0xff52;//'r';
		strExpr[nStrLen++] = 0xff54;//'t';
		strExpr[nStrLen++] = 0xff08;
		++*score;
		nSubStrLen = TreeToExpr(pAnalyzer, dataHead->leftChild, strSubExpr, score);
		for (i = 0; i < nSubStrLen; i++)
		{
			strExpr[nStrLen++] = strSubExpr[i];
		}
		strExpr[nStrLen++] =0xff09;
	}
	
	else
	{
		//unicode2char(dataHead);
		strExpr[nStrLen++] = dataHead->symbol;

		// 幂指数
		if (dataHead->rightChild >= 0)
		{		
			strSubExpr[0] = 0;
			nSubStrLen = 0;
			bool UpperLeftFlag = 0;                               // huyang
			for (int i = 0; i < pAnalyzer->m_nSymbolNum; i++)         
			{
				if (pAnalyzer->m_ppnSpacialRelation[i][7] >= 0)
					UpperLeftFlag = 1;
			}
			if (UpperLeftFlag)
				strExpr[nStrLen++] = 0x005e;// '^';
			else
			strExpr[nStrLen++] = 0x005f;// '_';
			strExpr[nStrLen++] = 0xff08;
			nSubStrLen = TreeToExpr(pAnalyzer, dataHead->rightChild, strSubExpr, score);
			for (i = 0; i < nSubStrLen; i++)
			{
				strExpr[nStrLen++] = strSubExpr[i];
			}
			strExpr[nStrLen++] =0xff09;
		}
	}

	// 写入右侧符号
	if (dataHead->next >= 0)
	{
		strSubExpr[0] = 0;
		nSubStrLen = 0;
		nSubStrLen = TreeToExpr(pAnalyzer, dataHead->next, strSubExpr, score);
		++*score;
		for (i = 0; i < nSubStrLen; i++)
		{
			strExpr[nStrLen++] = strSubExpr[i];
		}
	}
	
	/* if((fp=fopen("C:\\yanghu\\Formula\\HC\\string.txt","w+"))==NULL) 
        {
            printf("cannot open");
        }
	fputs(strExpr,fp);*/
	//printf("the result is: ");
	//printf("%s",strExpr) ;
	//printf("\n");
	return nStrLen;
}

// get node number in one tree
static short GetTreeNodeNum(PSTRUCTANALYZERHCR psa)
{
	short i, nCurNodeID;
	short nTraverseNodeID[HC_MAX_GROUP * 2];
	short nTraverseNodeNum, nNewTraverseNodeNum;
	short *p1 = &nTraverseNodeID[0];
	short *p2 = &nTraverseNodeID[HC_MAX_GROUP];

	short nVisitedNodeFlag[HC_MAX_GROUP];
	short nVisitedNodeNum;

	short nSymbolNum = psa->m_nSymbolNum;
	PTRINARYTREEHCR pTreeNode = &psa->treeNode[0];

	if (psa->nRootNodeID < 0 || nSymbolNum <= 0)
	{
		return 0;
	}
	p1[0] = psa->nRootNodeID;
	nTraverseNodeNum = 1;

	for (i = 0; i < nSymbolNum; i++)
	{
		nVisitedNodeFlag[i] = 0;
	}

	while (nTraverseNodeNum > 0)
	{
		nNewTraverseNodeNum = 0;
		for (i = 0; i < nTraverseNodeNum; i++)
		{
			nCurNodeID = p1[i];
#ifdef HMC_DEBUG
			assert(nCurNodeID >= 0 && nCurNodeID < HC_MAX_GROUP);
#endif
			if (nCurNodeID >= 0 && nCurNodeID < HC_MAX_GROUP)
			{
				nVisitedNodeFlag[nCurNodeID] = 1;

				if (pTreeNode[nCurNodeID].leftChild >= 0)
				{
					p2[nNewTraverseNodeNum++] = pTreeNode[nCurNodeID].leftChild;
				}
				if (pTreeNode[nCurNodeID].rightChild >= 0)
				{
					p2[nNewTraverseNodeNum++] = pTreeNode[nCurNodeID].rightChild;
				}
				if (pTreeNode[nCurNodeID].next >= 0)
				{
					p2[nNewTraverseNodeNum++] = pTreeNode[nCurNodeID].next;
				}
			}
		}

		FPExchange(short*, p1, p2);
		nTraverseNodeNum = nNewTraverseNodeNum;
	}

	for (i = nVisitedNodeNum = 0; i < nSymbolNum; i++)
	{
		if (nVisitedNodeFlag[i])
		{
			nVisitedNodeNum++;
		}
	}

	return nVisitedNodeNum;
}

void hmcAnalysis(PSPLITPATHHCR pSplitPath, const short nCandPathNum, 
				 PSTROKEINFOHCR pStrokeInfo, const short nStrokeNum,
				 unsigned short uCandidateResult[])
{
	int nPointBuf[HC_MAX_GROUP * NUM_OF_RELATIONS];
	int *point[HC_MAX_GROUP];
	int i, j, k, m, num, nCount;
	short nGroupNum;
	TEXTCODE recresult[HC_MAX_GROUP];
	STRUCTANALYZERHCR structAnalyzer;
	short dataHead;
	unsigned short strExpr[FINAL_SP_CAND_NUM][BUF_SIZE];
	short nStrExprLen[FINAL_SP_CAND_NUM];
	char strTemp[BUF_SIZE] = {0};
	int anExprScores[FINAL_SP_CAND_NUM] = {0};
	unsigned short* pcTemp;
	TEXTCODE pptcRecoResult[HC_MAX_GROUP * FP_CHNCAND_NUM];
	TEXTCODE *pTmpText;
	PSPLITPATHHCR pCurPath;

	unsigned short *pCurResult = uCandidateResult;

	short nPathID, nGroupID, nStrokeID, nCandID;
	RECTHCR rcGroup[HC_MAX_GROUP], rcCurGroup;

	PGROUPINFOHCR pGroupInfo;
	short nFinalCandPathNum;

	short nTmp16;
	// if missed symbols exist in one tree, it must be incorrect
	short nCorrectTree[FINAL_SP_CAND_NUM] = {0,};
	short nHasCorrectTree = 0;
	pCurPath = pSplitPath;
	for (int GroupNum = 0;  GroupNum < pCurPath->nGroupNum; GroupNum++)	
		if (pCurPath->group[GroupNum].uText[0] == 0x0000 )             // 0x0000
			{
				pCurPath->group[GroupNum].uText[0] = pCurPath->group[GroupNum].uText[1];
			}
	nCount = 1;
	for (i = 0; i < FINAL_SP_CAND_NUM; ++i)
	{
		for (j = 0; j < BUF_SIZE; ++j)
		{
			strExpr[i][j] = 0;
		}
		nStrExprLen[i] = 0;
	}

	for (nPathID = 0, pCurPath = pSplitPath; nPathID < nCandPathNum; nPathID++, pCurPath++)
	{
		nGroupNum = pCurPath->nGroupNum;

		if (nGroupNum <= 0 || nGroupNum > HC_MAX_GROUP)
		{
			uCandidateResult[0] = 0;
			return;
		}

		pTmpText = pptcRecoResult;
		for (nGroupID = 0; nGroupID < nGroupNum; nGroupID++)
		{
			for (nCandID = 0; nCandID < pCurPath->group[nGroupID].nCandCount; nCandID++)
			{
				pTmpText[nCandID] = pCurPath->group[nGroupID].uText[nCandID];
			}
			pTmpText += FP_CHNCAND_NUM;
		}

		point[0] = &nPointBuf[0];

		for (nGroupID = 1; nGroupID < nGroupNum; nGroupID++)
			point[nGroupID] = point[nGroupID-1] + NUM_OF_RELATIONS;
		for (nGroupID = 0; nGroupID < nGroupNum; nGroupID++)
		{
			for (num = 0; num < NUM_OF_RELATIONS; num++)
			{
				point[nGroupID][num] = -1;
			}
		}

		// group rectangles
		for (nGroupID = 0, pGroupInfo = &pCurPath->group[0]; 
			nGroupID < nGroupNum; nGroupID++, pGroupInfo++)
		{
			nStrokeID = pGroupInfo->nStrokeBe;
			rcCurGroup = pStrokeInfo[nStrokeID].rect;
			nStrokeID++;
			while (nStrokeID <= pGroupInfo->nStrokeEn)
			{
				Union2(&pStrokeInfo[nStrokeID].rect, &rcCurGroup);
				nStrokeID++;
			}
			rcGroup[nGroupID] = rcCurGroup;

			// 小数点
			if (nGroupID > 0 && nGroupID < nGroupNum - 1
				&& RECT_WIDTH(&rcGroup[nGroupID]) < max(RECT_HEIGHT(&rcGroup[nGroupID - 1]), 
				RECT_WIDTH(&rcGroup[nGroupID - 1]))
				&& RECT_HEIGHT(&rcGroup[nGroupID]) < 0.4f * max(RECT_HEIGHT(&rcGroup[nGroupID - 1]), 
				RECT_WIDTH(&rcGroup[nGroupID - 1]))
				&& rcGroup[nGroupID].nTop > pCurPath->group[nGroupID - 1].nGravY
				- 0.1 * RECT_HEIGHT(&rcGroup[nGroupID - 1])
				&& pCurPath->group[nGroupID - 1].uText[0] != 0x221A
				&& pCurPath->group[nGroupID - 1].uText[0] != '('
				&& pCurPath->group[nGroupID - 1].uText[0] != ')'
				&& pCurPath->group[nGroupID - 1].uText[0] != '-'
				&& pCurPath->group[nGroupID + 1].uText[0] != '('
				&& pCurPath->group[nGroupID + 1].uText[0] != ')'
				&& pCurPath->group[nGroupID + 1].uText[0] != '-'
				&& RECT_WIDTH(&rcGroup[nGroupID - 1]) < 2.5 * RECT_HEIGHT(&rcGroup[nGroupID - 1])
				&& RECT_WIDTH(&rcGroup[nGroupID]) < 5 * RECT_HEIGHT(&rcGroup[nGroupID])
				&& !(pCurPath->group[nGroupID].uText[0] >= '2' && pCurPath->group[nGroupID].uText[0] <= '9')
				&& pCurPath->group[nGroupID].nStrokeEn == pCurPath->group[nGroupID].nStrokeBe
				&& pCurPath->group[nGroupID].uText[0] != '0'
				&& rcGroup[nGroupID].nBottom > pCurPath->group[nGroupID - 1].nGravY
				+ 0.2 * RECT_HEIGHT(&rcGroup[nGroupID - 1])
				)
			{
				pCurPath->group[nGroupID].uText[0] = (TEXTCODE)('.');
				pptcRecoResult[nGroupID * FP_CHNCAND_NUM] = (TEXTCODE)('.');
			}
			else if (nGroupID > 0 && nGroupID < nGroupNum - 1
				&& RECT_HEIGHT(rcGroup + nGroupID) < 3
				&& RECT_WIDTH(rcGroup + nGroupID) < 3)
			{
				pCurPath->group[nGroupID].uText[0] = (TEXTCODE)('.');
				pptcRecoResult[nGroupID * FP_CHNCAND_NUM] = (TEXTCODE)('.');
			}

			// '8' 误认为是 '5'
			/*if (pCurPath->group[nGroupID].uText[0] == '5'
				&& pCurPath->group[nGroupID].uText[1] == '8'
				&& pCurPath->group[nGroupID].nStrokeBe == pCurPath->group[nGroupID].nStrokeEn
				)
			{
				FPExchange(TEXTCODE, pCurPath->group[nGroupID].uText[0], pCurPath->group[nGroupID].uText[1]);
			}*/

			// 横线
			if (RECT_WIDTH(&rcGroup[nGroupID]) > 5 * RECT_HEIGHT(&rcGroup[nGroupID])
				&& pCurPath->group[nGroupID].uText[0] == 0x221A
				&& pCurPath->group[nGroupID].uText[1] == '-')
			{
				FPExchange(TEXTCODE, 
					pCurPath->group[nGroupID].uText[0],
					pCurPath->group[nGroupID].uText[1]);
				FPExchange(TEXTCODE, 
					pptcRecoResult[nGroupID * FP_CHNCAND_NUM],
					pptcRecoResult[nGroupID * FP_CHNCAND_NUM + 1]);
			}
		}

		JudgePositionRelations(nGroupNum, 
			pptcRecoResult, point, rcGroup);

		// 目前的结构分析基于首选字符
		for (i = 0; i < nGroupNum; ++i)
		{
			recresult[i] = pptcRecoResult[i*FP_CHNCAND_NUM];
		}

		// initialize struct analyzer
		structAnalyzer.m_nSymbolNum = nGroupNum;
		structAnalyzer.m_ppnSpacialRelation = point;
		structAnalyzer.m_pRecResult = recresult;
		structAnalyzer.m_nMaxInRank = 0;
		structAnalyzer.nRootNodeID = -1;

		for (i = 0; i < nGroupNum; ++i)
		{
			structAnalyzer.m_pnType[i] = 0;
			structAnalyzer.m_pnRank[i] = 0;
			structAnalyzer.m_pnBoxWidth[i] = (int)RECT_WIDTH(&rcGroup[i]);
			structAnalyzer.m_pnBoxLeft[i] = (int)rcGroup[i].nLeft;
			for (j = 0; j < nGroupNum; j++)
			{
				structAnalyzer.m_ppnDomRelation[i][j] = 0;
			}
			structAnalyzer.nNodeCreated[i] = 0;
		}

		// 构造树
		dataHead = SAAnalyseAndGetTree(&structAnalyzer, pCurPath);

		// added by Yang Hu on 2013-10-03
		// 将表达式树进行一些处理，包括符号纠正、括号匹配等
		RefineTree(structAnalyzer.treeNode, dataHead);

		// check tree node number = group number ?
		if (!nHasCorrectTree)
		{
			nTmp16 = GetTreeNodeNum(&structAnalyzer);
			if (nTmp16 == pCurPath->nGroupNum)
			{
				nCorrectTree[nPathID] = 1;
				nHasCorrectTree = 1;
			}
		}

	
		nStrExprLen[nCount-1] = TreeToExpr(&structAnalyzer, dataHead, strExpr[nCount-1],
			&(anExprScores[nCount - 1]));
		anExprScores[nCount - 1] *= 1024;


		// 对表达式的合理性进行简单修正.
		for (i = 0; i < nStrExprLen[nCount-1]; ++i)
		{
			unsigned short *pcExpr = strExpr[nCount-1];
			nTmp16 = 0;
			
			switch (pcExpr[i])
			{
			case 0xff08:                      // 将（修正为c
				bool ExistRightBracket = false;
				for (j = 0; j < nStrExprLen[nCount-1]; ++j)
					if ( pcExpr[j] == 0xff09)
						ExistRightBracket = true;
				if (!ExistRightBracket)
					pcExpr[i] = 0xff43;
				break;
			}
		}
		

		++nCount;
	}
	if (nCandPathNum > 1 && !nCorrectTree[0] && nHasCorrectTree)
	{
		for (i = 1; i < nCandPathNum; i++)
		{
			if (nCorrectTree[i])
			{
				// HMC_SWAP these 2 results
				short nStrLen1, nStrLen2;
				unsigned short *pStr1, *pStr2;
				if (nStrExprLen[0] < nStrExprLen[i])
				{
					nStrLen1 = nStrExprLen[0];
					nStrLen2 = nStrExprLen[i];
					pStr1 = &strExpr[0][0];
					pStr2 = &strExpr[i][0];
				}
				else
				{
					nStrLen1 = nStrExprLen[i];
					nStrLen2 = nStrExprLen[0];
					pStr1 = &strExpr[i][0];
					pStr2 = &strExpr[0][0];
				}
				for (j = 0; j < nStrLen1; j++)
				{
					HMC_SWAP(pStr1[j], pStr2[j]);
				}
				for (; j < nStrLen2; j++)
				{
					pStr1[j] = pStr2[j];
				}
				HMC_SWAP(nStrExprLen[0], nStrExprLen[i]);
				break;
			}
		}
	}

	nFinalCandPathNum = nCandPathNum;


	// 对候选结果字符串重排顺序
	for (i = 0; i < nFinalCandPathNum; ++i)
	{
		for (j = i + 1; j < nFinalCandPathNum; ++j)
		{
			if (nStrExprLen[i] == nStrExprLen[j] && nStrExprLen[i] > 0)
			{
				for (k = 0; k < nStrExprLen[i]; k++)
				{
					if (strExpr[i][k] != strExpr[j][k])
					{
						break;
					}
				}
				if (k == nStrExprLen[i])
				{
					// equal strings
					for (k = j; k+1 < nCandPathNum; ++k)
					{
						//strcpy(strExpr[k], strExpr[k + 1]);
						for (m = 0; m < nStrExprLen[k+1]; m++)
						{
							strExpr[k][m] = strExpr[k+1][m];
						}
						nStrExprLen[k] = nStrExprLen[k+1];
					}
					--j;
					if (nFinalCandPathNum > 0)
					{
						
						nFinalCandPathNum--;
					}
				}
			}
			/*else if (nStrExprLen[j] >= 2 * nStrExprLen[i] && nStrExprLen[i] < 10 &&
				nStrExprLen[i] > 3)
			{
				for (k = 0; k < FINAL_SP_CAND_NUM; ++k)
				{
					FPExchange(char, strExpr[i][k], strExpr[j][k]);
					FPExchange(int, nStrExprLen[i], nStrExprLen[j]);
				}
			}*/
		}
	}

	for (j = 0; j < nFinalCandPathNum; ++j)
	{
		if (nStrExprLen[j] > 0)
		{
			for (i = 0; i < nStrExprLen[j]; i++)
			{
				*pCurResult++ = (unsigned short)strExpr[j][i];
			}
			*pCurResult++ = 0xffff;
		}
	}
	*pCurResult = 0;
}
