#include "xrouter_handle.h"
#include "xtools.h"
#include "xservice_proxy.h"
#include "xauth_proxy.h"
#include "xlog_client.h"
#include <list>
#include <thread>
using namespace std;
using namespace xmsg;

//存储消息的发送的过程
//class RouterMsg
//{
//public:
//
//    //发送的消息
//    xmsg::XMsgHead send_head;
//    XMsg send_msg;
//
//    //接收的消息
//    xmsg::XMsgHead recv_head;
//    XMsg recv_msg;
//};
//
//class MsgServerThread 
//{
//public:
//    void Start()
//    {
//        thread th(&XServiceProxy::Main, this);
//        th.detach();
//    }
//    //开启线程用于处理消息
//    void Main()
//    {
//        while (!is_exit_)
//        {
//
//        }
//    }
//    void Stop()
//    {
//        is_exit_ = true;
//    }
//
//    void Send(xmsg::XMsgHead *head, XMsg *m, XMsgEvent *ev)
//    {
//        XMsg msg;
//        //msg
//
//    }
//private:
//    bool is_exit_ = false;
//    list<XMsg> msgs;
//    //map<string, RouterMsg>
//
//};
//static MsgServerThread msg_server;
void XRouterHandle::InitMsgServer()
{
    //msg_server.Start();
}

void XRouterHandle::CloseMsgServer()
{
   // msg_server.Stop();
}

bool XRouterHandle::SendMsg(xmsg::XMsgHead *head, XMsg *msg)
{
    bool re = XMsgEvent::SendMsg(head, msg);
    if(re)
        cout << "消息已回复" << head ->DebugString()<< endl;
    else
        cout << "消息回复异常" << head->DebugString() << endl;
    return re;
}

void XRouterHandle::ReadCB(xmsg::XMsgHead *head, XMsg *msg)
{
    //转发消息
    LOGDEBUG("XRouterHandle::ReadCB");
    static int i = 0;
    i++;
    cout << i <<"XRouterHandle::ReadCB" << head->DebugString();

    //获取token信息
    string token = head->token();
    string user = head->username();
    //如果是CheckToken 消息，空包 msg也有对象，size=0
    // 验证token是否有效，是否与用户一致
    //bool re = XAuthProxy::Get()->CheckToken(user,token);

    
    //此函数线程池调用，如果阻塞或影响同线程任务。
    //验证消息权限

    //鉴权成功再发送消息
    if (!head)
        return;

    //排除不需要鉴权的消息 ,需要优化
    if (head->msg_type() != MSG_LOGIN_REQ && 
        !XAuthProxy::CheckToken(head))
    {
        LOGINFO(head->DebugString());
        //LOGINFO("XAuthProxy::CheckToken failed!");
        //鉴权失败，暂不处理，需要重构优化回复内容
        return;
        //XMsgEvent::SendMsg(head, msg);
    }


    head->set_msg_id((long long)this);
    //msg->head = head;
    //msg_server.Send(head, msg, this);
    XServiceProxy::Get()->SendMsg(head, msg,this);
}
void XRouterHandle::Close()
{
	XServiceProxy::Get()->DelEvent(this);
    XMsgEvent::Close();
}


XRouterHandle::XRouterHandle()
{
}


XRouterHandle::~XRouterHandle()
{
}
