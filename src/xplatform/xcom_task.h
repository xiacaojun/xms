#ifndef XCOM_TASK_H
#define XCOM_TASK_H
#include "xtask.h"
#include "xmsg.h"
#include <string>
#include "xssl_ctx.h"

XCOM_API const char * XGetPortName(unsigned short port);

class XSSLCtx;

class XCOM_API XComTask : public XTask
{
public:

    XComTask();

    virtual ~XComTask();
    
    /////////////////////////////////////////////////////////////////
    /// 开始连接服务器，调用成员 server_ip_ server_port_
    /// 要先 通过set_server_ip set_server_port 设定IP和端口
    /// set_auto_connect 设定是否自动重连
    /// @return bev环境未处理，返回false，返回true不代表连接成功,表示连接任务加入成功
    virtual bool Connect();

    /////////////////////////////////////////////////////////////////
    ///初始化bufferevent，客户端建立连接
    ///添加到线程池任务列表调用，包括客户端和服务端
    /// @return 服务端返回true，客户端调用Connect并返回
    virtual bool Init();


    /////////////////////////////////////////////////////////////////
    /// 清理资源，对象空间释放根据auto_delete_
    /// 如果设置了auto_delete_ 会delete当前对象，并清理定时器
    /// 如果未设置auto_delete_ 一般用于自动重连的客户端，知情理bev和缓冲数据
    virtual void Close();



    ///设定和获取服务器的IP 用于连接服务器的客户端
    void set_server_ip(const char* ip);
    const char *server_ip() { return server_ip_; }
    
    ///设定和获取服务器的Port 用于连接服务器的客户端
    void set_server_port(int port) { this->server_port_ = port; }
    int server_port() { return this->server_port_; }
    
    //本地IP用于获取配置项
    // 客户端在连接成功后设置 不是服务端的接收连接的客户端IP（client_ip()）
    void set_local_ip(const char *ip);
    const char *local_ip() { return local_ip_; };


    ///事件回调函数
    virtual void EventCB(short what);

    ////////////////////////////////////////////////////////////
    /// 读取bev中的数据
    /// @para data 读取用缓冲
    /// @para size 缓冲空间大小（字节数）
    /// @return 返回读取的数据的字节数
    int Read(void *data, int datasize);

    ////////////////////////////////////////////////////////////
    /// 发送消息
    /// @para data 数据
    /// @para size 数据大小（字节数）
    /// @return 返回true表示bev写入缓冲成功，不能保证发送成功
    virtual bool Write(const void *data, int size);

    ///现有缓冲（未发送）的大小
    virtual long long BufferSize();

    ///连接成功的消息回调，由业务类重载
    virtual void ConnectedCB() {};

    /////////////////////////////////////////////////////////
    ///当关闭消息接收时，数据将发送到此函数，由业务模块重载
    virtual void ReadCB(void *data, int size) {}

    ///写入数据回调函数
    virtual void WriteCB() {};

    ////////////////////////////////////////
    /// 当缓冲有数据是会调用此纯虚函数
    /// 读取数据回调函数
    /// 需要用户重载，用户在重载函数中调用 Read函数读取数据
    virtual void ReadCB() = 0;


    //////////////////////////////////////////////////////////////
    ///等待连接成功
    ///@para timeout_sec 最大等待时间
    bool WaitConnected(int timeout_sec);

    //////////////////////////////////////////////////////
    ///建立连接，如果断开，会再次重连，知道连接成功，或者超时
    bool AutoConnect(int timeout_sec);

    ///是否正在连接
    bool is_connecting() { return is_connecting_; }

    ///是否连接成功
    bool is_connected() { return is_connected_; }

    //连接断开是否自动清理对象 包含清理定时器
    void set_auto_delete(bool is) { auto_delete_ = is; }

    //是否自动重连 默认不自动 要在添加到线程池之前做
    //设置自动重连 对象就不能自动清理
    void set_auto_connect(bool is) 
    {
        auto_connect_ = is;
        if(is)//自动重连 对象就不能自动清理
            auto_delete_ = false;
    }

    //////////////////////////////////////////////////////////////
    ///设定定时器 只能设置一个定时器 定时调用TimerCB()回调
    /// 在Init函数中调用 在连接建立前调用无效，可以通过set_timer_ms来调用设置
    ///@para ms 定时调用的毫秒
    virtual void SetTimer(int ms);

    ///清理所有定时器
    virtual void ClearTimer();
    
    /////////////////////////////////////////
    ///定时器回调函数
    virtual void TimerCB() {}

    //////////////////////////////////////////////////////////////
    /// 设定自动重连的定时器
    virtual void SetAutoConnectTimer(int ms);

    /////////////////////////////////////////
    ///自动重连定时器回调函数
    virtual void AutoConnectTimerCB() ;

    ///设定要在加入线程池之前
    void set_read_timeout_ms(int ms) { read_timeout_ms_ = ms; }
    
    //设定要在加入线程池之前 virtual void TimerCB() {}
    void set_timer_ms(int ms) { timer_ms_ = ms; }

    ///服务器接收到连接，客户端的IP和端口
    void set_client_ip(const char*ip);
    const char *client_ip() { return client_ip_; }
    const int client_port() { return client_port_; }
    void set_client_port(int port) { this->client_port_ = port; }


    bool is_closed() { return is_closed_; } //是否已关闭，用于非自动连接，不请求空间

    ///是否有错误
    bool has_error() { return has_error_; }  //是否有出错
    
    ///出错原因 线程不安全
    const char *error() { return error_; };  
    
    // 已经写入缓冲 （XMsg *msg ）的字节大小 不包含消息头
    long long send_data_size() { return send_data_size_; }

    // 已经接收的字节大小
    long long recv_data_size() { return recv_data_size_; }

protected:

    //设置错误，线程不安全
    void set_error(const char * err);

    ///服务器接收到连接，存放客户端的IP
    char client_ip_[16] = { 0 };

    ///服务器接收到连接，存放客户端的Port
    int client_port_ = 0;

    //读取缓存
    char read_buf_[4096] = { 0 };

    //本地IP用于获取配置项
    char local_ip_[16] = { 0 };
    
    //XSSL *ssl_ = 0;
   // struct ssl_st *ssl_ = 0;
private:

    // 已经写入缓冲 （XMsg *msg ）的字节大小
    long long send_data_size_ = 0;

    // 已经接收的字节大小
    long long recv_data_size_ = 0;

    //TimerCB 定时调用时间
    int timer_ms_ = 0;

    //读超时时间，毫秒
    int read_timeout_ms_ = 0;

    //是否自动重连
    bool auto_connect_ = false;

    //定时器清理 close时不清理
    struct event * auto_connect_timer_event_ = 0;

    //连接断开是否自动清理对象
    bool auto_delete_ = true;

    /// 初始化 bev的上下文，包括ssl初始化
    bool InitBev(int sock);

    /// 服务器IP
    char server_ip_[16] = {0};

    ///服务器端口
    int server_port_ = 0;

    struct bufferevent *bev_ = 0;

    //数据包缓存
    XMsg msg_;

    //客户单的连接状态 
    //1 未处理  => 开始连接 （加入到线程池处理）
    //2 连接中 => 等待连接成功
    //3 已连接 => 做业务操作
    //4 连接后失败 => 根据连接间隔时间，开始连接
    bool is_connecting_ = true; //连接中
    bool is_connected_ = false;
    bool is_closed_ = false;    //是否关闭了，只有非自动重连才用到此参数
    bool has_error_ = false;     //是否有出错
    char error_[1024] = { 0 };  //出错原因
    std::mutex * mux_ = 0;

    //定时器清理 close时不清理
    struct event * timer_event_ = 0;
};

#endif