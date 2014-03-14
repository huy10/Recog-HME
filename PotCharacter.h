/*====================================/
/				Lily Wic
/	
/				20120718
/
/====================================*/
#include<vector>
#include<string>
#include<cmath>
#include<iostream>
using namespace std;
#define POTX 
#ifdef POTX
int openPotx(FILE *fo,unsigned short PointData[],char* code,unsigned long * size,int iterator);
#else
int openPot(FILE *fo,unsigned char PointData[],char* code);
#endif
//这是Pot样本，即Pot文件中一个字的数据
class PotSample
{
public:
	struct PointF;
	struct point
	{
		unsigned char x,y;

		void ReadIn(unsigned char* xPos){x = xPos[0];y = xPos[1];}
		bool IsStrokeEnd(){return x==0xFF && y==0x0;};
		bool IsSampleEnd(){return x==0xFF && y==0xFF;};

		unsigned short ToWord(){return 256*y+x;};/*读入点所存储的两字节数据，注意大小端*/
		PointF ToFloat(){return PointF(x,y);};
	};
	/*以便运算*/
	struct PointF
	{
		float x,y;
		
		PointF(){x=0,y=0;}
		PointF(float nx,float ny){x=nx,y=ny;}
		PointF(point p){x=p.x,y=p.y;}

		/*返回以此点为原点的坐标系中B点的坐标*/
		PointF Difference(PointF b){b.x-=x;b.y-=y;return b;}
		/*返回此点到原点的距离*/
		float Norm(){return sqrt(x*x+y*y);}
		/*返回此点与b点的直线的夹角余弦值*/
		float Angle(PointF b){return (b.x*x+b.y*y)/Norm()/b.Norm();}
		/*返回此点辐线的角度，(-180,180]*/
		float Angle(){return atan2(y,x)/3.1415926*180;}
		
	};
	typedef vector<point> stroke; /*样本中的每一个笔画*/
	vector<stroke> strokes;

	unsigned int PatternLength;/*模板的长度*/
	char InnerCode[3];/*所存汉字的内码*/
	unsigned int NumberOfStroke;/*文件中所存储的笔画数*/

	int ReadIn(FILE* fo);/*从文件中读出一个样本*/
	PotSample(){InnerCode[2]=0;};
};

//这是Pot字符，即一个Pot文件所含的数据
class PotCharacter
{
public:
	vector<PotSample> samples;

	/*从文件中读入 联机手写单字样本的存储格式*/
	/*返回中的样本个数*/
	int FromFile(string fileName); 
	PotCharacter(string fileName){FromFile(fileName);};

};
#ifndef POTX
/*用来分析对比不同字的*/
class StrokeAnalyzer
{

	vector<string> strokeType;
	/*判断两个方向是否一致的阈值*/
	float directionThreshold;
	/*判断长度上多小的差异就不再考虑的阈值*/
	float lengthThreshold;
public:
	StrokeAnalyzer()
	{
		strokeType.push_back("横");
		strokeType.push_back("竖");
		strokeType.push_back("撇");
		strokeType.push_back("捺");
		strokeType.push_back("拐");
		strokeType.push_back("点");/*点就是长度不够长的笔画*//*不要给我来草书*/

		directionThreshold = 0.3;
		lengthThreshold = 20;
	}
	/*返回笔画序号对应的名称*/
	//string StrokeToString(int n){return strokeType[n];}
	/*返回此点对应的角度字符*/
	//char RecognizeDirection(PotSample::PointF p);
	/*查看两个笔划衔接间的位置关系是否一致*/
	//bool CheckRelativePosition(PotSample::PointF A1,/*第一个字的前一笔画的最后一点*/
		//PotSample::PointF A2,/*第一个字的后一笔画的第一点*/
		//PotSample::PointF B1,/*……*/
		//PotSample::PointF B2
		//);
	/*返回输入的笔画序号，种类见strokeType常量*/
	//int RecognizeStrokeType(PotSample::stroke s);
	/*int DirectionToIndex(char currentDirection)
	{
		if(currentDirection == '-')
			return 0;
		else if(currentDirection == '|')
			return 1;
		else if(currentDirection == '/')
			return 2;
		else if(currentDirection == '\\')
			return 3;
		else if(currentDirection == 'o')
			return 5;
		throw "Invalid Direction character.";
	}*/
};
#endif
