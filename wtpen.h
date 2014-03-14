/******************************************************************

******************************************************************/

#ifndef __WTPEN_HEAD_H
#define __WTPEN_HEAD_H

/*
 * Variable type definition 
 */
typedef short	WTError;
typedef long	WT_int32;	/*定义一个四字节的变量*/

/*
 * Define Macros
 */
#define WTPEN_abs(value)	  (((value)<0)?(-(value)):(value))
/*
 * Declaration of functioins				
 */
#ifdef __cplusplus
extern "C" {
#endif

/*
 *	功能：初始化。该函数在启动识别系统时调用一次即可						
 *	输入参数：																			
 *		RamAddress:															
 *			由主调函数开辟的空间，至少4K									
 *		RamSize:															
 *			RamAddress所指向的空间的大小									
 *		LibStartAddress:													
 *			库在内存中存放的起始位置	
 *      left:
 *          手写区域左上角的X坐标
 *      top:
 *          手写区域左上角的Y坐标
 *      right:
 *          手写区域右下角的X坐标
 *      bottom:
 *          手写区域右下角的Y坐标
 *	返回值：																
 *		0--表示成功
 *		其它值--表示出错
 */
WTError WTRecognizeInitEx(char *RamAddress,WT_int32 RamSize,char *LibStartAddress
						  ,short left, short top, short right, short bottom);


/*
 *	功能：设置识别范围，函数参数的对应位设置为1时，设别范围将包括对应字符集，
 *		设置为0时，对应字符集将不在识别范围内。另外，设置能否起作用还取决于设
 *		别库是否支持对应字符集。
 *	输入参数：
 *		Range:按照bit位是否置为1设定识别范围，由于采用的不同的语言识别词典库，所以该范围设置也是确定的
 *      具体设置请参见每一个版本对应的用户手册
 *		RamAddress:
 *			同WTRecognizeInitEx()中的RamAddress
 *	返回值:
 *		0--表示成功
 *		其它值--表示出错
 */
WTError WTSetRange(WT_int32 Range,char *RamAddress);

/*
 *	功能：设置识别速度 (Limited function)
 *	输入参数：
 *		Speed:
 *			0-10, 0最慢,10最快，缺省为5
 *		RamAddress:
 *			同WTRecognizeInitEx()中的RamAddress
 *	返回值:
 *		0--表示成功
 *		其它值--表示出错
 */
WTError WTSetSpeed(short Speed,char *RamAddress);
/*
*	功能: 如果手写区域长边或宽边大于254时，可能会存在坐标大于等于255的点，则可以调用本函数, 将坐标等比例缩小到0-255之间(不包括255)。
*	输入参数:
*   	PointData:
*           轨迹点数据，依次为(X1,Y1,X2,Y2,...0xff,0x00,Xn,Yn,Xn+1,Yn+1,
*			...0xffff,0x0000,0xffff,0xffff),其中的点对(0xffff,0x0000)表示一个笔划结束，点对(0xffff,0xffff)
*           表示该字结束.每4个字节存放一对数据，即x,y坐标分别都是unsigned short型;                  
*	    NumberOfPoint:
*           轨迹点的点数
*		RamAddress:
*			同WTRecognizeInitEx中的RamAddress
*	输出参数:
*   	PointData:
            轨迹点数据，依次为(X1,Y1,X2,Y2,...0xff,0x00,Xn,Yn,Xn+1,Yn+1,
*			...0xff,0x00,0xff,0xff),其中的点对(0xff,0x00)表示一个笔划结束，点对(0xff,0xff)
*            表示该字结束.每2个字节存放一对数据，即x,y坐标分别都是unsigned char型，因为已经将轨迹点坐标转换到0-255之间;
*	返回值:
*		0--表示成功
*		其它值--表示出错 
*	其他: 调用者须保证PointData数据格式和点数的准确性，本函数不进行数据正确性校验
*/
WTError WTNormData(unsigned short *PointData,short NumberOfPoint, char *RamAddress);

WTError WTNormDataEx2(unsigned short *PointData,short NumberOfPoint, char *RamAddress);


/*
 *	功能：必要的清除工作。该函数在退出识别系统时调用一次即可
 *	输入参数:
 *		RamAddress:
 *			同WTRecognizeInitEx()中的RamAddress
 *	返回值:
 *		0--表示成功
 *		其它值--表示出错
 */									
WTError WTRecognizeEnd(char *RamAddress);


/************************************************************************/
/*  名称：WTCorrectByPhrase(char *RamAdd,unsigned short *Candidate, unsigned short *CandidateDistance,char CandidateNum, char CharacterNum)
	功能：用词语来修正识别结果
	输入参数：Candidate　--候选结果; CandidateDistance　--候选结果对应的距离;
	CandidateNum　--每个单字的候选字数;CharacterNum　--包括的单字数;
	RamAdd --用来进行数据暂时存放使用，大小为2*sizeof(char)*CharacterNum　+sizeof(unsigned short)*(CharacterNum+CandidateNum*CharacterNum　)+函数WTIsPhrase()所需内存;
	输出参数：Candidate　--修正后的结果，词与词之间的间隔用0x0000隔开。
	Return：0--表示无修改结果；其它值-修正结果的有效字节数（最大值不大于CandidateNum* CharacterNum,并且总值不能超过16000）
	应用范围：有T9和无T9                                                                    */
/************************************************************************/     
//unsigned short WTCorrectByPhrase(char *RamAddForMC,unsigned short Candidate[], unsigned short CandidateDistance[],char CandidateNum, char CharacterNum);

/************************************************************************/
/*  名称：WTSetFullScreen(short nFullTag, char *RamAddress)
	功能：用词语来修正识别结果
	输入参数：nFullTag　--全屏输入标志, 1为全屏,0为非全屏。全屏即输入区域的大小超过了254*254
	Return：Null
    应用范围：全屏和非全屏模式设置                                       */
/************************************************************************/ 
void WTSetFullScreen(short nFullTag, char *RamAddress);

/************************************************************************/
/*  名称：WTSetWriteArea(unsigned short unLeft,//输入区域左上角x坐标
*	unsigned short unTop,
*	unsigned short unRight,//右下角x坐标
*	unsigned short unBottom,
*	unsigned short unBase,//输入英文单词时使用的基线y方向坐标
*	unsigned short unMid,//输入英文单词时使用的限制中间字母大小的y方向坐标，与基线平行
*	char *pGlobalRam)
	功能：设置文字区域大小
	输入参数：
	应用范围：*/
/************************************************************************/ 
WTError WTSetWriteArea(unsigned short unLeft,//输入区域左上角x坐标
					   unsigned short unTop,
					   unsigned short unRight,//右下角x坐标
					   unsigned short unBottom,
					   unsigned short unBase,//输入英文单词时使用的基线y方向坐标
					   unsigned short unMid,//输入英文单词时使用的限制中间字母大小的y方向坐标，与基线平行
					   char *pGlobalRam);
//for CAL
void Norm(unsigned char *PointData,short NumberOfPoint,char *RamAddress);
WTError WTRecognizeEx(unsigned char * PointData,short PointsNumber,short NumberOfCandidate,unsigned short *CandidateResult,unsigned short *CandidateDistance,char *RamAddress);
/*
*	功能：进行识别(扩展)。该函数每次对输入的轨迹进行识别，结果放在CandidateResult中
*	参数：
*		PointBuf：
*			输入的轨迹点，依次为(X1,Y1,X2,Y2,...0xff,0x00,Xn,Yn,Xn+1,Yn+1,
*			...0xff,0x00,0xff,0xff),其中的点对(0xff,0x00)表示一个笔划结束，
*			点对(0xff,0xff)表示该字结束
*		PointsNumber:
*			总共轨迹点的点数
*		NumberOfCandidate:
*			要得到的候选字数目(1~30)
*		CandidateResult:
*			存放识别结果,由主调函数开辟，至少应有2*NumberOfCandidate个字节
*		CandidateDistance:
*			各个候选字的识别距离
*		RamAddress:
*			相当于WTRecognizeInit中的RamAddress
*	返回值:
*		0--表示成功
*		其它值--表示出错
*/
WTError WTRecognizeInit(char *pRamAddress,WT_int32 RamSize,char *pLibStartAddress);
/*
*	功能：初始化。该函数在启动识别系统时调用一次即可						
*	参数：																			
*		pRamAddress:															
*			由主调函数开辟的空间，至少2K									
*		RamSize:															
*			pRamAddress所指向的空间的大小									
*		pLibStartAddress:													
*			库在内存中存放的起始位置										
*	返回值：																
*		0--表示成功
*		其它值--表示出错
*/
					   
void ExtractFeature(unsigned char *NormPoint,short nNumberOfPoint,char *RamAddress);
					   


#ifdef __cplusplus
}
#endif

#endif
