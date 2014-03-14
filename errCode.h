#ifndef __ERROR_CODE_H__
#define __ERROR_CODE_H__

/*
 *	The following micro definition is for debug use or for special customer
 */
/*
#define SINGAPORE
#define __FOR_DEBUG__
#define __PC_EXECUTE__
*/

/*
 *	Define the Error code						
 */
#define WTSuccess					0x0000			/*成功*/
#define WTFail						0x0001			/*失败*/
#define GeneralLibFileNotFound		0x0002			/*通用库未找到*/
#define UsrLibFileNotFound			0x0003			/*用户库未找到*/
#define CanNotSetInnerCodeType		0x0004			/*不能进行输出内码的设置*/
#define InnerCodeTypeNotSupported	0x0005			/*当前库不支持该内码类型*/
#define UsrLibNotValid				0x0006			/*用户库格式不对*/
#define InvalidEndianType			0x0007			/*库的Endian类型不正确*/
#define NotSupportedFunction1		0x0008			/*未支持的功能1 */
#define NoLookupTableSection		0x0014			/*库中没有LookupTableSection*/
#define NoModelInfoSection			0x0015			/*库中没有ModelInfoSection*/
#define NoAntiForgeSection			0x0016			/*库中没有AntiForgeSection*/
#define NoInformationSection		0x0019			/*库中没有InformationSection*/
#define TraSimLibErr        		0x0020			/*繁简映射库文件不存在或在库文件中没找到繁简映射表*/
#define InvalidUsrModelNum     		0x0021			/*用户模板数过大或过小，应为1－4096*/
#define NoFreeSpaceForDefModel      0x0022          /*自定义模板空间已满，不能再添加*/
#endif //__ERROR_CODE_H__

