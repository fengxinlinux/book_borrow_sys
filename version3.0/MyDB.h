/*************************************************************************
	> File Name: MyDB.h
	> Author: yanyuchen
	> Mail: 794990923@qq.com
	> Created Time: 2017年08月01日 星期二 10时26分18秒
 ************************************************************************/

#ifndef _MYDB_H
#define _MYDB_H
#include<iostream>
#include<string>
#include<mysql/mysql.h>
using namespace std;

class MyDB
{
    public:
    MyDB();
    ~MyDB();
    bool initDB(string host,string user,string pwd,string db_name); //连接mysql
    bool exeSQL(string sql);   //执行sql语句
    MYSQL_RES *result;
    MYSQL_ROW row; 
    private:
    MYSQL *mysql;          //连接mysql句柄指针
  //  MYSQL_RES *result;    //指向查询结果的指针
  //  MYSQL_ROW row;       //按行返回的查询信息
};


#endif
