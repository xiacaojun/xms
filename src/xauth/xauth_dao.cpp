#include "xauth_dao.h"
#include "xtools.h"
#include "LXMysql.h"
#include "xmsg_com.pb.h"
#include <thread>
#include <mutex>
using namespace std;
using namespace LX;
using namespace xmsg;

//互斥访问my_
static mutex auth_mutex;

///初始化数据库
//bool Init(const char *ip, const char *user, const char*pass, const char*db_name, int port = 3306);
bool XAuthDao::Init()
{
    {
        XMutex mux(&auth_mutex);

        if (!my_)
            my_ = new LXMysql();
        if (!my_->Init())
        {
            LOGDEBUG("XAuthDao  my_->Init() failed!");
            return false;
        }

        //自动重连
        my_->SetReconnect(true);

        my_->SetConnectTimeout(3);
        //if (!my_->Connect(ip, user, pass, db_name, port))
        if (!my_->InputDBConfig())
        {
            LOGDEBUG("my_->Connect failed!");
            return false;
        }
    }
    LOGDEBUG("my_->Connect success!");
    XAddUserReq user;
    user.set_username("root");
    //char md5_password[128] = { 0 };
    string password = "123456";
    auto md5_password = XMD5_base64((unsigned char *)password.data(), password.size());
    user.set_password(md5_password);
    AddUser(&user);
    return true;
}


//////////////////////////////////////////////////////////////////
///接收添加用户消息
bool XAuthDao::AddUser(const xmsg::XAddUserReq *user)
{
    LOGDEBUG("XAuthDao::AddUser");

    XMutex mux(&auth_mutex);
    if (!my_)
    {
        LOGERROR("mysql not init");
        return false;
    }
    XDATA data;
    data["xms_username"] = user->username().c_str();
    data["xms_password"] = user->password().c_str();
    data["xms_rolename"] = user->rolename().c_str();
    if (!my_->Insert(data, "xms_auth"))
    {
        LOGERROR("Insert xms_auth failed！");
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////
///接收修改密码消息
bool XAuthDao::ChangePassword(const xmsg::XChangePasswordReq *pass)
{
    LOGDEBUG("XAuthDao::ChangePassword");

    XMutex mux(&auth_mutex);
    if (!my_)
    {
        LOGERROR("mysql not init");
        return false;
    }

    XDATA data;
    data["xms_password"] = pass->password().c_str();
    string where = " where xms_username='" + pass->username()+"'";

    if (!my_->Update(data, "xms_auth",where))
    {
        LOGERROR("Update xms_auth failed！");
        return false;
    }
    return true;
}
///安装数据库的表
bool XAuthDao::Install()
{
    LOGDEBUG("XAuthDao::Install()");

    XMutex mux(&auth_mutex);
    if (!my_)
    {
        LOGERROR("mysql not init");
        return false;
    }

    string sql = "";

    //如果表不存在则创建
    sql = "CREATE TABLE IF NOT EXISTS `xms_auth` ( \
        `id` INT AUTO_INCREMENT,\
        `xms_username` VARCHAR(128) ,\
        `xms_password` VARCHAR(1024) ,\
        `xms_rolename` VARCHAR(128) ,\
        PRIMARY KEY(`id`),\
        UNIQUE KEY `xms_username_UNIQUE` (`xms_username`)\
        );";


    if (!my_->Query(sql.c_str()))
    {
        LOGINFO("CREATE TABLE xms_auth failed!");
        return false;
    }
    LOGINFO("CREATE TABLE xms_auth success!");
    
    //如果表不存在则创建
    sql = "CREATE TABLE IF NOT EXISTS `xms_token` ( \
        `id` INT AUTO_INCREMENT,\
        `xms_username` VARCHAR(1024) ,\
        `xms_rolename` VARCHAR(128) ,\
        `token` VARCHAR(64) ,\
        `expired_time` int ,\
        PRIMARY KEY(`id`));";

    if (!my_->Query(sql.c_str()))
    {
        LOGINFO("CREATE TABLE xms_token failed!");
        return false;
    }
    LOGINFO("CREATE TABLE xms_token success!");

    
    return true;
}


bool XAuthDao::CheckToken(const xmsg::XMsgHead *head, xmsg::XLoginRes *user_res)
{
    LOGDEBUG("ConfigDao::CheckToken");
    string token = "";
    if (!head || head->token().empty())
    {
        token = "token is null";
        LOGERROR(token);
        user_res->set_token(token);
        return false;
    }

    token = head->token();
    user_res->set_res(XLoginRes::ERROR);
    XMutex mux(&auth_mutex);
    if (!my_)
    {
        token = "mysql not init";
        LOGERROR(token);
        user_res->set_token(token);
        return false;
    }

    if (head->token().empty())
    {
        token = "token is empty!";
        LOGERROR(token);
        user_res->set_token(token);
        return false;
    }


    //验证用户名密码
    string table_name = "xms_token";
    stringstream ss;
    ss << "select xms_username,xms_rolename,expired_time from " << table_name;
    ss << " where token='" <<token<< "'";
    //ss << " where xms_username='" << user_req->username() << "' and xms_password='" << user_req->password() << "'";
    auto rows = my_->GetResult(ss.str().c_str());
    if (rows.size() == 0)
    {
        token = "token  invalid !";
        LOGERROR(token);
        user_res->set_token(token);
        return false;
    }
    user_res->set_username(rows[0][0].data);
    user_res->set_rolename(rows[0][1].data);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////
/// 登录系统
/// @para user_name 用户名
/// @hash_password 经过md5 hash编码后的密码
/// @error 如果出错，返回错误原因
/// @return 登录成功或者失败
//bool XAuthDao::Login(std::string username, std::string hash_password, std::string &token, int timeout_sec)
bool XAuthDao::Login(const xmsg::XLoginReq *user_req, xmsg::XLoginRes *user_res, int timeout_sec)
{
    LOGDEBUG("ConfigDao::LoadConfig");
    string token = "";
    user_res->set_res(XLoginRes::ERROR);
    XMutex mux(&auth_mutex);
    if (!my_)
    {
        
        token = "mysql not init";
        LOGERROR(token);
        user_res->set_token(token);
        return false;
    }
    if (user_req->username().empty())
    {
        token = "user_name is empty!";
        LOGERROR(token);
        user_res->set_token(token);
        return false;
    }
    //验证用户名密码
    string table_name = "xms_auth";
    stringstream ss;
    ss << "select xms_username,xms_rolename from " << table_name;
    ss << " where xms_username='" << user_req->username() << "' and xms_password='" << user_req->password()<<"'";
    auto rows = my_->GetResult(ss.str().c_str());
    if (rows.size() == 0)
    {
        token = "username or password error!";
        LOGERROR(token);
        user_res->set_token(token);
        return false;
    }
    string rolename = rows[0][1].data;
    string username = user_req->username();
    user_res->set_rolename(rolename);
    user_res->set_username(username);

    int now = time(0);
    int expired_time = now+ timeout_sec;
    //创建token
    XDATA data;
    data["@token"] = "UUID()";
    data["xms_username"] = username.c_str();
    data["xms_rolename"] = rolename.c_str();
    ss.str("");
    ss << expired_time;
    string expired = ss.str();
    data["expired_time"] = expired.c_str();
    if (!my_->Insert(data, "xms_token"))
    {
        token = "Insert token failed！";
        LOGERROR(token);
        user_res->set_token(token);
        return false;
    }
    //返回token
    int id = my_->GetInsertID();
    ss.str("");
    ss << "select token,expired_time from xms_token where id=" << id;
    rows = my_->GetResult(ss.str().c_str());
    if (rows.size() <= 0 || rows[0][0].data == NULL || rows[0][0].size <=0)
    {
        token = "Insert token id error！";
        LOGERROR(token);
        user_res->set_token(token);
        return false;
    }
    user_res->set_res(XLoginRes::OK);
    token = rows[0][0].data;
    user_res->set_token(token);
    user_res->set_expired_time(atoi(rows[0][1].data));
    //清理过期登录信息,后面改成用线程定期清理
    ss.str("");
    ss << "delete from xms_token where expired_time<"<<now;
    if (!my_->Query(ss.str().c_str()))
    {
        LOGDEBUG(ss.str().c_str());
    }
    return true;
}

XAuthDao::XAuthDao()
{
}


XAuthDao::~XAuthDao()
{
}
