/*************************************************************************
	> File Name: protocol.h
	> Author:fengxin 
	> Mail:903087053@qq.com 
	> Created Time: 2017年06月05日 星期一 20时55分40秒
 ************************************************************************/

#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#define NET_PACKET_DATA_SIZE 5000

/// 网络数据包包头
struct NetPacketHeader
{
    unsigned short      wDataSize;  ///< 数据包大小，包含封包头和封包数据大小
    unsigned short      wOpcode;    ///< 操作码
};

/// 网络数据包
struct NetPacket
{
    NetPacketHeader     Header;                         ///< 包头
    char       Data[NET_PACKET_DATA_SIZE];     ///< 数据
};



/// 网络操作码
enum eNetOpcode
{
    LOGIN  = 1,  //登录
    REGISTER,  //注册
    LOGIN_YES,  //登录成功
    LOGIN_NO,    //登录失败
    REGISTER_YES,  //注册成功
    REGISTER_NO,  //注册失败
    PERSONAL_DATA,
};



void my_err(const char *err_string,int line)  //自定义错误函数
{
    std::cerr<<"line:"<<line<<std::endl; //输出错误发生在第几行
    perror(err_string);       //输出错误信息提示
    exit(1);
}


#endif
