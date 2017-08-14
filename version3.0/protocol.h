/*************************************************************************
	> File Name: protocol.h
	> Author: yanyuchen
	> Mail: 794990923@qq.com
	> Created Time: 2017年08月01日 星期二 10时30分09秒
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
    PERSONAL_DATA,//查询个人信息
    ADD_BOOKS_INFO, //管理员添加书籍信息
    ADD_BOOKS_INFO_YES,//添加成功
    ADD_BOOKS_INFO_NO,//添加失败
    DEL_BOOKS_INFO, //管理员删除书籍信息
    DEL_BOOKS_INFO_YES,//删除成功
    DEL_BOOKS_INFO_NO,//删除失败
    CHAN_BOOKS_INFO,//管理员更改书籍信息
    CHAN_BOOKS_INFO_YES,//更改成功
    CHAN_BOOKS_INFO_NO,//更改失败
    SEA_BOOKS_INFO, //管理员搜索书籍信息
    SEA_BOOKS_INFO_YES,//管理员找到书籍信息
    SEA_BOOKS_INFO_NO,
    SEA_BOOKS_ALL_INFO,//搜索全部
    SEA_BOOKS_ALL_INFO_YES,//搜索全部成功
    SEA_BOOKS_ALL_INFO_NO,//搜索全部失败
    BOR_BOOK,           //借阅书籍
    BOR_BOOK_YES,       //借阅成功
    BOR_BOOK_NO,        //借阅失败
    RET_BOOK,           //归还书籍
    RET_BOOK_YES,       //归还成功
    RET_BOOK_NO,        //归还失败
    SEA_BOOK,           //查询书籍
    SEA_BOOK_YES,       //查询成功
    SEA_BOOK_NO,        //查询失败
    SEA_BOR_BOOK,       //查看借阅记录
    SEA_BOR_BOOK_YES,   //查看借阅记录成功
    SEA_BOR_BOOK_NO,    //查看借阅记录失败
    SEA_RET_BOOK,       //查询归还记录
    SEA_RET_BOOK_YES,   //查询归还记录成功
    SEA_RET_BOOK_NO,    //查询归还记录失败
    SEARCH_BOOK,         //搜索指定图书
    SEARCH_BOOK_YES,    //搜索图书成功
    SEARCH_BOOK_NO,     //搜索图书失败
    SEARCH_BOOK_END,    //搜索图书结束
};



void my_err(const char *err_string,int line)  //自定义错误函数
{
    std::cerr<<"line:"<<line<<std::endl; //输出错误发生在第几行
    perror(err_string);       //输出错误信息提示
    exit(1);
}

#endif
