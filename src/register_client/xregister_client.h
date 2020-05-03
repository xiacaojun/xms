#ifndef XREGISTER_CLIENT_H
#define XREGISTER_CLIENT_H
#include "xservice_client.h"

////////////////////////////////////////////////
//// 注册中心客户端 在windows当中直接引用文件
class XRegisterClient:public XServiceClient
{
public:
    static XRegisterClient *Get()
    {
        static XRegisterClient *xc = 0;
        if (!xc)
        {
            xc = new XRegisterClient();
        }
        return xc;
    }

    ~XRegisterClient();

    //连接成功的消息回调，由业务类重载
    virtual void ConnectedCB();


    //接收服务的注册响应
    void RegisterRes(xmsg::XMsgHead *head, XMsg *msg);

    //获取服务列表的响应
    void GetServiceRes(xmsg::XMsgHead *head, XMsg *msg);

    static void RegMsgCallback()
    {
        RegCB(xmsg::MSG_REGISTER_RES, (MsgCBFunc)&XRegisterClient::RegisterRes);
        RegCB(xmsg::MSG_GET_SERVICE_RES, (MsgCBFunc)&XRegisterClient::GetServiceRes);
    }

    ///////////////////////////////////////////////////////////////////////
    //// 向注册中心注册服务 此函数，需要第一个调用，建立连接
    /// @para service_name 微服务名称
    /// @para port 微服务接口
    /// @para ip 微服务的ip，如果传递NULL，则采用客户端连接地址
    /// @para is_find 是否可以为外网发现
    void RegisterServer(const char *service_name, int port, const char *ip, bool is_find = false);

    /// 获取所有的服务列表，复制原数据，每次清理上次的复制数据
    /// 此函数和操作XServiceMap数据的函数在一个线程
    xmsg::XServiceMap *GetAllService();


    /////////////////////////////////////////////////////////////////////////////
    /// 获取指定服务名称的微服务列表 （阻塞函数）
    /// 1 等待连接成功 2 发送获取微服务的消息 3 等待微服务列表消息反馈（有可能拿到上一次的配置）
    /// @para service_name 服务名称
    /// @para timeout_sec 超时时间
    /// @return 服务列表
    xmsg::XServiceList GetServcies(const char *service_name, int timeout_sec);

    /////////////////////////////////////////////////////////////
    ///发出有获取微服务列表的请求
    ///@para service_name == NULL 则取全部
    void GetServiceReq(const char *service_name);

    //定时器，用于发送心跳
    virtual void TimerCB();
private:
    XRegisterClient();

    //读取本地缓存 线程不安全，需要对service_map加锁
    bool LoadLocalFile();

    char service_name_[32] = {0};
    int service_port_ = 0;
    char service_ip_[16] = {0};

    //是否可以被外网发现
    bool is_find_ = false;


};

#endif