#ifndef XAUTH_CLIENT_H
#define XAUTH_CLIENT_H
#include "xservice_client.h"
#include <mutex>
#include <map>
#include <vector>
#define XAUTH XAuthClient::Get()
class XAuthClient:public XServiceClient
{
public:

    ~XAuthClient();
    static XAuthClient*Get()
    {
        static XAuthClient xc;
        return &xc;
    }

    bool Login(std::string username, std::string password);


    //////////////////////////////////////////////////////////////////
    ///发送登录请求
    /// @para username 用户
    /// @para password 密码（原文），在函数中会md5编码后发送
    //static void LoginReq(XMsgEvent *msg_ev, std::string username, std::string password);

    //////////////////////////////////////////////////////////////////
    ///发送登录请求
    /// @para username 用户
    /// @para password 密码（原文），在函数中会md5编码后发送
    void LoginReq(std::string username, std::string password);


    //检查token 是否有效 更新本地的 login登录数据
    void CheckTokenReq(std::string token) ;


    //////////////////////////////////////////////////////////////////
    ///添加用户消息
    void AddUserReq(const xmsg::XAddUserReq *user);

    //////////////////////////////////////////////////////////////////
    ///修改密码消息
    void ChangePasswordReq(const xmsg::XChangePasswordReq *pass);


    /////////////////////////////////////////////////////////////////
    /// 检验登录
    /// @timeoue_ms 检测的超时时间，但如果连接异常则立刻返回
    /// 如果token快要过期，会自动发送更新请求
    /// @return 失败返回错误到 token 中
    bool GetLoginInfo(std::string username,xmsg::XLoginRes *out_info, int timeoue_ms=200);

    xmsg::XLoginRes GetLogin();

    //////////////////////////////////////////////////////////////////
    ///注册接收服务器的消息类型
    static void RegMsgCallback()
    {
        RegCB(xmsg::MSG_LOGIN_RES, (MsgCBFunc)&XAuthClient::LoginRes);
        RegCB(xmsg::MSG_ADD_USER_RES, (MsgCBFunc)&XAuthClient::AddUserRes);
        RegCB(xmsg::MSG_CHANGE_PASSWORD_RES, (MsgCBFunc)&XAuthClient::ChangePasswordRes);
        RegCB(xmsg::MSG_CHECK_TOKEN_RES, (MsgCBFunc)&XAuthClient::CheckTokenRes);
    }

    //当前登录的用户名
    std::string cur_user_name(){ return cur_user_name_; }

private:
    //当前登录的用户名
    std::string cur_user_name_;
    XAuthClient();

    //////////////////////////////////////////////////////////////////
    ///接收登录消息
    void LoginRes(xmsg::XMsgHead *head, XMsg *msg);

    //////////////////////////////////////////////////////////////////
    ///接收添加用户消息
    void AddUserRes(xmsg::XMsgHead *head, XMsg *msg);

    //////////////////////////////////////////////////////////////////
    ///接收修改密码消息
    void ChangePasswordRes(xmsg::XMsgHead *head, XMsg *msg);

    void CheckTokenRes(xmsg::XMsgHead *head, XMsg *msg);
    
    //登录数据（包含token） key是用户名 同一个用户支持多个客户端登录
    //std::map<std::string,std::vector<xmsg::XLoginRes> > login_map_;
    std::map<std::string, xmsg::XLoginRes > login_map_;
    //xmsg::XLoginRes login_;
    std::mutex logins_mutex_;

};


#endif // !XAUTH_CLIENT_H