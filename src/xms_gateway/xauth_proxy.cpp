#include "xauth_proxy.h"
#include "xauth_client.h"
#include <map>
#include <string>
#include <thread>
#include "xmsg_com.pb.h"
#include "xtools.h"
using namespace std;
using namespace xmsg;
static map<string, XLoginRes> token_cache;
static mutex token_cache_mutex;

class TokenThread
{
public:
    void Start()
    {
        thread th(&TokenThread::Main, this);
        th.detach();
    }
    ~TokenThread()
    {
        is_exit_ = true;
        this_thread::sleep_for(10ms);
    }
private:
    bool is_exit_ = false;
    void Main()
    {
        while (!is_exit_)
        {
            token_cache_mutex.lock();
            //检测过期token 需要优化
            auto ptr = token_cache.begin();
            for (; ptr != token_cache.end(); )
            {
                auto tmp = ptr;
                auto tt = time(0);
                ptr++;
                if (tmp->second.expired_time() < tt)
                {
                    cout << "expired_time " << tmp->second.expired_time() << endl;
                    token_cache.erase(tmp);
                }
            }
            token_cache_mutex.unlock();
            this_thread::sleep_for(1s);
        }
    }
};

static TokenThread token_thread;
void XAuthProxy::InitAuth()
{
    token_thread.Start();
}

bool XAuthProxy::CheckToken(const xmsg::XMsgHead *head)
{
    XMutex mux(&token_cache_mutex);
    string token = head->token();
    if (token.empty())
    {
        LOGINFO("XAuthProxy::CheckToken failed! token is empty!");
        return false;
    }
    auto tt = token_cache.find(token);
    if (tt == token_cache.end())
    {
        LOGINFO("XAuthProxy::CheckToken failed! the token cache not find!");
        return false;
    }
        
    if (tt->second.username() != head->username())
    {
        stringstream ss;
        ss << "XAuthProxy::CheckToken failed! username is error!(head/cache)" << head->username();
        ss << "/" << tt->second.username();
        LOGINFO(ss.str().c_str());
        return false;
    }

    if (tt->second.rolename() != head->rolename())
    {
        stringstream ss;
        ss << "XAuthProxy::CheckToken failed! rolename is error!(head/cache)" << head->rolename();
        ss << "/" << tt->second.rolename();
        LOGINFO(ss.str().c_str());
        return false;
    }
    return true;
}

void XAuthProxy::ReadCB(xmsg::XMsgHead *head, XMsg *msg)
{
    if (!head)return;
    //如果是登录验证，和token延期 则本地保存
    // 前期先只验证token有效，后面再验证权限
    //XLoginRes
    XLoginRes res;
    switch (head->msg_type())
    {
    case MSG_LOGIN_RES: //登录响应
        if (res.ParseFromArray(msg->data, msg->size))
        {
            XMutex mux(&token_cache_mutex);
            token_cache[res.token()] = res;
        }
        cout << res.DebugString();

    default:
        break;
    }
    XServiceProxyClient::ReadCB(head, msg);
}
//class CXAuthProxy :public XAuthProxy
//{
//public:
//    bool CheckToken(std::string username, std::string token)
//    {
//        XAuthClient::Get()->CheckTokenReq(token);
//        return true;
//    }
//private:
//
//};
//bool XAuthProxy::CheckToken(std::string username, std::string token)
//{
//    return true;
//}
//
//
//XAuthProxy *XAuthProxy::Get()
//{
//    static CXAuthProxy xp;
//    
//    return &xp;
//}