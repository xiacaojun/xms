#ifndef XSERVICEPROXY_H
#define XSERVICEPROXY_H
#include <map>
#include <vector>
#include <string>
#include "xservice_proxy_client.h"
class XServiceProxy
{
public:
    static XServiceProxy *Get()
    {
        static XServiceProxy xs;
        return &xs;
    }
    XServiceProxy();
    ~XServiceProxy();
    ///初始化微服务列表（注册中心获取），建立连接
    bool Init();

    //负载均衡找到客户端连接，进行数据发送
    bool SendMsg(xmsg::XMsgHead *head, XMsg *msg,XMsgEvent *ev);

    //清理消息回调
    void DelEvent(XMsgEvent *ev);


    //开启自动重连的线程
    void Start();

    //停止线程
    void Stop();

    void Main();

private:

    bool is_exit_ = false;
    //存放与各个微服务的连接对象
    std::map < std::string, std::vector<XServiceProxyClient *>> client_map_;

    std::mutex client_map_mutex_;
    //记录上次轮询的索引
    std::map<std::string, int>client_map_last_index_;


    //用于清理callback缓冲
    std::map<XMsgEvent*, XServiceProxyClient*> callbacks_;
    std::mutex callbacks_mutex_;


};

#endif