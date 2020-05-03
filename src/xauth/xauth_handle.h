#pragma once
#include "xservice_handle.h"
class XAuthHandle:public XServiceHandle
{
public:
    XAuthHandle();
    ~XAuthHandle();

    //////////////////////////////////////////////////////////////////
    ///接收登录请求
    void LoginReq(xmsg::XMsgHead *head, XMsg *msg);

    //////////////////////////////////////////////////////////////////
    ///接收登录请求
    void CheckTokenReq(xmsg::XMsgHead *head, XMsg *msg);

    //////////////////////////////////////////////////////////////////
    ///接收添加用户消息
    void AddUserReq(xmsg::XMsgHead *head, XMsg *msg);

    //////////////////////////////////////////////////////////////////
    ///接收修改密码消息
    void ChangePasswordReq(xmsg::XMsgHead *head, XMsg *msg);

    static void RegMsgCallback()
    {
        RegCB(xmsg::MSG_LOGIN_REQ, (MsgCBFunc)&XAuthHandle::LoginReq);
        RegCB(xmsg::MSG_ADD_USER_REQ, (MsgCBFunc)&XAuthHandle::AddUserReq);
        RegCB(xmsg::MSG_CHANGE_PASSWORD_REQ, (MsgCBFunc)&XAuthHandle::ChangePasswordReq);
        RegCB(xmsg::MSG_CHANGE_PASSWORD_REQ, (MsgCBFunc)&XAuthHandle::CheckTokenReq);
    }
};

