#pragma once
#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>
#include <sys/stat.h>
#include"ServerFrame.h"

#define OK 1
#define ERROR 0
#define ACKNULL 0x00			//应答信息为空
#define UserNameLength 16		//用户名长度限制
#define FileNameLength 36		//文件名长度限制
#define FrameFileLength 1000	//文件包内文件信息长度
/*准许上传文件的应答类型*/
enum RplyAllowUpload
{
	UploadOk = 0x00,	//允许上传
	UploadRefuse = 0x01	//拒绝上传
};
/*准许接受文件的应答类型*/
enum RplyAllowAccept
{
	AcceptOk = 0x00,	//允许接收
	AcceptRefuse = 0x01	//拒绝接收
};
/***************************************************************************
结构体名称：FrameHead
功      能：帧头格式
成员及描述：type:帧类型,ack:应答类型或0x00或特殊用途,length:帧长度
说      明：
***************************************************************************/
typedef struct
{
	uint8_t type;
	uint8_t ack;
	uint16_t length;
}FrameHead;
/*****************文件操作中S->C的所有帧类型*******************/
/***************************************************************************
结构体名称：S2CFileHeadRply
功      能：文件信息帧1应答帧
成员及描述：FH:帧头
说      明：C1向S端发送的文本信息帧1的应答帧
			C1向S表示上传文件的请求时，S对当前是否能上传文件做出的应答
***************************************************************************/
typedef struct
{
	FrameHead FH;
}FrameS2CFileHeadRply;
/***************************************************************************
结构体名称：FrameS2CFileBodyRply
功      能：文件信息帧2应答帧
成员及描述：FH:帧头,FrameOrder:帧的序号
说      明：C1向S端发送的文件信息帧2，S向C1发送的应答帧以表示确认收到
***************************************************************************/
typedef struct
{
	FrameHead FH;
	uint32_t FrameOrder;
}FrameS2CFileBodyRply;
/***************************************************************************
结构体名称：FrameS2CFileInform
功      能：传输文件消息帧
成员及描述：FH:帧头,SendUserName:发送者用户名,FileSize:文件大小,PackNum:文件包的数量,FileName:文件名
说      明：C1向S文件上传成功后，S向C2发送的帧，用来通知C2这是谁传送来的文件，文件名是什么，文件大小多少
***************************************************************************/
typedef struct
{
	FrameHead FH;
	char SendUserName[UserNameLength];
	uint32_t FileSize;
	uint32_t PackNum;
	char FileName[FileNameLength];
}FrameS2CFileInform;
/***************************************************************************
结构体名称：FrameS2CFileHead
功      能：文件信息帧1(传送文件名等文件头信息)
成员及描述：FH:帧头,SendUserName:发送者用户名,FileSize:文件大小,PackNum:文件包的数量,FileName:文件名
说      明：C2向S发送下载指令后，S向C2发送的信息头
***************************************************************************/
typedef struct
{
	FrameHead FH;
	char SendUserName[UserNameLength];
	uint32_t FileSize;
	uint32_t PackNum;
	char FileName[FileNameLength];
}FrameS2CFileHead;
/***************************************************************************
结构体名称：FrameS2CFileBody
功      能：文件信息帧2(传送文件内容)
成员及描述：FH:帧头,SendUserName:发送者用户名,OrderNum:文件包序列号,FileCont:文件内容
说      明：将文件拆开形成多个文件内容包
***************************************************************************/
typedef struct
{
	FrameHead FH;
	char SendUserName[UserNameLength];
	uint32_t OrderNum;
	char FileCont[FrameFileLength];
}FrameS2CFileBody;
/*****************文件操作中C->S的所有帧类型*******************/
/***************************************************************************
结构体名称：FrameC2SFileHead
功      能：文件信息帧1(传送文件名等文件头信息)
成员及描述：FH:帧头,AcceptUserName:接受者用户名,FileSize:文件大小,PackNum:文件包数量,FileName:文件名
说      明：C1向S发送的文件传输请求
***************************************************************************/
typedef struct
{
	FrameHead FH;
	char AcceptUserName[UserNameLength];
	uint32_t FileSize;
	uint32_t PackNum;
	char FileName[FileNameLength];
}FrameC2SFileHead;
/***************************************************************************
结构体名称：FrameC2SFileBody
功      能：文件信息帧2(传送文件内容)
成员及描述：FH:帧头,AcceptUserName:接受者用户名,OrderNum:文件包序列号,FileCont:文件内容
说      明：C1向S发送文件内容
***************************************************************************/
typedef struct
{
	FrameHead FH;
	char AcceptUserName[UserNameLength];
	uint32_t OrderNum;
	char FileCont[FrameFileLength];
}FrameC2SFileBody;
/***************************************************************************
结构体名称：FrameC2SFileDownload
功      能：下载通知帧(客户端请求下载)
成员及描述：FH:帧头,发送者用户名，文件名
说      明：C2向S发送的下载文件请求帧
***************************************************************************/
typedef struct
{
	FrameHead FH;
	char SendUserName[UserNameLength];
	char FileName[FileNameLength];
}FrameC2SFileDownload;
/***************************************************************************
结构体名称：FrameC2SFileHeadReply
功      能：文件信息帧1应答帧
成员及描述：FH:帧头
说      明：S向C2发送文件头信息，C2发送的应答帧
***************************************************************************/
typedef struct
{
	FrameHead FH;
}FrameC2SFileHeadReply;
/***************************************************************************
结构体名称：FrameC2SFileBodyReply
功      能：文件信息帧2应答帧
成员及描述：FH:帧头,FrameOrder:文件包序号
说      明：C2对于文件包缺少情况的信息头
***************************************************************************/
typedef struct
{
	FrameHead FH;
	uint32_t FrameOrder;
}FrameC2SFileBodyReply;
/*****************文件操作中S->C的生成帧函数*******************/
/***************************************************************************
函数名称：CrtFrameFileHeadRply
功    能：生成文件信息帧1应答帧
输入参数：文件信息帧1应答帧引用,应答类型
返 回 值：帧长度
说    明：函数外部定义一个帧的指针作为参数，在函数内申请空间
***************************************************************************/
int CrtFrameFileHeadRply(FrameS2CFileHeadRply*&p_Frame, RplyAllowUpload RplyType);
/***************************************************************************
函数名称：CrtFrameFileBodyReply
功    能：生成文件信息帧2应答帧
输入参数：文件信息帧2应答帧引用,序号
返 回 值：帧长度
说    明：函数外部定义一个帧的指针作为参数，在函数内申请空间
***************************************************************************/
int CrtFrameFileBodyReply(FrameS2CFileBodyRply*&p_Frame, uint32_t FrameOrder);
/***************************************************************************
函数名称：CrtFrameFileInform
功    能：生成传输文件消息帧
输入参数：帧引用,_SendUserName:用户名,_FileSize:文件大小,_PackNum:包的数量,_FileName:文件名,ChkAll:群发判断
返 回 值：帧长度
说    明：函数外部定义一个帧的指针作为参数，在函数内申请空间
***************************************************************************/
int CrtFrameFileInform(FrameS2CFileInform*&p_Frame, char* _SendUserName, uint32_t _FileSize, char* _FileName, SglOrAll ChkAll);
/***************************************************************************
函数名称：CrtFrameFileHead
功    能：生成文件信息帧1
输入参数：帧引用,_SendUserName:用户名,_FileSize:文件大小,_PackNum:包的数量,_FileName:文件名
返 回 值：帧长度
说    明：函数外部定义一个帧的指针作为参数，在函数内申请空间
***************************************************************************/
int CrtFrameFileHead(FrameS2CFileHead*&p_Frame, char* _SendUserName, uint32_t _FileSize, char* _FileName);
/***************************************************************************
函数名称：CrtFrameFileBody
功    能：生成文件信息帧2
输入参数：帧引用,_SendUserName:发送者用户名,_OrderNum:文件包序号,FileCont:文件内容文件,Contlength:内容部分长度
返 回 值：帧长度
说    明：函数外部定义一个帧的指针作为参数，在函数内申请空间
***************************************************************************/
int CrtFrameFileBody(FrameS2CFileBody*&p_Frame, char* _SendUserName, uint32_t _OrderNum, char* FileCont, uint16_t Contlength);
/*****************文件操作中C->S的解析帧函数*******************/
/***************************************************************************
函数名称：AnlsFrameFileHead
功    能：解析文件信息帧1
输入参数：帧的指针,接受者用户名,文件大小,文件包数量,文件名
返 回 值：
说    明：参数1将帧数组的数组名以强制类型转换的方式输入，参数2,5将用户名数组和文件片段数组的数组名输入，参数3,4以相应变量的引用方式输入
***************************************************************************/
int AnlsFrameFileHead(FrameC2SFileHead*p_Frame, char*_AcceptUserName, uint32_t&_FileSize, uint32_t&_PackNum, char* _FileName);
/***************************************************************************
函数名称：AnlsFrameFileBody
功    能：解析文件信息帧2
输入参数：帧的指针,接收文件用户姓名,文件包序号(引用),文件内容,数据长度(引用)
返 回 值：
说    明：参数1将帧数组的数组名以强制类型转换的方式输入，参数2和参数4将用户名数组和文件片段数组的数组名输入，参数3,5以相应变量的引用方式输入
***************************************************************************/
int AnlsFrameFileBody(FrameC2SFileBody*p_Frame, char*_AcceptUserName, uint32_t&_OrderNum, char* _FileCont, uint16_t&ContLength);
/***************************************************************************
函数名称：AnlsFrameFileDownload
功    能：解析下载通知帧
输入参数：帧的指针,发送文件用户名,文件名
返 回 值：
说    明：参数1将帧数组的数组名以强制类型转换的方式输入，参数2和参数4将用户名数组和文件片段数组的数组名输入，参数3,5以相应变量的引用方式输入
***************************************************************************/
int AnlsFrameFileDownload(FrameC2SFileDownload*p_Frame,char*_SendUserName,char*_FileName);
/***************************************************************************
函数名称：AnlsFrameFileHeadReply
功    能：解析文件信息帧1应答帧
输入参数：帧的指针
返 回 值：应答类型(AcceptOk(0x00):允许接收,AcceptRefuse(0x01):拒绝接收)
说    明：
***************************************************************************/
RplyAllowAccept AnlsFrameFileHeadReply(FrameC2SFileHeadReply*p_Frame);
/***************************************************************************
函数名称：AnlsFrameFileBodyReply
功    能：解析文本信息帧2应答帧
输入参数：帧的指针
返 回 值：文件包序号
说    明：
***************************************************************************/
uint32_t AnlsFrameFileBodyReply(FrameC2SFileBodyReply*p_Frame);

/***************************************************************************
  函数名称：get_file_size
  功    能：获取文件大小
  输入参数：路径
  返 回 值：
  说    明：
***************************************************************************/
uint32_t get_file_size(const char *path);
/***************************************************************************
  函数名称：CrtFileDir
  功    能：创建路径
  输入参数：
  返 回 值：
  说    明：
***************************************************************************/
int CrtFileDir(char*loc,char*SendName,char*RecvName,char*FileName);