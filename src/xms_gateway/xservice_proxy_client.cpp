#include "xservice_proxy_client.h"
#include "xtools.h"
#include "xauth_proxy.h"
#include "xlog_client.h"
using namespace std;

XServiceProxyClient* XServiceProxyClient::Create(std::string service_name)
{
    if (service_name == AUTH_NAME)
    {
        return new XAuthProxy();
    }
    return new XServiceProxyClient();
}

bool XServiceProxyClient::SendMsg(xmsg::XMsgHead *head, XMsg *msg, XMsgEvent *ev)
{
    RegEvent(ev);
    head->set_msg_id((long long)ev);
    return XMsgEvent::SendMsg(head, msg);
}
void XServiceProxyClient::DelEvent(XMsgEvent *ev)
{
    XMutex mux(&callback_task_mutex_);
    callback_task_.erase((long long)ev);
}
//注册一个事件
void XServiceProxyClient::RegEvent(XMsgEvent *ev)
{
    XMutex mux(&callback_task_mutex_);
    callback_task_[(long long)ev] = ev;
}
void XServiceProxyClient::ReadCB(xmsg::XMsgHead *head, XMsg *msg)
{
    if (!head || !msg)return;


    cout << "***************************************" << endl;
    cout << head->DebugString();
    //XMutex mux(&callback_task_mutex_);
    //转发给XRouterHandle
    //每个XServiceProxyClient对象可能管理多个XRouterHandle
    auto router = callback_task_.find(head->msg_id());
    if (router == callback_task_.end())
    {
        LOGDEBUG("callback_task_ can't find");
        return; 
    }
    // 多线程问题？？ 通过 锁 解决
    router->second->SendMsg(head, msg);
}

XServiceProxyClient::XServiceProxyClient()
{
}


XServiceProxyClient::~XServiceProxyClient()
{
}
