#pragma once

#include "xservice_client.h"

typedef void(*ConfigResCBFunc) (bool is_ok, const char *msg);
typedef void(*GetConfigResCBFunc) (xmsg::XConfig);


#define MCONF XConfigManager::Get()
class XConfigManager :public XServiceClient
{
public:
    static XConfigManager *Get()
    {
        static XConfigManager cc;
        return &cc;
    }
    XConfigManager();
    ~XConfigManager();

    /////////////////////////////////////////////////////////////////////
    /// 删除配置
    //发出删除配置请求
    void DeleteConfig(const char *ip, int port);

    //删除配置响应
    void DeleteConfigRes(xmsg::XMsgHead *head, XMsg *msg);

    /////////////////////////////////////////////////////////////////////
    /// 获取全部配置列表
    /// 1 断开连接自动重连
    /// 2 等待结果返回
    xmsg::XConfigList GetAllConfig(int page, int page_count, int timeout_sec);

    ///响应获取配置列表  存储到本地
    void LoadAllConfigRes(xmsg::XMsgHead *head, XMsg *msg);

    /////////////////////////////////////////////////////////////////////
    //发送保存配置
    void SendConfig(xmsg::XConfig *conf);

    //接收到保存配置的消息
    void SendConfigRes(xmsg::XMsgHead *head, XMsg *msg);

    /////////////////////////////////////////////////////////////////////
    ///发出获取配置请求 IP如果为NULL 则取连接配置中心的地址
    void LoadConfig(const char *ip, int port);

    ///响应获取自己的配置  存储到本地
    void LoadConfigRes(xmsg::XMsgHead *head, XMsg *msg);

    static void RegMsgCallback()
    {
        RegCB(xmsg::MSG_SAVE_CONFIG_RES, (MsgCBFunc)&XConfigManager::SendConfigRes);
        RegCB(xmsg::MSG_LOAD_CONFIG_RES, (MsgCBFunc)&XConfigManager::LoadConfigRes);
        RegCB(xmsg::MSG_LOAD_ALL_CONFIG_RES, (MsgCBFunc)&XConfigManager::LoadAllConfigRes);
        RegCB(xmsg::MSG_DEL_CONFIG_RES, (MsgCBFunc)&XConfigManager::DeleteConfigRes);
    }


    //设定登录信息
    void set_login(xmsg::XLoginRes login);

    //保存配置后的回调函数
    ConfigResCBFunc SendConfigResCB = 0;

    //加载配置后的回调
    GetConfigResCBFunc LoadConfigResCB = 0;




private:
    xmsg::XLoginRes login_;
    
    std::mutex login_mutex_;

    virtual bool  SendMsg(xmsg::MsgType type, const google::protobuf::Message *message);

   // xmsg::XConfig config_;
    //std::mutex config_mutex_;
};

