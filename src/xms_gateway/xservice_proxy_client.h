#ifndef XSERVICEPROXYCLIENT
#define XSERVICEPROXYCLIENT
#include "xservice_client.h"
#include <map>


class XServiceProxyClient :public XServiceClient
{
public:

    static XServiceProxyClient* Create(std::string service_name);

    ~XServiceProxyClient();
    virtual void ReadCB(xmsg::XMsgHead *head, XMsg *msg);

    //发送数据，添加标识
    virtual bool SendMsg(xmsg::XMsgHead *head, XMsg *msg, XMsgEvent *ev);

    //注册一个事件
    void RegEvent(XMsgEvent *ev);
    void DelEvent(XMsgEvent *ev);

    //是否可以被外网发现
    bool is_find() { return is_find_; }
    void set_is_find(bool is) { this->is_find_ = is; }
protected:

    XServiceProxyClient();
    bool is_find_ = false;
         
    //消息转发的对象，一个proxy对应多个XMsgEvent
    //用指针的值作为索引，要兼容64位
    std::map<long long, XMsgEvent *> callback_task_;
    std::mutex callback_task_mutex_;
};

#endif

