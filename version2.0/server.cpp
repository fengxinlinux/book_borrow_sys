/*************************************************************************
	> File Name: server.cpp
	> Author: yanyuchen
	> Mail: 794990923@qq.com
	> Created Time: 2017年08月01日 星期二 08时42分31秒
 ************************************************************************/

#include<iostream>
#include<cstring>
#include<string>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/signal.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<errno.h>
#include<list>

#include <json/json.h>
#include"MyDB.h"
#include"protocol.h"

using namespace std;


#define PORT 6666   //服务器端口
#define LISTEN_SIZE 5000   //连接请求队列的最大长度
#define EPOLL_SIZE  5000   //epoll监听客户端的最大数目



class information
{
    public:

    char lnumber[20];   //账号
    int lsocket;         //套接字
    information()=default;
    information(char *p,int fd)
    {
        strcpy(lnumber,p);
        lsocket=fd;
    }
};


list<information>  head;


class TCPServer
{
    public: 

    TCPServer();
    ~TCPServer();

    /// 接受客户端接入
    void acceptClient();

    /// 关闭客户端
    void closeClient(int conn_fd);
    //处理接收到的数据
    bool dealwithpacket(TCPServer &server,int conn_fd,char *recv_data,uint16_t wOpcode,int datasize);

    bool server_recv(TCPServer &server,int conn_fd);  //接收数据函数
    bool server_send(int conn_fd,char send_buf[10000],int datasize,uint16_t wOpcode); //发送数据函数

    void run(TCPServer &server);  //运行函数

    int epollfd;  //epoll监听描述符
    private:

    int sock_fd;  //监听套接字
    int conn_fd;    //连接套接字
    socklen_t cli_len;  //记录连接套接字地址的大小
    struct epoll_event  event;   //epoll监听事件
    struct epoll_event*  events;  //epoll监听事件结果集合指针
    struct sockaddr_in cli_addr;  //客户端地址
    struct sockaddr_in serv_addr;   //服务器地址


};

TCPServer::TCPServer()  //构造函数
{

    //创建一个套接字
    sock_fd=socket(AF_INET,SOCK_STREAM,0);
    if(sock_fd<0)
    {
        my_err("socket",__LINE__);
    }
    //设置该套接字使之可以重新绑定端口
    int optval=1;
    if(setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,(void*)&optval,sizeof(int))<0)
    {
        my_err("setsock",__LINE__);
    }
    //初始化服务器端地址结构
    memset(&serv_addr,0,sizeof(struct sockaddr_in));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(PORT);
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(sock_fd,(struct sockaddr*)&serv_addr,sizeof(struct sockaddr_in))<0)
    {
        my_err("bind",__LINE__);
    }
    //将套接字转化为监听套接字
    if(listen(sock_fd,LISTEN_SIZE)<0)
    {
        my_err("listen",__LINE__);
    }


    cli_len=sizeof(struct sockaddr_in);
    events=(struct epoll_event*)malloc(sizeof(struct epoll_event)*EPOLL_SIZE); //分配内存空间

    //创建一个监听描述符epoll,并将监听套接字加入监听列表
    epollfd=epoll_create(EPOLL_SIZE);
    if(epollfd==-1)
    {
        my_err("epollfd",__LINE__);
    }
    event.events = EPOLLIN;
    event.data.fd = sock_fd;
    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,sock_fd,&event)<0)
    {
        my_err("epoll_ctl",__LINE__);
    }

}

TCPServer::~TCPServer()   //析构函数
{
    close(sock_fd);    //关闭监听套接字
    close(epollfd);   //关闭epoll描述符
    free(events);  //释放事件集空间
    cout<<"服务器成功退出"<<endl;
}

//线程参数结构体
struct pthread_arg
{
    public:
    TCPServer &server;
    int conn_fd;
    pthread_arg(TCPServer &server1);

};

pthread_arg::pthread_arg(TCPServer &server1):server(server1),conn_fd(0){}   //线程参数类构造函数

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;   //创建并初始化一个线程互斥锁


void TCPServer::acceptClient()      //接受客户端连接请求
{
    conn_fd=accept(sock_fd,(struct sockaddr*)&cli_addr,&cli_len);
    if(conn_fd<0)
    {
        my_err("accept",__LINE__);
    }
    event.events = EPOLLIN  |  EPOLLONESHOT; //监听连接套接字的可读
    event.data.fd = conn_fd;
    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,conn_fd,&event)<0) //将新连接的套接字加入监听
    {
        my_err("epoll",__LINE__);
    }
    cout<<"a connet is connected,ip is "<<inet_ntoa(cli_addr.sin_addr)<<endl;
}


void TCPServer::closeClient(int conn_fd)     //处理客户端退出
{
    cout<<"a connet is quit "<<endl;
    epoll_ctl(epollfd,EPOLL_CTL_DEL,conn_fd,&event);
    close(conn_fd);

}

//函数声明:
    //
bool Login(TCPServer &server,int conn_fd,char *recv_data);   //登录处理函数
bool Register(TCPServer &server,int conn_fd,char *recv_data);   //注册处理函数
void reset_oneshot(int epollfd,int fd);   //重置conn_fd上的EPOLLONESHOT事件
void* threadFunc(void *arg);  //线程处理函数
bool Personal_data(TCPServer &server,int conn_fd,char *recv_data); 


//重置fd上的EPOLLONESHOT事件
void reset_oneshot( int epollfd, int fd )
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN  | EPOLLONESHOT ;
    epoll_ctl( epollfd, EPOLL_CTL_MOD, fd, &event );
}

bool Login(TCPServer &server,int conn_fd,char *recv_data)   //登录
{
    Json::Value accounts;
    Json::Reader reader;
    string str(recv_data);

    if(reader.parse(str,accounts)<0)   //json解析失败
    {
        cout<<"json解析失败"<<endl;
        return false;

    }

    MyDB db;
    int flags=LOGIN_NO;
    if(db.initDB("localhost","root","yycwsndns","book_borrow_sys")==false)  //连接数据库
    {
        cout<<"连接数据库失败"<<endl;
        return false;
    }


    string sentence="select number,passwd from accounts;";
    if(db.exeSQL(sentence)==false)
    {
        cout<<"执行sql语句失败"<<endl;
        return false;
    }
    if(db.result)  //结果集中有数据
    {
        int num_fields=mysql_num_fields(db.result);   //获取结果集中总共字段数
        int num_rows=mysql_num_rows(db.result);     //获取结果集中总共字段数集中总共行数
        for(int i=0;i<num_rows;i++)
        {
            db.row=mysql_fetch_row(db.result);
            if(db.row[0]==accounts["number"].asString())
            {
                if(db.row[1]==accounts["passwd"].asString())  //密码正确
                {
                    flags=LOGIN_YES;
                    
                    //information data;
                    //strcpy(data.lnumber, db.row[0]);
                    //data.lsocket = conn_fd;
                    //head.push_back(data);
                    head.emplace_back(db.row[0],conn_fd);

                    break;
                }
            }
        }
    }
    if(server.server_send(conn_fd,NULL,0,flags)==false)
    {
        cout<<"向客户端发送发送数据失败"<<endl;
        return false;
    }

    return true;

}

bool Register(TCPServer &server,int conn_fd,char* recv_data)    //注册
{
    Json::Value accounts;
    Json::Reader reader;
    string str(recv_data);


    if(reader.parse(str,accounts)<0)   //json解析失败
    {
        cout<<"json解析失败"<<endl;
        return false;

    }


    MyDB db;
    int flags=REGISTER_NO;
    if(db.initDB("localhost","root","yycwsndns","book_borrow_sys")==false)  //连接数据库
    {
        cout<<"连接数据库失败"<<endl;
        return false;
    }
    string sentence="select number,passwd from accounts where number=\"" + accounts["number"].asString()+"\";";
   

    if(db.exeSQL(sentence)==false)
    {
        cout<<"执行sql语句失败"<<endl;
    }


    if(db.result&&mysql_num_rows(db.result)==0)  //注册表中不存在该帐号
    {
        flags=REGISTER_YES;
    }
    else
    {
        flags=REGISTER_NO;
    }
    sentence.clear();
    sentence="insert accounts (number,passwd,nickname,sex,address,birthdate,phone) value(\""+accounts["number"].asString()+"\""+",\""+accounts["passwd"].asString()+"\""+",\""+accounts["nickname"].asString()+"\""+",\""+accounts["sex"].asString()+"\""+",\""+accounts["address"].asString()+"\""+",\""+accounts["birthdate"].asString()+"\""+",\""+accounts["phone"].asString()+"\""+");";
    if(flags==REGISTER_YES)
    {
        if(db.exeSQL(sentence)==false)
        {
            cout<<"执行sql语句失败"<<endl;
            flags=REGISTER_NO;
        }
    }
    if(server.server_send(conn_fd,NULL,0,flags)==false)
    {
        cout<<"向客户端发送数据失败"<<endl;
        return false;
    }


    return true;

}

bool Personal_data(TCPServer &server,int conn_fd,char *recv_data) 
{
    char buf[10000];
    string out;
    Json::Value accounts;

    MyDB db;
    if(db.initDB("localhost","root","yycwsndns","book_borrow_sys")==false)  //连接数据库
    {
        cout<<"连接数据库失败"<<endl;
        return false;
    }
  


    string sentence = "";
    auto it = head.begin();
    while (it != head.end()) {
        if (it -> lsocket == conn_fd) {
            sentence = sentence + "select number,passwd,nickname,sex,address,birthdate,phone from accounts where number='" + it->lnumber + "';";
            break;
        }
        it++;
    }




    if(db.exeSQL(sentence)==false)
    {
        cout<<"执行sql语句失败"<<endl;
    }
    if(db.result)  //结果集中有数据
    {
        db.row=mysql_fetch_row(db.result);
        accounts["number"] = db.row[0];
        accounts["passwd"] = db.row[1];
        accounts["nickname"] = db.row[2];
        accounts["sex"] = db.row[3];
        accounts["address"] = db.row[4];
        accounts["birthdate"] = db.row[5];
        accounts["phone"] = db.row[6];
        out  = accounts.toStyledString();
       
        memcpy(buf,out.c_str(),out.size());
    }


    if(server.server_send(conn_fd,buf,out.size(),PERSONAL_DATA)==false)
    {
        cout<<"向客户端发送发送数据失败"<<endl;
        return false;
    }


    return true;
}

void* threadFunc(void *arg)   //线程处理函数
{

    pthread_mutex_lock(&mutex);  //加锁
    //将参数结构体的内容复制
    struct pthread_arg p=*((struct pthread_arg*)arg);
    pthread_mutex_unlock(&mutex);  //解锁

    pthread_detach(pthread_self());   //将线程设置成detached状态，该线程运行结束后会自动释放所有资源

    //执行服务器接收处理函数
    p.server.server_recv(p.server,p.conn_fd);


    //重置conn_fd上的EPOLLONESHOT事件

    reset_oneshot(p.server.epollfd,p.conn_fd);

    pthread_exit(NULL);

} 


bool TCPServer::dealwithpacket(TCPServer &server,int conn_fd, char *recv_data,uint16_t wOpcode,int datasize)  //处理接收到的数据
{

    if(wOpcode==LOGIN)  //登录
    {
        if(Login(server,conn_fd,recv_data)==false)
        {
            cout<<"登录处理失败"<<endl;
            return false;
        }
    }
    else if(wOpcode==REGISTER)  //注册
    {
        if(Register(server,conn_fd,recv_data)==false)
        {
            cout<<"注册处理失败"<<endl;
            return false;
        }
    }
    else if(wOpcode==PERSONAL_DATA) //查询个人信息
    {
        if(Personal_data(server,conn_fd,recv_data)==false) 
        {
            cout << "查询信息失败"<<endl;
            return false;
        }
    }


    return true;

}

bool TCPServer::server_send(int conn_fd, char send_buf[10000],int datasize,uint16_t wOpcode) //发送数据
{
    NetPacket send_packet;  //数据包
    send_packet.Header.wDataSize=datasize+sizeof(NetPacketHeader); //数据包大小
    send_packet.Header.wOpcode=wOpcode;

    memcpy(send_packet.Data,send_buf,datasize); //数据拷贝

    if(send(conn_fd,&send_packet,send_packet.Header.wDataSize,0))
    return true;
    else
    return false;
}

bool TCPServer::server_recv(TCPServer &server,int conn_fd)  //接收数据函数
{
    int nrecvsize=0; //一次接收到的数据大小
    int sum_recvsize=0; //总共收到的数据大小
    int packersize;   //数据包总大小
    int datasize;     //数据总大小
    char recv_buffer[10000];  //接收数据的buffer

    memset(recv_buffer,0,sizeof(recv_buffer));  //初始化接收buffer

    
    while(sum_recvsize!=sizeof(NetPacketHeader))
    {
        nrecvsize=recv(conn_fd,recv_buffer+sum_recvsize,sizeof(NetPacketHeader)-sum_recvsize,0);
        if(nrecvsize==0)
        {
            //处理客户端退出;
            server.closeClient(conn_fd);
            
            auto it = head.begin();
            while (it != head.end()) {
                if (it->lsocket == conn_fd) {
                    it = head.erase(it);
                }
                else 
                    it++;
            }

            return false;
        }
        if(nrecvsize<0)
        {
            cout<<"从客户端接收数据失败"<<endl;
            return false;
        }
        sum_recvsize+=nrecvsize;

    }
    
    NetPacketHeader *phead=(NetPacketHeader*)recv_buffer;
    packersize=phead->wDataSize;  //数据包大小
    datasize=packersize-sizeof(NetPacketHeader);     //数据总大小
    

    while(sum_recvsize!=packersize)
    {
        nrecvsize=recv(conn_fd,recv_buffer+sum_recvsize,packersize-sum_recvsize,0);
        if(nrecvsize==0)
        {
            //客户端退出
            server.closeClient(conn_fd);
            
            auto it = head.begin();
            while (it != head.end()) {
                if (it->lsocket == conn_fd) {
                    it = head.erase(it);
                }
                else 
                    it++;
            }
            
            return false;
        }
        else if(nrecvsize<0)
        {
            cout<<"从客户端接收数据失败"<<endl;
            return false;
        }
        sum_recvsize+=nrecvsize;
    }
    
    dealwithpacket(server,conn_fd,(char*)(phead+1),phead->wOpcode,datasize);  //处理接收到的数据



}

void TCPServer::run(TCPServer &server)  //主执行函数
{
    pthread_t tt;
    pthread_arg p(server);

    while(1)   //循环监听事件
    {

        int sum=0,i;
        sum=epoll_wait(epollfd,events,EPOLL_SIZE,-1);


        for(i=0;i<sum;i++)
        {
            if(events[i].data.fd==sock_fd)    //客户端请求连接
            {
                acceptClient();  //处理客户端的连接请求

            }
            else if(events[i].events&EPOLLIN)    //客户端发来数据
            {
                pthread_mutex_lock(&mutex);     //加锁
                p.conn_fd=events[i].data.fd;
                pthread_mutex_unlock(&mutex);   //解锁

                if(pthread_create(&tt,NULL,threadFunc,(void*)&p)<0)
                {
                    cout<<"创建线程失败"<<endl;
                    return ;
                }    
            }
        }
    }
}

int main()
{
    TCPServer server;
    server.run(server);

    return 0;
}
