/*************************************************************************
	> File Name: client.cpp
	> Author: yanyuchen
	> Mail: 794990923@qq.com
	> Created Time: 2017年08月01日 星期二 08时21分29秒
 ************************************************************************/
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<sys/signal.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include <termios.h>
#include<assert.h>

#include <json/json.h>
#include"protocol.h"

#define PORT 6666   //服务器端口


//网络操作码flags:
    //定义在protocol头文件中

using namespace std;


class TCPClient
{
    public:

    TCPClient(int argc ,char** argv);
    ~TCPClient();

    char data_buffer[10000];    //存放发送和接收数据的buffer


    //向服务器发送数据
    bool send_to_serv(int datasize,uint16_t wOpcode);
    //从服务器接收数据
    bool recv_from_serv();
    void run(TCPClient & client); //主运行函数


    private:

    int conn_fd; //创建连接套接字
    struct sockaddr_in serv_addr; //储存服务器地址

};




TCPClient::TCPClient(int argc,char **argv)  //构造函数
{

    if(argc!=3)    //检测输入参数个数是否正确
    {
        cout<<"Usage: [-a] [serv_address]"<<endl;
        exit(1);
    }

    memset(data_buffer,0,sizeof(data_buffer));  //初始化buffer

    //初始化服务器地址结构
    memset(&serv_addr,0,sizeof(struct sockaddr_in));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(PORT);
    
    //从命令行服务器地址
    for(int i=0;i<argc;i++)
    {
        if(strcmp("-a",argv[i])==0)
        {

            if(inet_aton(argv[i+1],&serv_addr.sin_addr)==0)
            {
                cout<<"invaild server ip address"<<endl;
                exit(1);
            }
            break;
        }
    }

    //检查是否少输入了某项参数
    if(serv_addr.sin_addr.s_addr==0)
    {
        cout<<"Usage: [-a] [serv_address]"<<endl;
        exit(1);
    }

    //创建一个TCP套接字
    conn_fd=socket(AF_INET,SOCK_STREAM,0);


    if(conn_fd<0)
    {
        my_err("connect",__LINE__);
    }
    

    //向服务器发送连接请求
    if(connect(conn_fd,(struct sockaddr*)&serv_addr,sizeof(struct sockaddr))<0)
    {
        my_err("connect",__LINE__);
    }

}

TCPClient::~TCPClient()  //析构函数
{
    close(conn_fd);

}

bool TCPClient::send_to_serv(int datasize,uint16_t wOpcode) //向服务器发送数据
{
    NetPacket send_packet;   //数据包
    send_packet.Header.wDataSize=datasize+sizeof(NetPacketHeader);  //数据包大小
    send_packet.Header.wOpcode=wOpcode;


    memcpy(send_packet.Data,data_buffer,datasize);  //数据拷贝


    if(send(conn_fd,&send_packet,send_packet.Header.wDataSize,0))
    return true;
    else
    return false;

}

bool TCPClient::recv_from_serv()   //从服务器接收数据
{
    int nrecvsize=0; //一次接收到的数据大小
    int sum_recvsize=0; //总共收到的数据大小
    int packersize;   //数据包总大小
    int datasize;     //数据总大小


    memset(data_buffer,0,sizeof(data_buffer));   ///初始化buffer

      while(sum_recvsize!=sizeof(NetPacketHeader))
    {
        nrecvsize=recv(conn_fd,data_buffer+sum_recvsize,sizeof(NetPacketHeader)-sum_recvsize,0);
        if(nrecvsize==0)
        {
            //服务器退出;
            return false;
        }
        if(nrecvsize<0)
        {
            cout<<"从客户端接收数据失败"<<endl;
            return false;
        }
        sum_recvsize+=nrecvsize;

    }



    NetPacketHeader *phead=(NetPacketHeader*)data_buffer;
    packersize=phead->wDataSize;  //数据包大小
    datasize=packersize-sizeof(NetPacketHeader);     //数据总大小




    while(sum_recvsize!=packersize)
    {
        nrecvsize=recv(conn_fd,data_buffer+sum_recvsize,packersize-sum_recvsize,0);
        if(nrecvsize==0)
        {
            cout<<"从客户端接收数据失败"<<endl;
            return false;
        }
        sum_recvsize+=nrecvsize;
    }

    return true;

}

//函数声明:
bool inputpasswd(string &passwd);   //无回显输入密码
void Login_Register(TCPClient &client);    //登录注册函数
void menu(TCPClient &client);  //用户菜单函数
void Admin_menu(TCPClient &client); //管理员菜单函数
bool search_book(TCPClient &client);   //搜索指定图书函数




bool inputpasswd(string &passwd)   //无回显输入密码
{
    struct termios tm,tm_old;
    int fd = STDIN_FILENO, c,d;
    if(tcgetattr(fd, &tm) < 0)
    {
        return false;
    }
    tm_old = tm;
    cfmakeraw(&tm);
    if(tcsetattr(fd, TCSANOW, &tm) < 0)
    {
        return false;
    }
    cin>>passwd;
    if(tcsetattr(fd, TCSANOW, &tm_old) < 0)
    {
        return false;
    }
    return true;
}

void Login_Register(TCPClient &client)   //登录注册函数
{

    string choice;           //记录选择
    int choice_right=1;     //判断输入选项是否正确
    string number,nickname,sex,address,birthdate, phone;               
    string passwd,passwd1;
    Json::Value accounts;


    while(choice_right)
    {
        cout<<"\t\t欢迎使用图书借阅系统:"<<endl;
        cout<<"\t\t\t1.登录"<<endl;
        cout<<"\t\t\t2.注册"<<endl;
        cout<<"\t\t\t3.退出"<<endl;
        cout<<"\n请输入选择序号:";
        cin>>choice;

        if(choice=="1")  //登录
        {
            choice_right=0;
            cout<<"请输入帐号:";
            cin>>number;
            cout<<"请输入密码:"; 
            inputpasswd(passwd);
            cout<<endl;
            accounts["number"]=number.c_str();           //加入json对象中
            accounts["passwd"]=passwd.c_str();
            string out=accounts.toStyledString();
            memcpy(client.data_buffer,out.c_str(),out.size());   //拷贝到数据buffer
            if(client.send_to_serv(out.size(),LOGIN)==false)  //向服务器发送数据
            {
                cout<<"向服务器发送数据失败"<<endl;
                return;
            }
            if(client.recv_from_serv()==false)   //从服务器接收数据
            {
                cout<<"从服务器接收数据失败"<<endl;
                return;
            }

            NetPacketHeader *phead=(NetPacketHeader*)client.data_buffer;
            if(phead->wOpcode==LOGIN_YES) //登录成功
            {
                cout<<"登录成功"<<endl;
                if (accounts["number"] == "admin") {          //判断是否为管理员
                    Admin_menu(client);
                }
                menu(client);
            }
            else
            {
                cout<<"登录失败，帐号或密码错误"<<endl;
                choice_right=1;
            }


        }
        else if(choice=="2")  //注册
        {
            choice_right=0;
            cout<<"请输入要注册的帐号:";
            cin>>number;
            cout<<"请输入昵称:";
            cin>>nickname;
            cout<<"请输入性别:";
            cin>>sex;
            cout<<"请输入地址:";
            cin>>address;
            cout<<"请输入生日:";
            cin>>birthdate;
            cout<<"请输入手机号码:";
            cin>>phone;
            cout<<"请设置密码:";
            inputpasswd(passwd);
            cout<<endl;
            cout<<"请再次输入密码:";
            inputpasswd(passwd1);
            cout<<endl;
            while(passwd!=passwd1)
            {
                cout<<"两次输入的密码不同，请重新设置密码。"<<endl;
                cout<<"请设置密码:";
                inputpasswd(passwd);
                cout<<endl;
                cout<<"请再次输入密码:";
                inputpasswd(passwd1);
                cout<<endl;
            }
            accounts["number"]=number.c_str();
            accounts["nickname"]=nickname.c_str();
            accounts["sex"]=sex.c_str();
            accounts["address"]=address.c_str();
            accounts["birthdate"]=birthdate.c_str();
            accounts["phone"]=phone.c_str();
            accounts["passwd"]=passwd.c_str();
            string out=accounts.toStyledString();
            memcpy(client.data_buffer,out.c_str(),out.size());  //拷贝到数据buffer中


            if(client.send_to_serv(out.size(),REGISTER)==false)  //向服务器发送数据
            {
                cout<<"向服务器发送数据失败"<<endl;
                return;
            }
            if(client.recv_from_serv()==false)   //从服务器接收数据
            {
                cout<<"从服务器接收数据失败"<<endl;
                return;
            }

            NetPacketHeader *phead=(NetPacketHeader*)client.data_buffer;
            if(phead->wOpcode==REGISTER_YES)  //注册成功
            {
                cout<<"注册成功"<<endl;
                Login_Register(client);   
            }
            else
            {
                cout<<"注册失败,该帐号已被注册"<<endl;
            }
            
        }
            
        else if(choice=="3")
        {
            cout<<"Bye"<<endl;
            exit(0);
        }
        else
        {
            cout<<"输入格式错误，请重新输入"<<endl;
        }
    }
}

void Personal_data(TCPClient &client)
{
    if(client.send_to_serv(0, PERSONAL_DATA)==false)  //向服务器发送数据失败
    {
        cout<<"向服务器发送数据失败"<<endl;
        return;
    }
    cout<<"recv_from_serv"<<endl;////////////

    if(client.recv_from_serv()==false)   //从服务器接收数据
    {
        cout<<"从服务器接收数据失败"<<endl;
        return;
    }
    
    cout<<"recv end"<<endl;/////////

    NetPacket *phead=(NetPacket*)client.data_buffer;
    
    Json::Value accounts;
    Json::Reader reader;
    string str(phead->Data);
     
    if (reader.parse(str,accounts) < 0)
    {
        cout << "json解析失败" << endl;
    }
    else {
        cout << "账号：" << accounts["number"].asString()<< endl;
        cout << "密码：" << accounts["passwd"].asString()<< endl;
        cout << "昵称：" << accounts["nickname"].asString()<< endl;
        cout << "性别：" << accounts["sex"].asString()<< endl;
        cout << "地址：" << accounts["address"].asString()<< endl;
        cout << "生日：" << accounts["birthdate"].asString()<< endl;
        cout << "电话：" << accounts["phone"].asString()<< endl;
    }
    cout << "按任意键返回菜单\n" << endl;
    getchar(); 
    getchar(); 
    menu(client);
}
bool add_books_info(TCPClient &client)//上架书籍
{
    bool non_stop = true;
    string yes_or_no;
    Json::Value book;
    string ISBN,book_name,publish_house,author,count,stat;

    while(non_stop){
        cout << endl;
        cout << endl;
        cout << "是否开始本次录入[yes/no]:";
        cin >> yes_or_no;
        if(yes_or_no != "yes"){
            cout << "您输入的选项不是yes,已经退出上线图书功能" << endl;
            non_stop = false;
            continue;
        }
        cout << "请输入图书ISBN:";
        cin>>ISBN;
        cout << "请输入图书名称:";
        cin>>book_name;
        cout << "请输入图书出版社:";
        cin>>publish_house;
        cout << "请输入图书作者:";
        cin>>author;
        cout << "请输入图书数量:";
        cin>>count;
        do{
            cout << "请输入图书是否可借阅[yes/no]:";
            cin >> stat;
        }while((stat != "yes")&& (stat != "no"));
        
        book["ISBN"] = ISBN.c_str();
        book["book_name"] = book_name.c_str();
        book["publish_house"] = publish_house.c_str();
        book["author"] = author.c_str();
        book["count"] = count.c_str();
        book["stat"] = stat.c_str();

        string out = book.toStyledString();
        memcpy(client.data_buffer,out.c_str(),out.size());
        if(client.send_to_serv(out.size(),ADD_BOOKS_INFO) == false){
            cout << "发送数据失败!" << endl;
            return false;
        }
        if(client.recv_from_serv() == false){
             cout << "接收数据失败"<<endl;
             return false;
        }else{
            NetPacketHeader *phead = (NetPacketHeader*)client.data_buffer;
            if(phead -> wOpcode == ADD_BOOKS_INFO_YES){
                cout << "书籍添加成功" << endl;
            }else{
                cout << "书籍添加失败,原因可能是书籍在数据库中已存在" << endl;
                continue;
            }
        }
        cout << endl;
    }
    return true;
}

void del_isbn(TCPClient &client) //按isbn下架书籍
{
    
    bool non_stop = true;
    string temp_isbn1;
    string temp_isbn2;
    while(non_stop){
        cout << "请输入ISBN"<<endl;
        cin >> temp_isbn1;
        cout << "请再次输入ISBN确认"<<endl;
        cin >> temp_isbn2;
        if(temp_isbn1 == temp_isbn2){
            cout << "确认成功,正在删除..."<<endl;
            break;
        }
        else{
            cout << "两次输入不一致，请重新输入"<< endl;
            continue;
        }
    }
    Json:: Value book;
    string nu("");
    book["ISBN"] = temp_isbn1.c_str();
    book["book_name"] = nu.c_str();
    book["publish_house"] = nu.c_str();
    book["author"] = nu.c_str();
    book["count"] = nu.c_str();
    book["stat"] = nu.c_str();

    string out = book.toStyledString();
    memcpy(client.data_buffer,out.c_str(),out.size());
    if(client.send_to_serv(out.size(),DEL_BOOKS_INFO) == false){
        cout << "发送数据失败"<<endl;
        
    }
    if(client.recv_from_serv() == false){
        cout << "接收服务器发来的消息失败"<<endl;
    }
    NetPacketHeader *phead = (NetPacketHeader*)client.data_buffer;
    if(phead -> wOpcode == DEL_BOOKS_INFO_YES){
        cout << "删除成功"<< endl;
        return ;
    }
    if(phead -> wOpcode == SEA_BOOKS_INFO_NO){
        cout << "书籍不存在,没办法删除" << endl;
        return ;
    }
    if(phead -> wOpcode == DEL_BOOKS_INFO_NO){
        cout << "删除失败" << endl;
        return ;
    }
    
}
void del_book_name(TCPClient &client)//按书名下架函数
{
    bool non_stop = true;
    string temp_name1;
    string temp_name2;
    while(non_stop){
        cout << "请输入书籍名称:";
        cin >> temp_name1;
        cout << "请再次输入书籍名称确认:";
        cin >> temp_name2;
        if(temp_name1 == temp_name2){
            cout << "确认成功,正在删除"<<endl;
            break;
        }
        else{
            cout << "两次输入不一致,请重新输入"<<endl;
            continue;
        }
    }
    Json::Value book;
    string nu("");
    book["ISBN"] = nu.c_str();
    book["book_name"] = temp_name1.c_str();
    book["publish_house"] = nu.c_str();
    book["author"] = nu.c_str();
    book["count"] = nu.c_str();
    book["stat"] = nu.c_str();

    string out = book.toStyledString();
    memcpy(client.data_buffer,out.c_str(),out.size());
    if(client.send_to_serv(out.size(),DEL_BOOKS_INFO) == false){
        cout << "发送数据失败"<<endl;
        return ;
    }
    if(client.recv_from_serv() == false){
        cout << "接收服务器发来的消息失败"<<endl;
        return ;
    }
    NetPacketHeader *phead = (NetPacketHeader*)client.data_buffer;
    if(phead -> wOpcode == DEL_BOOKS_INFO_YES){
        cout << "删除成功"<<endl;
        return ;
    }
    if(phead -> wOpcode == SEA_BOOKS_INFO_NO){
        cout << "书籍不存在,没办法删除"<<endl;
        return ;
    }
    if(phead -> wOpcode == DEL_BOOKS_INFO_NO){
        cout << "删除失败"<<endl;
        return ;
    }

}
bool del_books_info(TCPClient &client){ //下架相关书籍函数1
    bool non_stop = true;
    string yes_or_no;
    Json::Value book;
    string ISBN,book_name,publish_house,author,count,stat;
    while(non_stop){
        cout << endl;
        cout << endl;
        cout << "是否开始删除程序[yes/no]:";
        cin >> yes_or_no;
        if(yes_or_no == "yes"){
            int choi;
            cout << "\t\t1.根据ISBN删除"<<endl;
            cout << "\t\t2.根据书籍名称删除"<<endl;
            //cout << "\t\t3.退出 "<< endl;
            cin >> choi;
            if((choi != 1) && (choi != 2) ){
                cout << "输入有误";
                continue;
            }
            switch(choi){
                case 1:del_isbn(client);break;
                case 2:del_book_name(client);break;
                //case 3:cout << "退出"<< endl;            }
            }
        }else{
            cout << "您选择的不是yes,现已退出删除程序"<<endl;
            non_stop = false;
        }
    }
}
void  fill_in_json(Json::Value &book) //填book信息
{
    string nu("");
    book["publish_house"] =nu.c_str();
    book["author"] = nu.c_str();
    book["count"] = nu.c_str();
    book["stat"] = nu.c_str();
}

void print(Json::Value &book_recv) //打印信息
{
    cout << "当前图书信息是:"<<endl;
        cout << "ISBN:"<<book_recv["ISBN"].asString()<<endl;
        cout << "书籍名称:"<<book_recv["book_name"].asString()<<endl;
        cout << "出版社:"<< book_recv["publish_house"].asString()<<endl;
        cout << "库存数量:"<<book_recv["count"].asString()<<endl;
        cout << "是否可借阅:"<<book_recv["stat"].asString()<<endl;
}
bool chan_publish(TCPClient&client,Json::Value &book) //改变书籍出版社
{
    fill_in_json(book);
    string out = book.toStyledString();
    memcpy(client.data_buffer,out.c_str(),out.size());
    if(client.send_to_serv(out.size(),SEA_BOOKS_INFO) == false){
        cout << "发送数据失败"<<endl;
        return false;
    }
    if(client.recv_from_serv() == false){
        cout << "从服务器接收失败"<<endl;
        return false;
    }
    NetPacket *phead = (NetPacket*)client.data_buffer;
    if(phead -> Header .wOpcode == SEA_BOOKS_INFO){
        cout << "服务器查找错误"<<endl;
        return false;
    }else if(phead -> Header.wOpcode == SEA_BOOKS_INFO_NO){
        cout << "没有这本书,请重试..."<<endl;
        return false;
    }
    Json::Value book_recv;
    Json::Reader reader;
    string str(phead -> Data);
    if(reader.parse(str,book_recv) < 0){
        cout << "json解析失败"<<endl;
        return false;
    }
    else{
        print(book_recv);
    }
    bool non_stop= true;
    while(non_stop){
        string yes_or_no;
        string of_course;
        string publish_house;
        cout << "是否修改出版社[yes/no]:"<<endl;
        cin >> yes_or_no;
        if(yes_or_no != "yes"){
            cout << "您选择的不是yes,修改出版社的程序即将退出"<<endl;
            non_stop = false;
            return false;
        }
        cout << "出入新的出版社名称:";
        cin >> publish_house;
        cout << "确认吗[yes/no]:";
        cin >> of_course;
        if(of_course != "yes") continue;
        else{
            book_recv["publish_house"] = publish_house.c_str();
            string out = book_recv.toStyledString();
            memcpy(client.data_buffer,out.c_str(),out.size());
            if(client.send_to_serv(out.size(),CHAN_BOOKS_INFO) == false) {
                cout << "更改的请求发送失败"<<endl;
                return false;
            }
            if(client.recv_from_serv() == false){
                cout << "接收来自服务器的请求失败"<<endl;
                return false;
            }
            NetPacketHeader *pheadd = (NetPacketHeader*)client.data_buffer;
            if(pheadd -> wOpcode == CHAN_BOOKS_INFO_YES) {
                cout << "更改成功"<<endl;
                non_stop = false;
                return true;
            }else{
                cout << "更改失败"<<endl;
                return false;
            }
        }
    }
}

bool chan_author(TCPClient &client,Json::Value &book)//改变书籍作者
{
     fill_in_json(book);
    string out = book.toStyledString();
    memcpy(client.data_buffer,out.c_str(),out.size());
    if(client.send_to_serv(out.size(),SEA_BOOKS_INFO) == false){
        cout << "发送数据失败"<<endl;
        return false;
    }
    if(client.recv_from_serv() == false){
        cout << "从服务器接收失败"<<endl;
        return false;
    }
    NetPacket *phead = (NetPacket*)client.data_buffer;
    if(phead -> Header .wOpcode == SEA_BOOKS_INFO){
        cout << "服务器查找错误"<<endl;
        return false;
    }else if(phead -> Header.wOpcode == SEA_BOOKS_INFO_NO){
        cout << "没有这本书,请重试..."<<endl;
        return false;
    }
    Json::Value book_recv;
    Json::Reader reader;
    string str(phead -> Data);
    if(reader.parse(str,book_recv) < 0){
        cout << "json解析失败"<<endl;
        return false;
    }
    else{
        print(book_recv);
    }
    bool non_stop= true;
    while(non_stop){
        string yes_or_no;
        string of_course;
        string author;
        cout << "是否修改作者[yes/no]:"<<endl;
        cin >> yes_or_no;
        if(yes_or_no != "yes"){
            cout << "您选择的不是yes,修改作者的程序即将退出"<<endl;
            non_stop = false;
            return false;
        }
        cout << "出入新的作者名字:";
        cin >> author;
        cout << "确认吗[yes/no]:";
        cin >> of_course;
        if(of_course != "yes") continue;
        else{
            book_recv["author"] = author.c_str();
            string out = book_recv.toStyledString();
            memcpy(client.data_buffer,out.c_str(),out.size());
            if(client.send_to_serv(out.size(),CHAN_BOOKS_INFO) == false) {
                cout << "更改的请求发送失败"<<endl;
                return false;
            }
            if(client.recv_from_serv() == false){
                cout << "接收来自服务器的请求失败"<<endl;
                return false;
            }
            NetPacketHeader *pheadd = (NetPacketHeader*)client.data_buffer;
            if(pheadd -> wOpcode == CHAN_BOOKS_INFO_YES) {
                cout << "更改成功"<<endl;
                non_stop = false;
                return true;
            }else{
                cout << "更改失败"<<endl;
                return false;
            }
    
        }
    }
}
bool chan_count(TCPClient&client,Json::Value &book)//改变书籍库存函数
{
     fill_in_json(book);
    string out = book.toStyledString();
    memcpy(client.data_buffer,out.c_str(),out.size());
    if(client.send_to_serv(out.size(),SEA_BOOKS_INFO) == false){
        cout << "发送数据失败"<<endl;
        return false;
    }
    if(client.recv_from_serv() == false){
        cout << "从服务器接收失败"<<endl;
        return false;
    }
    NetPacket *phead = (NetPacket*)client.data_buffer;
    if(phead -> Header .wOpcode == SEA_BOOKS_INFO){
        cout << "服务器查找错误"<<endl;
        return false;
    }else if(phead -> Header.wOpcode == SEA_BOOKS_INFO_NO){
        cout << "没有这本书,请重试..."<<endl;
        return false;
    }
    Json::Value book_recv;
    Json::Reader reader;
    string str(phead -> Data);
    if(reader.parse(str,book_recv) < 0){
        cout << "json解析失败"<<endl;
        return false;
    }
    else{
        print(book_recv);
    }
    bool non_stop= true;
    while(non_stop){
        string yes_or_no;
        string of_course;
        string count;
        cout << "是否修改库存[yes/no]:";
        cin >> yes_or_no;
        if(yes_or_no != "yes"){
            cout << "您选择的不是yes,修改库存的程序即将退出"<<endl;
            non_stop = false;
            return false;
        }
        cout << "输入想改变的库存(+5:增加5本书/-5:减少5本书):";
        cin >> count;
        cout << "确认吗[yes/no]:";
        cin >> of_course;
        
        if(of_course != "yes") continue;
        else{
            if(atoi(book_recv["count"].asString().c_str())+atoi(count.c_str())<0)
            {
                cout<<"库存数量不够"<<endl;
                continue;
            }
            count = book_recv["count"].asString() + count;
            book_recv["count"] = count.c_str();
            string out = book_recv.toStyledString();
            memcpy(client.data_buffer,out.c_str(),out.size());
            if(client.send_to_serv(out.size(),CHAN_BOOKS_INFO) == false) {
                cout << "更改的请求发送失败"<<endl;
                return false;
            }
            if(client.recv_from_serv() == false){
                cout << "接收来自服务器的请求失败"<<endl;
                return false;
            }
            NetPacketHeader *pheadd = (NetPacketHeader*)client.data_buffer;
            if(pheadd -> wOpcode == CHAN_BOOKS_INFO_YES) {
                cout << "更改成功"<<endl;
                non_stop = false;
                return true;
            }else{
                cout << "更改失败"<<endl;
                return false;
            }
    
        }
    }
}
bool chan_stat(TCPClient &client,Json:: Value &book) //改变书籍状态函数
{
       fill_in_json(book);
    string out = book.toStyledString();
    memcpy(client.data_buffer,out.c_str(),out.size());
    if(client.send_to_serv(out.size(),SEA_BOOKS_INFO) == false){
        cout << "发送数据失败"<<endl;
        return false;
    }
    if(client.recv_from_serv() == false){
        cout << "从服务器接收失败"<<endl;
        return false;
    }
    NetPacket *phead = (NetPacket*)client.data_buffer;
    if(phead -> Header .wOpcode == SEA_BOOKS_INFO){
        cout << "服务器查找错误"<<endl;
        return false;
    }else if(phead -> Header.wOpcode == SEA_BOOKS_INFO_NO){
        cout << "没有这本书,请重试..."<<endl;
        return false;
    }
    Json::Value book_recv;
    Json::Reader reader;
    string str(phead -> Data);
    if(reader.parse(str,book_recv) < 0){
        cout << "json解析失败"<<endl;
        return false;
    }
    else{
        print(book_recv);
    }
    bool non_stop= true;
    while(non_stop){
        string yes_or_no;
        string of_course;
        string stat;
        cout << "是否修改可借阅状态[yes/no]:";
        cin >> yes_or_no;
        if(yes_or_no != "yes"){
            cout << "您选择的不是yes,修改借阅状态的程序即将退出"<<endl;
            non_stop = false;
            return false;
        }
        cout << "输入借阅状态:";
        cin >> stat;
        cout << "确认吗[yes/no]:";
        cin >> of_course;
        if(of_course != "yes") continue;
        else{
            book_recv["stat"] = stat.c_str();
            string out = book_recv.toStyledString();
            memcpy(client.data_buffer,out.c_str(),out.size());
            if(client.send_to_serv(out.size(),CHAN_BOOKS_INFO) == false) {
                cout << "更改的请求发送失败"<<endl;
                return false;
            }
            if(client.recv_from_serv() == false){
                cout << "接收来自服务器的请求失败"<<endl;
                return false;
            }
            NetPacketHeader *pheadd = (NetPacketHeader*)client.data_buffer;
            if(pheadd -> wOpcode == CHAN_BOOKS_INFO_YES) {
                cout << "更改成功"<<endl;
                non_stop = false;
                return true;
            }else{
                cout << "更改失败"<<endl;
                return false;
            }
    
        }
    }
}

bool isbn_or_name(TCPClient &client) //改变书籍信息函数2
{
    Json::Value book;
    int choi;
    
    cout << "\t\t1.输入ISBN更改书籍"<<endl;
    cout << "\t\t2.输入书籍名称更改书籍"<<endl;
    cout << "\t\t3.退出"<<endl;
    cout << "请输入您的选择:";
    cin >> choi;
    
    if(choi == 1){
        cout << "请输入ISBN:";
        string ISBN;
        cin >> ISBN;
        string nu("");
        book["ISBN"] = ISBN.c_str();book["book_name"] = nu.c_str();
    }
    if(choi == 2){
        cout << "请输入书籍名称:";
        string book_name;
        cin>>book_name;
        string nu("");
        book["book_name"] = book_name.c_str();book["ISBN"] = nu.c_str();
    }
    if(choi == 3){
        return false;
    }
 
    int chan_what;
    do{
        cout << "\t\t1.修改出版社"<<endl;
        cout << "\t\t2.修改作者"<<endl;
        cout << "\t\t3.修改数量"<<endl;
        cout << "\t\t4.修改是否可借阅的状态"<<endl;
        cout << "\t\t5.退出" << endl;
        cout <<"请输入您的选择:";
        cin >> chan_what;
    }while((chan_what != 1) && (chan_what != 2) && (chan_what != 3) && (chan_what != 4) && (chan_what != 5));
    if(chan_what == 1) chan_publish(client,book);
    if(chan_what == 2) chan_author(client,book);
    if(chan_what == 3) chan_count(client,book);
    if(chan_what == 4) chan_stat(client,book);
    return true;
}

bool chan_books_info(TCPClient &client) //改变书籍信息的函数1
{
    bool non_stop = true;
    string yes_or_no;
    while(non_stop){
        system("clear");
        cout << "是否开始更改书籍[yes/no]:";
        cin >> yes_or_no;
        if(yes_or_no != "yes"){
            cout <<"您所输入的不是yes,更改书籍功能已经退出"<<endl;
            non_stop = false;
            continue;
        }
        isbn_or_name(client);
        cout << "按任意键继续..."<<endl;
        getchar();
        getchar();
    }
}
bool sea_books_info(TCPClient &client)//查询书籍信息
{
    Json::Value book;
    Json::Value book_recv;
    Json::Reader reader;
    string nu("");
    bool non_stop = true;
    book["ISBN"] = nu.c_str();
    book["book_name"] = nu.c_str();
    fill_in_json(book);
    string out = book.toStyledString();
    memcpy(client.data_buffer,out.c_str(),out.size());
    if(client.send_to_serv(out.size(),SEA_BOOKS_ALL_INFO) == false){
        cout << "发送数据失败"<<endl;
        return false;
    }
    cout << "书籍信息如下:"<<endl;
    cout <<"ISBN  书籍名称 出版社   作者   库存  可借阅状态 "<<endl;
    do{
        if(client.recv_from_serv() == false){
            cout << "从客户端接受信息失败"<<endl;
            return false;
        }
        NetPacket *phead = (NetPacket*)client.data_buffer;
        if(phead ->Header. wOpcode == SEA_BOOKS_ALL_INFO_YES){
            //当前接收正确
            string str(phead -> Data);
            if(reader.parse(str,book_recv) < 0){
                cout << "json解析失败"<<endl;
                return false;
            }
            else{
                cout << "|"<<book_recv["ISBN"].asString() <<"\t|"
                << book_recv["book_name"].asString() << "\t|"
                << book_recv["publish_house"].asString() << "\t|"
                << book_recv["author"].asString() << "\t|"
                << book_recv["count"].asString() << "\t|"
                << book_recv["stat"].asString() << endl;
            }
        }
        else if(phead -> Header.wOpcode == SEA_BOOKS_ALL_INFO_NO){
            //当前接收错误
            cout << "读取错误,请稍后再试"<<endl;
            return false;
        }
        else if(phead -> Header.wOpcode == SEA_BOOKS_ALL_INFO){
            //读取完成
            cout<< "已读取完全部数据"<<endl;
            break;
        }
    }while(non_stop);
}
string temp_isbn;    //查询和借阅函数所需要的ISBN
string temp_name;    //查询和借阅函数所需要的书名
string temp_days;    //借阅时间，多少天
bool borrow_book_name(TCPClient &client) //按书名借阅图书
{
    bool non_stop = true;
    Json::Value book;
    string nu("");

    book["ISBN"] = nu.c_str();
    book["book_name"] = temp_name.c_str();
    book["publish_house"] = nu.c_str();
    book["author"] = nu.c_str();
    book["count"] = nu.c_str();
    book["stat"] = nu.c_str();
    book["days"]=temp_days;

    string out = book.toStyledString();
    memcpy(client.data_buffer,out.c_str(),out.size());

    if(client.send_to_serv(out.size(),BOR_BOOK) == false)
    {
        cout << "发送数据失败"<<endl;
        return false;
    }
    if(client.recv_from_serv() == false)
    {
        cout << "接收服务器发来的消息失败"<<endl;
        return false;
    }
    NetPacketHeader *phead = (NetPacketHeader*)client.data_buffer;
    if(phead -> wOpcode == BOR_BOOK_YES)
    {
        cout << "借阅成功"<<endl;
        return true;
    }
    if(phead -> wOpcode == SEA_BOOKS_INFO_NO)
    {
        cout << "书籍不存在,无法借阅"<<endl;
        return false;
    }
    if(phead -> wOpcode == BOR_BOOK_NO)
    {
        cout << "借阅失败"<<endl;
        return false;
    }
  
}

bool borrow_isbn(TCPClient &client)//按ISB号借阅图书
{
    bool non_stop = true;
    while(non_stop)
    {
        Json:: Value book;
        string nu("");
        book["ISBN"] = temp_isbn.c_str();
        book["book_name"] = nu.c_str();
        book["publish_house"] = nu.c_str();
        book["author"] = nu.c_str();
        book["count"] = nu.c_str();
        book["stat"] = nu.c_str();
        book["days"]=temp_days;

        string out = book.toStyledString();
        memcpy(client.data_buffer,out.c_str(),out.size());

        if(client.send_to_serv(out.size(),BOR_BOOK) == false)
        {
            cout << "发送数据失败"<<endl;
            return false;
        }
        if(client.recv_from_serv() == false)
        {
            cout << "接收服务器发来的消息失败"<<endl;
            return false;
        }
        NetPacketHeader *phead = (NetPacketHeader*)client.data_buffer;
        if(phead -> wOpcode == BOR_BOOK_YES)
        {
            cout << "借阅成功"<< endl;
            return true;
        }
        if(phead -> wOpcode == SEA_BOOKS_INFO_NO)
        {
            cout << "书籍不存在,无法借阅" << endl;
            return false;
        }
        if(phead -> wOpcode == BOR_BOOK_NO)
        {
            cout << "借阅失败" << endl;
            return false;
        }
    }
}

bool borrow_book_infor(TCPClient &client)//查阅借阅信息
{
    Json::Value book;
    Json::Value book_recv;
    Json::Reader reader;
    bool non_stop = true;

    if(client.send_to_serv(0,SEA_BOR_BOOK) == false)
    {
        cout << "发送数据失败"<<endl;
        return false;
    }
    cout << "借阅信息如下:"<<endl;
    cout <<"用户名   ISBN    书籍名称　借书日期　  还书日期"<<endl;
    do{
        if(client.recv_from_serv() == false)
        {
            cout << "从服务器接受信息失败"<<endl;
            return false;
        }
        NetPacket *phead = (NetPacket*)client.data_buffer;
        if(phead ->Header. wOpcode == SEA_BOR_BOOK_YES)
        {
            string str(phead -> Data);
            if(reader.parse(str,book_recv) < 0)
            {
                cout << "json解析失败"<<endl;
                return false;
            }
            else
            {
                cout << book_recv["account"].asString() <<"\t"
                << book_recv["ISBN"].asString() << "\t"
                << book_recv["book_name"].asString() << "\t"
                << book_recv["borrow_date"].asString() <<"\t"
                <<book_recv["ret_date"].asString()<<endl;
            }
        }
        else if(phead -> Header.wOpcode == SEA_BOR_BOOK_NO)
        {
            //当前接收错误
            cout << "读取错误,请稍后再试"<<endl;
            return false;
        }
        else if(phead -> Header.wOpcode == SEA_BOR_BOOK)
        {
            //读取完成
            break;
        }
    }while(non_stop);
}

bool borrow_book(TCPClient &client)
{
    bool non_stop = true;
    int temp_number;
    Json::Value book;
    string ISBN,book_name,publish_house,author,count,stat;
    while(non_stop)
    {
            int choi;
            cout << "\t\t1.根据ISBN借阅"<<endl;
            cout << "\t\t2.根据书名借阅"<<endl;
            cout << "\t\t3.查看借阅信息 "<< endl;
            cout << "\t\t4.退出 "<< endl;
            cout<<"\n请输入选择序号:";
            cin >> choi;
            if((choi != 1) && (choi != 2) && (choi != 3) &&(choi != 4))
            {
                cout << "输入有误";
                continue;
            }
            switch(choi)
            {
                case 1:
                    temp_isbn.clear();
                    cout << "请输入ISBN"<<endl;
                    cin >> temp_isbn;
                    cout << "请输入借阅图书的数量"<<endl;
                    cin >> temp_number;
                    cout<<"请输入借阅时间(days):";
                    cin>>temp_days;
                    temp_name = "";
                    for(int i = 1;i<=temp_number;i++)
                        borrow_isbn(client);
                    temp_isbn.clear();
                    break;
                case 2:
                    temp_name.clear();
                    cout << "请输入所要借阅书籍名称:"<<endl;
                    cin >> temp_name;
                    temp_isbn = "";
                    cout << "请输入借阅图书的数量"<<endl;
                    cin >> temp_number;
                    cout<<"请输入借阅时间"<<endl;
                    cin>>temp_days;
                    for(int i = 1;i<=temp_number;i++)
                        borrow_book_name(client);
                    temp_name.clear();
                    break;
                case 3:
                    borrow_book_infor(client);
                    break;
                case 4:cout << "退出"<< endl;
                return true;      
            }
            
    }
   
}
bool ret_book_name(TCPClient &client)//按书名归还
{
    bool non_stop = true;
    Json::Value book;
    string nu("");
    book["ISBN"] = nu.c_str();
    book["book_name"] = temp_name.c_str();
    book["publish_house"] = nu.c_str();
    book["author"] = nu.c_str();
    book["count"] = nu.c_str();
    book["stat"] = nu.c_str();

    string out = book.toStyledString();
    memcpy(client.data_buffer,out.c_str(),out.size());
    if(client.send_to_serv(out.size(),RET_BOOK) == false)
    {
        cout << "发送数据失败"<<endl;
        return false;
    }
    if(client.recv_from_serv() == false)
    {
        cout << "接收服务器发来的消息失败"<<endl;
        return false;
    }
    NetPacketHeader *phead = (NetPacketHeader*)client.data_buffer;
    if(phead -> wOpcode == RET_BOOK_YES)
    {
        cout << "归还成功"<<endl;
        return true;
    }
    if(phead -> wOpcode == SEA_BOOKS_INFO_NO)
    {
        cout << "无此书籍的借阅记录"<<endl;
        return false;
    }
    if(phead -> wOpcode == RET_BOOK_NO)
    {
        cout << "归还失败"<<endl;
        return false;
    }
  
}
bool ret_isbn(TCPClient &client)//按isbn归还
{
    bool non_stop = true;
    while(non_stop)
    {
        Json:: Value book;
        string nu("");
        book["ISBN"] = temp_isbn.c_str();
        book["book_name"] = nu.c_str();
        book["publish_house"] = nu.c_str();
        book["author"] = nu.c_str();
        book["count"] = nu.c_str();
        book["stat"] = nu.c_str();

        string out = book.toStyledString();
        memcpy(client.data_buffer,out.c_str(),out.size());
        if(client.send_to_serv(out.size(),RET_BOOK) == false)
        {
            cout << "发送数据失败"<<endl;
            return false;    
        }
        if(client.recv_from_serv() == false)
        {
            cout << "接收服务器发来的消息失败"<<endl;
            return false;
        }
        NetPacketHeader *phead = (NetPacketHeader*)client.data_buffer;
        if(phead -> wOpcode == RET_BOOK_YES)
        {
            cout << "归还成功"<< endl;
            return true;
        }
        if(phead -> wOpcode == SEA_BOOKS_INFO_NO)
        {
            cout << "无此书籍的借阅记录" << endl;
            return false;
        }
        if(phead -> wOpcode == RET_BOOK_NO)
        {
            cout << "归还失败" << endl;
            return false;
        }
    }
}



bool ret_book_infor(TCPClient &client)//查阅借阅信息
{
    Json::Value book;
    Json::Value book_recv;
    Json::Reader reader;
    bool non_stop = true;
    if(client.send_to_serv(0,SEA_RET_BOOK) == false)
    {
        cout << "发送数据失败"<<endl;
        return false;
    }
    cout << "借阅信息如下:"<<endl;
    cout <<"用户名   ISBN    书籍名称　　归还日期　　　"<<endl;
    do{
        if(client.recv_from_serv() == false)
        {
            cout << "从客户端接受信息失败"<<endl;
            return false;
        }
        NetPacket *phead = (NetPacket*)client.data_buffer;
        if(phead ->Header. wOpcode == SEA_RET_BOOK_YES)
        {
            //当前接收正确
            string str(phead -> Data);
            if(reader.parse(str,book_recv) < 0)
            {
                cout << "json解析失败"<<endl;
                return false;
            }
            else
            {
                cout << book_recv["account"].asString() <<"\t"
                << book_recv["ISBN"].asString() << "\t"
                << book_recv["book_name"].asString() << "\t"
                << book_recv["return_date"].asString() << endl;
            }
        }
        else if(phead -> Header.wOpcode == SEA_RET_BOOK_NO)
        {
            //当前接收错误
            cout << "读取错误,请稍后再试"<<endl;
            return false;
        }
        else if(phead -> Header.wOpcode == SEA_RET_BOOK)
        {
            //读取完成
            break;
        }
    }while(non_stop);
    
}
bool ret_book(TCPClient &client)
{
    bool non_stop = true;
    Json::Value book;
    string ISBN,book_name,publish_house,author,count,stat;
    while(non_stop)
    {
            int choi;
            cout << "\t\t1.根据ISBN归还"<<endl;
            cout << "\t\t2.根据书名归还"<<endl;
            cout << "\t\t3.查看归还信息 "<< endl;
            cout << "\t\t4.退出 "<< endl;
            cout<<"\n请输入选择序号:";
            cin >> choi;
            if((choi != 1) && (choi != 2) && (choi != 3) &&(choi != 4))
            {
                cout << "输入有误";
                continue;
            }
            switch(choi)
            {
                case 1:
                    temp_isbn.clear();
                    cout << "请输入所要归还书籍ISBN"<<endl;
                    cin >> temp_isbn;
                    temp_name = "";
                    ret_isbn(client);   //ISBN归还函数调用
                    temp_isbn.clear();
                    break;
                case 2:
                    temp_name.clear();
                    cout << "请输入所要归还书籍名称:"<<endl;
                    cin >> temp_name;
                    temp_isbn = "";
                    ret_book_name(client);  //书名归还函数调用
                    temp_name.clear();
                    break;
                case 3:
                    ret_book_infor(client);
                    break;
                case 4:
                    cout << "退出"<< endl;
                    return true;      
            }
    }
}

bool search_book(TCPClient &client)   //搜索指定图书
{
    string information;
    string colour_begin("\e[37;44m");
    string colour_end("\e[0m");
    system("clear");
    cout<<"请输入要查询的ISBN/名称/作者/出版社:";
    getchar();
    getline(cin,information);
    Json::Value book;
    Json::Reader reader;
    book["information"]=information;
    string out=book.toStyledString();
    memcpy(client.data_buffer,out.c_str(),out.size());
    if(client.send_to_serv(out.size(),SEARCH_BOOK)==false)
    {
        cout<<"向服务器发送数据失败"<<endl;
        return false;
    }
    
    cout << "\n书籍信息如下:"<<endl;
    cout <<"ISBN\t\t书籍名称\t\t出版社\t\t作者\t\t库存\t\t可借阅状态 "<<endl;
    while(1)
    {
        if(client.recv_from_serv()==false)  //接收数据失败
        {
            cout<<"搜索失败"<<endl;
            return false;
        }
        NetPacketHeader *phead=(NetPacketHeader*)client.data_buffer;
        if(phead->wOpcode==SEARCH_BOOK_YES) 
        {
            string str((char*)(phead+1));
            if(reader.parse(str,book) < 0){
                cout << "搜索失败"<<endl;
                return false;
            }
            else{
                string ISBN=book["ISBN"].asString();
                string book_name=book["book_name"].asString();
                string publish_house=book["publish_house"].asString();
                string author=book["author"].asString();
                string count=book["count"].asString();
                string stat=book["stat"].asString();

                int it;

                if((it=ISBN.find(information))!=string::npos)
                {
                    if(it!=0)
                    ISBN=string(ISBN,0,it)+colour_begin+string(ISBN,it,information.size())+colour_end \
                    +string(ISBN,it+information.size());
                    else
                    ISBN=colour_begin+string(ISBN,0,information.size())+colour_end+\
                    string(ISBN,information.size());

                }

                if((it=book_name.find(information))!=string::npos)
                {
                    if(it!=0)
                    book_name=string(book_name,0,it)+colour_begin+string(book_name,it,information.size())+colour_end \
                    +string(book_name,it+information.size());
                    else
                    book_name=colour_begin+string(book_name,0,information.size())+colour_end+\
                    string(book_name,information.size());

                }

                if((it=publish_house.find(information))!=string::npos)
                {
                    if(it!=0)
                    publish_house=string(publish_house,0,it)+colour_begin+string(publish_house,it,information.size())\
                    +colour_end+string(publish_house,it+information.size());
                    else
                    publish_house=colour_begin+string(publish_house,0,information.size())+colour_end+\
                    string(publish_house,information.size());

                }

                if((it=author.find(information))!=string::npos)
                {
                    if(it!=0)
                    author=string(author,0,it)+colour_begin+string(author,it,information.size())+colour_end \
                    +string(author,it+information.size());
                    else
                    author=colour_begin+string(author,0,information.size())+colour_end+\
                    string(author,information.size());

                }
                cout << "|"<<ISBN<<"\t|"
                << book_name << "\t|"
                << publish_house << "\t|"
                << author << "\t|"
                << count << "\t|"
                <<stat << endl;
            }
        }
        else if(phead->wOpcode==SEARCH_BOOK_NO)
        {
            cout<<"搜索失败"<<endl;
            return false;
        }
        else if(phead->wOpcode==SEARCH_BOOK_END)
        {
            break;
        }        
    }
    return true;
 
}



void Admin_menu(TCPClient &client) //管理员菜单
{
    int choice;
    bool non_stop = true;
    while (non_stop) {
        system("clear");
        cout << "\t\t欢迎进入管理员功能界面"<<endl;
        cout << "\t\t1.上线图书"<< endl;
        cout << "\t\t2.下线图书"<<endl;
        cout << "\t\t3.更改图书信息"<< endl;
        cout << "\t\t4.查看所有图书"<<endl;
        cout << "\t\t5.搜索图书(ISBN/名称/作者/出版社)" << endl;
        cout << "\t\t6.退出" << endl;
        cout << "请输入您的选择:";
        cin >> choice;

        switch (choice)
        {
            case 1:add_books_info(client); break;
            case 2:del_books_info(client); break;
            case 3:chan_books_info(client);break;
            case 4:sea_books_info(client);break;
            case 5:search_book(client);break;
            case 6: {cout<<"Bye"<<endl;     non_stop = false ; exit(0);}
        }
        cout << "\n按任意键继续..."<< endl;
        getchar();
        getchar();
    }
}


void menu(TCPClient &client) 
{
    int choice;
    while (1) {
        system("clear");
        cout<<"\t\t\t欢迎进入功能界面:"<<endl;
        cout<<"\t\t\t1.查询个人资料"<<endl;
        cout<<"\t\t\t2.查询所有书籍信息"<<endl;
        cout<<"\t\t\t3.借阅图书"<<endl;
        cout<<"\t\t\t4.归还图书"<<endl;
        cout<<"\t\t\t5.搜索图书(ISBN/名称/作者/出版社)" << endl;
        cout<<"\t\t\t6.还书提醒"
        cout<<"\t\t\t7.聊天"<<endl;
        cout<<"\t\t\t8.退出"<<endl;
        cout<<"\n请输入选择序号:";
        cin >> choice;

        switch (choice)
        {
            case 1: Personal_data(client); break;
            case 2:sea_books_info(client);break;
            case 3: borrow_book(client); break;
            case 4: ret_book(client); break;
            case 5: search_book(client); break;
            case 6:break;
            case 7:break;
            case 8: cout<<"Bye"<<endl;      exit(1);
        }
        cout << "\n\n按任意键返回菜单" << endl;
        getchar();
        getchar();
    }
}

void TCPClient::run(TCPClient& client)
{
    Login_Register(client);  //登录注册界面

}

int main(int argc ,char **argv)
{

    TCPClient client(argc,argv);


    client.run(client);

}
