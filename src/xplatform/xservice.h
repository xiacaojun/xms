#pragma once
#include "xtask.h"
#include "xservice_handle.h"
#include "xthread_pool.h"
class XCOM_API XService :public XTask
{
public:
    XService();
    ~XService();

    //需要重载，每个连接进入，调用此函数创建处理对象，加入到线程池
    virtual XServiceHandle* CreateServiceHandle() = 0;
    
    ///服务初始化 由线程池调用
    bool Init();

    ///开始服务运行，接收连接任务加入到线程池
    bool Start();

    //服务器监听端口
    void set_server_port(int port) { this->server_port_ = port; }

    //接入连接的入口文件
    void ListenCB(int client_socket, struct sockaddr *addr, int socklen);

    //等待线程池退出
    void Wait();

private:
    
    //接收用户连接的线程池
    XThreadPool *thread_listen_pool_ = 0;

    //处理用户的数据的连接池
    XThreadPool *thread_client_pool_ = 0;

    //客户数据处理的线程数量
    int thread_count_ = 10;

    //服务器监听端口
    int server_port_ = 0;


};

