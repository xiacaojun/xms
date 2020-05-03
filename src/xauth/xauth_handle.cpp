#include "xauth_handle.h"
#include "xauth_dao.h"
#include "xtools.h"
#include <string>
using namespace std;
using namespace xmsg;

//////////////////////////////////////////////////////////////////
///接收登录请求
void XAuthHandle::CheckTokenReq(xmsg::XMsgHead *head, XMsg *msg)
{
    XLoginRes res;
    XAuthDao::Get()->CheckToken(head, &res);
}

//////////////////////////////////////////////////////////////////
///接收登录请求
void XAuthHandle::LoginReq(xmsg::XMsgHead *head, XMsg *msg)
{
    //登录超时时间，后面改为从配置中心获取
    int timeout_sec = 1800;
   
    
    //登录请求
    XLoginReq req;

    //登录返回消息
    XLoginRes res;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoginReq failed!");
        res.set_res(XLoginRes::ERROR);
        res.set_token("LoginReq ParseFromArray failed!");
        SendMsg(MSG_LOGIN_RES, &res);
        return;
    }
    //bool re = XAuthDao::Get()->Login(req.username(), req.password(), token, timeout_sec);
    bool re = XAuthDao::Get()->Login(&req,&res, timeout_sec);
    if (!re)
    {
        LOGDEBUG("XAuthDao::Get()->Login failed!");
        res.set_res(XLoginRes::ERROR);
        res.set_token("username or password failed!");
        //res.set_res(XLoginRes::ERROR);
        //res.set_token("XAuthDao::Get()->Login failed!");
        //SendMsg(MSG_LOGIN_RES, &res);
        //return;
    }
    //res.set_res(XLoginRes::OK);
    //res.set_token(token);
    head->set_msg_type(MSG_LOGIN_RES);
    SendMsg(head, &res);
    //SendMsg(MSG_LOGIN_RES, &res);
}


//////////////////////////////////////////////////////////////////
///接收添加用户消息
void XAuthHandle::AddUserReq(xmsg::XMsgHead *head, XMsg *msg)
{
    XAddUserReq req;
    XMessageRes res;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        res.set_return_(XMessageRes::ERROR);
        res.set_msg("XAddUserReq ParseFromArray failed!");
        SendMsg(MSG_ADD_USER_RES, &res);
        return;
    }
    bool re = XAuthDao::Get()->AddUser(&req);
    if (!re)
    {
        res.set_return_(XMessageRes::ERROR);
        res.set_msg("XAuthDao::Get()->AddUser failed!");
        SendMsg(MSG_ADD_USER_RES, &res);
        return;
    }
    res.set_return_(XMessageRes::OK);
    res.set_msg("OK!");
    head->set_msg_type(MSG_ADD_USER_RES);
    SendMsg(head, &res);

}

//////////////////////////////////////////////////////////////////
///接收修改密码消息
void XAuthHandle::ChangePasswordReq(xmsg::XMsgHead *head, XMsg *msg)
{
    XChangePasswordReq req;
    XMessageRes res;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        res.set_return_(XMessageRes::ERROR);
        res.set_msg("XChangePasswordReq ParseFromArray failed!");
        SendMsg(MSG_CHANGE_PASSWORD_RES, &res);
        return;
    }
    bool re = XAuthDao::Get()->ChangePassword(&req);
    if (!re)
    {
        res.set_return_(XMessageRes::ERROR);
        res.set_msg("XAuthDao::Get()->ChangePassword failed!");
        SendMsg(MSG_CHANGE_PASSWORD_RES, &res);
        return;
    }
    res.set_return_(XMessageRes::OK);
    res.set_msg("OK!");

    head->set_msg_type(MSG_CHANGE_PASSWORD_RES);
    SendMsg(head, &res);
    //SendMsg(MSG_CHANGE_PASSWORD_RES, &res);
}

XAuthHandle::XAuthHandle()
{
}


XAuthHandle::~XAuthHandle()
{
}
