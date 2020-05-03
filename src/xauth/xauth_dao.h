#ifndef XAUTH_DAO_H
#define XAUTH_DAO_H

#include <string>
namespace LX {
    class  LXMysql;
}
namespace xmsg {
    class  XAddUserReq;
    class XChangePasswordReq;
    class XLoginReq;
    class XLoginRes;
    class XMsgHead;

}
class XAuthDao
{
public:
    static XAuthDao *Get()
    {
        static XAuthDao dao;
        return &dao;
    }
    ///初始化数据库
    //bool Init(const char *ip, const char *user, const char*pass, const char*db_name, int port = 3306);
    bool Init();

    ///安装数据库的表
    bool Install();

    ///////////////////////////////////////////////////////////////////////////////////
    /// 登录系统
    /// @para username 用户名
    /// @hash_password 经过md5 hash编码后的密码
    /// @token 如果成功返回登录令牌 如果出错，返回错误原因
    /// @timeout_sec 令牌的超时时间（秒），默认30分钟
    /// @return 登录成功或者失败
    //bool Login(std::string username, std::string hash_password,std::string &token,int timeout_sec=1200);
    bool Login(const xmsg::XLoginReq *user_req,xmsg::XLoginRes *user_res, int timeout_sec);

    bool CheckToken(const xmsg::XMsgHead *head, xmsg::XLoginRes *user_res);


    //////////////////////////////////////////////////////////////////
    ///接收添加用户消息
    bool AddUser(const xmsg::XAddUserReq *user);

    //////////////////////////////////////////////////////////////////
    ///接收修改密码消息
    bool ChangePassword(const xmsg::XChangePasswordReq *pass);



    XAuthDao();
    ~XAuthDao();
private:
    LX::LXMysql *my_ = 0;
};


#endif
