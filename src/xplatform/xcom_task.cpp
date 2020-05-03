#include "xcom_task.h"

#include "xlog_client.h"
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <iostream>
#include <string.h>
#include "xtools.h"
#include "event2/bufferevent_ssl.h"
#include <chrono>
#include <thread>
#include "xssl.h"
using namespace std;

XCOM_API const char * XGetPortName(unsigned short port)
{
    switch (port)
    {
    case API_GATEWAY_PORT:
        return API_GATEWAY_NAME;
        break;
    case REGISTER_PORT:
        return REGISTER_NAME;
        break;
    case CONFIG_PORT:
        return AUTH_NAME;
        break;
    case XLOG_PORT:
        return XLOG_NAME;
        break;
    case DOWNLOAD_PORT:
        return DOWNLOAD_NAME;
        break;
    case DIR_PORT:
        return DIR_NAME;
        break;
    case UPLOAD_PORT:
        return UPLOAD_NAME;
        break;
    default:
        break;
    }
    return "";
}

static void SReadCB(struct bufferevent *bev, void *ctx)
{
    auto task = (XComTask*)ctx;
    task->ReadCB();
    //static int i = 0;
    //i++;
    //cout << "{"<<i<<"}" << flush;
}
static void SWriteCB(struct bufferevent *bev, void *ctx)
{
    auto task = (XComTask*)ctx;
    task->WriteCB();
}

void SAutoConnectTimerCB(evutil_socket_t s, short w, void *ctx)
{
    auto task = (XComTask*)ctx;
    task->AutoConnectTimerCB();
}
void STimerCB(evutil_socket_t s, short w, void *ctx)
{
    auto task = (XComTask*)ctx;
    task->TimerCB();
}
static void SEventCB(struct bufferevent *bev, short what,void *ctx)
{
    auto task = (XComTask*)ctx;
    task->EventCB(what);
}

XComTask::XComTask()
{
    mux_ = new mutex;
}
XComTask::~XComTask()
{
    // 清理mutex需要考虑，如果在对象delete 后还有线程在lock
    // 目前只有通信服务端会清理对象，客户端会自动重连，所以不会清理对象
    // 服务端单个任务在线程池中处理，清理关闭后，不会再用此互斥
    // 但需要考虑网关的处理
    delete mux_;
    mux_ = NULL;
}
//设置错误，线程不安全
void XComTask::set_error(const char * err)
{
    if (!err)return;
    strncpy(error_, err, sizeof(error_));
}
void XComTask::set_client_ip(const char*ip)
{
    if (!ip)return;
    strncpy(client_ip_, ip, sizeof(client_ip_));
}

//////////////////////////////////////////////////////////////
/// 设定自动重连的定时器
void XComTask::SetAutoConnectTimer(int ms)
{
    if (!base())
    {
        LOGERROR("SetAutoConnectTimer failed : base not set!");
        return;
    }
    if(auto_connect_timer_event_)
    {
        event_free(auto_connect_timer_event_);
        auto_connect_timer_event_ = 0;
    }

    auto_connect_timer_event_ = event_new(base(), -1, EV_PERSIST, SAutoConnectTimerCB, this);
    if (!auto_connect_timer_event_)
    {
        LOGERROR("SetAutoConnectTimer  failed :event_new faield!");
        return;
    }
    int sec = ms / 1000; //秒
    int us = (ms % 1000) * 1000;//微妙
    timeval tv = { sec,us };
    event_add(auto_connect_timer_event_, &tv);
}
    
/////////////////////////////////////////
///自动重连定时器回调函数
void XComTask::AutoConnectTimerCB()
{
     //如果正在连接，则等待，如果没有，则开始连接
    if (is_connected())
        return ;
    if (!is_connecting())
    {
        Connect();
        cout << "." << flush;
    }
}

//////////////////////////////////////////////////////////////
///设定定时器 只能设置一个定时器 定时调用TimerCB()回调
/// 在Init函数中调用
///@para ms 定时调用的毫秒
void XComTask::SetTimer(int ms)
{
    if (!base())
    {
        LOGERROR("SetTimer failed : base not set!");
        return;
    }

    timer_event_ = event_new(base(), -1, EV_PERSIST, STimerCB, this);
    if (!timer_event_)
    {
        LOGERROR("set timer failed :event_new faield!");
        return;
    }
    int sec = ms / 1000; //秒
    int us = (ms % 1000) * 1000;//微妙
    timeval tv = { sec,us };
    event_add(timer_event_, &tv);
}
void XComTask::set_local_ip(const char *ip)
{
    strncpy(this->local_ip_, ip, sizeof(local_ip_));
}

void XComTask::set_server_ip(const char* ip)
{
    strncpy(this->server_ip_, ip, sizeof(server_ip_));
}

int XComTask::Read(void *data, int datasize)
{
    if (!bev_)
    {
        LOGERROR("bev not set");
        return 0;
    }
    int re = bufferevent_read(bev_, data, datasize);
    if(re>0)
        recv_data_size_ += re;
    return re;
}

///清理所有定时器
void XComTask::ClearTimer()
{
    if(auto_connect_timer_event_)
        event_free(auto_connect_timer_event_);
    auto_connect_timer_event_ = 0;
    if(timer_event_)
        event_free(timer_event_);
    timer_event_ = 0;
}

void XComTask::Close()
{
    {
        XMutex mux(mux_);
        is_connected_ = false;
        is_connecting_ = false;
        is_closed_ = true;
        if (bev_)
        {
            //如果设置了 BEV_OPT_CLOSE_ON_FREE会释放ssl  和socket
            //不一定能释放所有占用空间，任务列表中的释放不了通过event_base_loop(base, EVLOOP_NONBLOCK);取出
            bufferevent_free(bev_);
            bev_ = NULL;
        }

        if (msg_.data)
            delete msg_.data;
        memset(&msg_, 0, sizeof(msg_));
        //if (ssl_ctx_) 设置者自己清理
        //{
        //    ssl_ctx_->Close();
        //    delete ssl_ctx_;
        //    ssl_ctx_ = 0;
        //}
    }
    //清理连接对象空间，如果断开重连，需要单独处理
    if(auto_delete_)
    {
        //清理定时器
        ClearTimer();
        delete this;
    }
        
}

//现有缓冲（未发送）的大小
long long XComTask::BufferSize()
{
    XMutex mux(mux_);
    if (!bev_ )return 0;
    auto evbuf = bufferevent_get_output(bev_);
    auto len = evbuffer_get_length(evbuf);
    return  len;
}

bool XComTask::Write(const void *data, int size)
{
    XMutex mux(mux_);
    if (!bev_  || !data || size <= 0)return false;
    int re = bufferevent_write(bev_, data, size);
    if (re != 0)return false;
    send_data_size_ += size;
    return true;
}

void XComTask::EventCB(short what)
{
    stringstream ss;
    ss << "SEventCB:" << what ;
    
    if (what & BEV_EVENT_CONNECTED)
    {
        //ss << "BEV_EVENT_CONNECTED " ;
        stringstream ss;
        ss << "connnect server " << server_ip_ << ":" << server_port_ << " " << XGetPortName(server_port_) << " success!";
        LOGINFO(ss.str().c_str());
        //通知连接成功
        is_connected_ = true;
        is_connecting_ = false;
        auto ssl = bufferevent_openssl_get_ssl(bev_);
        if (ssl)
        {
            XSSL xssl;
            xssl.set_ssl(ssl);
            xssl.PrintCert();
            xssl.PrintCipher();
        }
        //获取本地地址
        //int sock = bufferevent_getfd(bev_);
        /*if (sock > 0)
        {
            sockaddr_in sin;
            memset(&sin, 0, sizeof(sin));
            int len = sizeof(sin);
            getsockname(sock, (sockaddr*)&sin, &len);
            char buf[16] = { 0 };
            evutil_inet_ntop(AF_INET, &sin.sin_addr.s_addr, buf, sizeof(buf));
            cout << "client ip is " << buf << endl;
        }*/
        ConnectedCB();
    }

    ///退出要处理缓冲内容
    if (what & BEV_EVENT_ERROR )
    {
        auto ssl = bufferevent_openssl_get_ssl(bev_);
        if (ssl)
        {
            XSSL xssl;
            xssl.set_ssl(ssl);
            xssl.PrintCert();
        }
        ss << "BEV_EVENT_ERROR " ;
        int sock = bufferevent_getfd(bev_);
        int err = evutil_socket_geterror(sock);
        //err = evutil_socket_geterror(sock);
        //LOGDEBUG(server_ip());
        //stringstream log;
        ss<<server_ip()<<":"<<server_port()<<" " << XGetPortName(server_port());
        ss<<local_ip()<<evutil_socket_error_to_string(err);
        //LOGDEBUG(log.str().c_str());
        LOGINFO(ss.str().c_str());
        has_error_ = true;
        strcpy(error_, evutil_socket_error_to_string(err));
        Close();
    }
    if (what & BEV_EVENT_TIMEOUT)
    {
        ss << "BEV_EVENT_TIMEOUT" ;
        LOGINFO(ss.str().c_str());
        has_error_ = true;
        strcpy(error_, "BEV_EVENT_TIMEOUT");
        Close();
    }
    if (what & BEV_EVENT_EOF)
    {
        ss << "BEV_EVENT_EOF" ;
        LOGINFO(ss.str().c_str());
        Close();
    }
}

bool XComTask::Connect()
{
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(server_port_);
    evutil_inet_pton(AF_INET, server_ip_, &sin.sin_addr.s_addr);
    XMutex mux(mux_);
    is_connected_ = false;
    is_connecting_ = false;
    
    //初始化bufferevent
    if(!bev_)InitBev(-1);
    if (!bev_)
    {
        LOGERROR("XComTask::Connect failed! bev is null!");
        return false;
    }
    int re = bufferevent_socket_connect(bev_, (sockaddr*)&sin, sizeof(sin));
    if (re != 0)
    {
        return false;
    }

    //开始连接
    is_connecting_ = true;
    return true;
}

bool XComTask::InitBev(int sock)
{
    //mux_->lock();
    ///用bufferevent 建立连接 
    /// 创建bufferevent上下文 -1自动创建socket
    if (!ssl_ctx())
    {
        bev_ = bufferevent_socket_new(base(), sock, BEV_OPT_CLOSE_ON_FREE);
        if (!bev_)
        {
            //mux_->unlock();
            LOGERROR("bufferevent_socket_new failed!");
            return false;
        }
    }
    else// 加密通信
    {
        auto xssl = ssl_ctx()->NewXSSL(sock);
        //客户端
        if (sock < 0)
        {
            bev_ = bufferevent_openssl_socket_new(base(), sock, xssl.ssl()
                , BUFFEREVENT_SSL_CONNECTING,
                BEV_OPT_CLOSE_ON_FREE// bufferevent_free会同时关闭socket和ssl
            );
        }
        else
        {//服务端
            bev_ = bufferevent_openssl_socket_new(base(), sock, xssl.ssl()
                , BUFFEREVENT_SSL_ACCEPTING,
                BEV_OPT_CLOSE_ON_FREE// bufferevent_free会同时关闭socket和ssl
            );
        }
        if (!bev_)
        {
            //mux_->unlock();
            LOGERROR("bufferevent_openssl_socket_new failed!");
            return false;
        }
    }

    //设定读超时时间
    if (read_timeout_ms_ > 0)
    {
        //秒，微妙
        timeval read_tv = { read_timeout_ms_ / 1000,
            (read_timeout_ms_ % 1000) * 1000 };
        bufferevent_set_timeouts(bev_, &read_tv, 0);
    }

    //定时器设定
    if (timer_ms_ > 0)
    {
        SetTimer(timer_ms_);
    }

    //设置回调函数
    bufferevent_setcb(bev_, SReadCB, SWriteCB, SEventCB, this);
    bufferevent_enable(bev_, EV_READ | EV_WRITE);
    //mux_->unlock();
    return true;
}

//添加到任务列表调用
bool XComTask::Init()
{
    //区分服务端还是客户端
    int comsock = this->sock();
    if (comsock <= 0)
        comsock = -1;
    {
        XMutex mux(mux_);
        InitBev(comsock);
    }
    //timeval tv = { 10,0 };
    //bufferevent_set_timeouts(bev_, &tv, &tv);

    //连接服务器
    if (server_ip_[0] == '\0')
    {
        return true;
    }
    
    //断开三秒自动重连
    SetAutoConnectTimer(3000);

    //客户端
    return Connect();
}

//////////////////////////////////////////////////////
///建立连接，如果断开，会再次重连，知道连接成功，或者超时
bool XComTask::AutoConnect(int timeout_sec)
{
    //如果正在连接，则等待，如果没有，则开始连接
    if (is_connected())
        return true;
    if (!is_connecting())
        Connect();
    return WaitConnected(timeout_sec);
}
//////////////////////////////////////////////////////////////
///等待连接成功
///@para timeout_sec 最大等待时间
bool  XComTask::WaitConnected(int timeout_sec)
{
    //10毫秒监听一次
    int count = timeout_sec * 100;
    for (int i = 0; i < count; i++)
    {
        if (is_connected())
            return true;
        this_thread::sleep_for(chrono::milliseconds(10));
    }
    return is_connected();
}
