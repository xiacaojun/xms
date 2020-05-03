#include "xservice.h"
#include "event2/bufferevent.h"
#include "event2/listener.h"
#include "xservice_handle.h"
#include "xlog_client.h"
#include "xtools.h"
#include <sstream>
using namespace std;
static void SListenCB(struct evconnlistener *ev, evutil_socket_t sock, struct sockaddr *addr, int socklen, void *arg)
{
    LOGDEBUG("SListenCB");
    auto task = (XService*)arg;
    task->ListenCB(sock, addr, socklen);

}
void XService::Wait()
{ 
    XThreadPool::Wait();
}
void XService::ListenCB(int client_socket, struct sockaddr *client_addr, int socklen)
{
    //创建客户端处理对象
    auto handle = CreateServiceHandle();
    handle->set_sock(client_socket);
    handle->set_ssl_ctx(ssl_ctx());

    stringstream ss;
    char ip[16] = { 0 };
    auto addr = (sockaddr_in *)client_addr;
    evutil_inet_ntop(AF_INET, &addr->sin_addr.s_addr, ip, sizeof(ip));
    int client_port = ntohs(addr->sin_port);
    ss << "accept client ip :" << ip << " port:" << client_port << endl;
    LOGINFO(ss.str().c_str());

    
    //任务加入到线程池
    handle->set_client_ip(ip);
    handle->set_client_port(client_port);
    thread_client_pool_->Dispatch(handle);
}
///服务初始化 由线程池调用
bool XService::Init()
{
    if (server_port_ <= 0)
    {
        LOGERROR("server_port_ not set!");
        return false;
    }
    //绑定端口
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(server_port_);
    auto evc = evconnlistener_new_bind(base(), SListenCB, this,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
        10,  ///listen back
        (sockaddr*)&sin,
        sizeof(sin)
    );
    if (!evc)
    {
        stringstream ss;
        ss << "listen port " << server_port_ << " failed!" << endl;
        LOGERROR(ss.str().c_str());
        return false;
    }
    stringstream ss;
    ss << "listen port " << server_port_ << " success!" << endl;
    LOGINFO(ss.str().c_str());
    return true;
}

///开始服务运行，接收连接任务加入到线程池
bool XService::Start()
{
    thread_listen_pool_->Init(1);
    thread_client_pool_->Init(thread_count_);
    thread_listen_pool_->Dispatch(this);
    return true;
}

XService::XService()
{
    this->thread_client_pool_ = XThreadPoolFactory::Create();
    this->thread_listen_pool_ = XThreadPoolFactory::Create();
}


XService::~XService()
{
    delete thread_client_pool_;
    thread_client_pool_ = NULL;
    delete thread_listen_pool_;
    thread_listen_pool_ = NULL;
}
