#include "xservice_client.h"
#include "xlog_client.h"
#include <string>
#include "xtools.h"
using namespace xmsg;
using namespace std;

XServiceClient::~XServiceClient()
{
    XMUTEX(&login_mutex_);
    delete login_;
    login_ = NULL;
    //delete thread_pool_;
    //thread_pool_ = NULL;
}

void XServiceClient::set_login(xmsg::XLoginRes *login)
{
    XMUTEX(&login_mutex_);
    if (!login_)
    {
        login_ = new XLoginRes();
    }
    login_->CopyFrom(*login);
}
xmsg::XMsgHead *XServiceClient::SetHead(xmsg::XMsgHead *head)
{
    if (!head)
        return NULL;
    if (service_name_.empty())
    {
        LOGDEBUG("service_name_ is empty");
    }
    else if(head->service_name().empty())
    {
        head->set_service_name(service_name_);
    }
    XMUTEX(&login_mutex_);
    if (!login_)
        return head;

    //需要考虑登录信息
    if (!login_->token().empty())
    {
        //LOGINFO(login_.DebugString());
        string token = login_->token();
        //char *tmp = new char[token.size() + 1];
        //strcpy(tmp, token.c_str());
        //string *t = new string;
        //*t = token;

        //head->set_allocated_token(t);
        head->set_token(token);
        head->set_username(login_->username().c_str());
        head->set_rolename(login_->rolename().c_str());
        //LOGINFO(head->DebugString());
    }
    return head;
}
//获取添加了服务名称和登录信息的head
//xmsg::XMsgHead XServiceClient::GetHeadByType(MsgType type)
//{
//    XMsgHead head;
//    //SetHead(&head);
//    return head;
//}
    //发送消息
bool XServiceClient::SendMsg(xmsg::XMsgHead *h, const google::protobuf::Message *message)
{
    XMsgHead head;
    head.CopyFrom(*h);
    SetHead(&head);
    return XMsgEvent::SendMsg(&head, message);
}
bool XServiceClient::SendMsg(MsgType type, const google::protobuf::Message *message)
{
    //XMsgHead head;
    //head.set_msg_type((MsgType)type);
    ////需要考虑登录信息
    //if (!login_.token().empty())
    //{
    //    head.set_token(login_.token());
    //    head.set_username(login_.username());
    //    head.set_rolename(login_.rolename());
    //}
    //if (service_name_.empty())
    //{
    //    LOGDEBUG("service_name_ is empty");
    //}

    //head.set_service_name(service_name_);
    XMsgHead head;
    head.set_msg_type(type);
    SetHead(&head);
    return XMsgEvent::SendMsg(&head, message);
}
//发送文件用
bool XServiceClient::SendMsg(xmsg::XMsgHead *head, XMsg *msg)
{
    if (!head || !msg)
    {
        LOGDEBUG("head or msg is null");
        return false;
    }
        
    ////需要考虑登录信息
    //if (!login_.token().empty())
    //{
    //    head->set_token(login_.token());
    //    head->set_username(login_.username());
    //    head->set_rolename(login_.rolename());
    //}
    //if (service_name_.empty())
    //{
    //    LOGDEBUG("service_name_ is empty");
    //}

    //head->set_service_name(service_name_);
    SetHead(head);
    return XMsgEvent::SendMsg(head, msg);
}
void XServiceClient::StartConnect()
{
    thread_pool_->Init(1);
    thread_pool_->Dispatch(this);
    //客户端不自动销毁，需要重连
    set_auto_delete(false);
}

XServiceClient::XServiceClient()
{
    this->thread_pool_ = XThreadPoolFactory::Create();
}

