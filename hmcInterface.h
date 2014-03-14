
#ifndef __HMC_INTERFACE_H__
#define __HMC_INTERFACE_H__

#ifdef __cplusplus
#define HMC_API extern "C"
#else //__cplusplus
#define HMC_API
#endif

/******************************************************************
* Definition:	return value
******************************************************************/
// success
#define HMCERROR_SUCCESS				0
// default fail
#define HMCERROR_FAIL					1
// specified address is null
#define HMCERROR_NULL_ADDRESS			2
// no database loaded
#define HMCERROR_NO_DB					3
// template db is error
#define HMCERROR_ERROR_DB				4
// too many points
#define HMCERROR_TOO_MANY_POINTS		5
// error points
#define HMCERROR_ERROR_POINTS			6
// error candidate number
#define HMCERROR_NO_CAND_NUM			7
// too big writing area
#define HMCERROR_TOO_BIG_RECT			8
// too small writing area
#define HMCERROR_TOO_SMALL_RECT			9

/******************************************************************
* Function: HmcInit
* Description: load database
* Call:
* Call By:
* General Variables:
* Input & Output:
*	->pDatabase:	database buffer
*	->pGlobalRam:	global ram
* Return:
*	0	success
*	else	fail
******************************************************************/
HMC_API short HmcInit(char *pDatabase,
					  char *pGlobalRam);

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
******************************************************************/
HMC_API short HmcSetRect(unsigned short uWidth,
						 unsigned short uHeight,
						 char *pGlobalRam);

HMC_API short HmcRecog(unsigned short uPointData[],
					   unsigned short uPointNum,
					   unsigned short uCandidateNum,
					   unsigned short uEndInput,
					   char *pGlobalRam,
					   unsigned  short uCandidateResult[]);

HMC_API char * HmcFinal(unsigned short uCandidateResult[]);
#endif // __HMC_INTERFACE_H__
