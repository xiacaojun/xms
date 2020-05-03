#pragma once
#include "xservice_handle.h"

//////////////////////////
///处理注册中心的客户端 对应一个连接
class XRegisterHandle:public XServiceHandle
{
public:
    XRegisterHandle();
    ~XRegisterHandle();

    //接收服务的注册请求
    void RegisterReq(xmsg::XMsgHead *head, XMsg *msg);


    //接收服务的发现请求
    void GetServiceReq(xmsg::XMsgHead *head, XMsg *msg);
    void HeartRes(xmsg::XMsgHead *head, XMsg *msg) {};
    static void RegMsgCallback()
    {
        RegCB(xmsg::MSG_HEART_REQ, (MsgCBFunc)&XRegisterHandle::HeartRes);
        RegCB(xmsg::MSG_REGISTER_REQ, (MsgCBFunc)&XRegisterHandle::RegisterReq);
        RegCB(xmsg::MSG_GET_SERVICE_REQ, (MsgCBFunc)&XRegisterHandle::GetServiceReq);
    }

   
};

