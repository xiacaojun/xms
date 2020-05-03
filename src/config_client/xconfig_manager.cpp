#include "xconfig_manager.h"
#include "xtools.h"
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>
#include <map>
#include <thread>
#include <fstream>
using namespace std;
using namespace google;
using namespace protobuf;
using namespace compiler;
using namespace xmsg;
//存储获取的配置列表
static XConfigList *all_config = 0;
static mutex all_config_mutex;
//发出删除配置请求
void XConfigManager::DeleteConfig(const char *ip, int port)
{
    if (!ip || strlen(ip) == 0 || port < 0 || port>65535)
    {
        LOGDEBUG("DeleteConfig failed!port or ip error");
        return;
    }
    XLoadConfigReq req;
    req.set_service_ip(ip);
    req.set_service_port(port);
    //发送消息到服务端
    SendMsg(MSG_DEL_CONFIG_REQ, &req);
}

//删除配置响应
void XConfigManager::DeleteConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("ParseFromArray failed!");
        return;
    }
    if (res.return_() == XMessageRes::OK)
    {
        LOGDEBUG("删除配置成功!");
        return;
    }
    LOGDEBUG("删除配置失败");
}
void XConfigManager::set_login(xmsg::XLoginRes login)
{
    XMutex mux(&login_mutex_);
    this->login_ = login;

}

bool XConfigManager::SendMsg(xmsg::MsgType type, const google::protobuf::Message *message)
{
    XMsgHead head;
    head.set_msg_type(type);
    if (!login_.token().empty())
    {
        XMutex mux(&login_mutex_);
        head.set_token(login_.token());
        head.set_username(login_.username());
        head.set_rolename(login_.rolename());
    }

    head.set_service_name(CONFIG_NAME);
    return XMsgEvent::SendMsg(&head, message);
}

///响应获取配置列表  存储到本地
void XConfigManager::LoadAllConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("响应获取配置列表 ");
    XMutex mux(&all_config_mutex);
    if (!all_config)
        all_config = new XConfigList();
    all_config->ParseFromArray(msg->data, msg->size);
}

/////////////////////////////////////////////////////////////////////
/// 获取全部配置列表
/// 1 断开连接自动重连
/// 2 等待结果返回
xmsg::XConfigList XConfigManager::GetAllConfig(int page, int page_count, int timeout_sec)
{
    //清理历史数据
    {
        XMutex mux(&all_config_mutex);
        delete all_config;
        all_config = NULL;
    }
    XConfigList confs;
    //1 断开连接自动重连
    if (!AutoConnect(timeout_sec))
        return confs;

    //2 发送获取配置列表的消息
    XLoadAllConfigReq req;
    req.set_page(page);
    req.set_page_count(page_count);
    SendMsg(MSG_LOAD_ALL_CONFIG_REQ, &req);

    //10毫秒监听一次
    int count = timeout_sec * 100;
    for (int i = 0; i < count; i++)
    {
        {
            //会在return 之后调用释放
            XMutex mux(&all_config_mutex);
            if (all_config)
            {
                return *all_config;
            }
        }
        //是否收到响应
        this_thread::sleep_for(10ms);
    }

    return confs;
}


//接收到保存配置的消息
void XConfigManager::SendConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("接收到上传配置的反馈");
    XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("ParseFromArray failed!");
        if (SendConfigResCB)
            SendConfigResCB(false, "ParseFromArray failed!");

        return;
    }
    if (res.return_() == XMessageRes::OK)
    {
        LOGDEBUG("上传配置成功!");
        if (SendConfigResCB)
            SendConfigResCB(true, "上传配置成功!");
        return;
    }
    stringstream ss;
    ss << "上传配置失败:" << res.msg();
    if (SendConfigResCB)
        SendConfigResCB(false, ss.str().c_str());
    LOGDEBUG(ss.str().c_str());

}
//发送配置
void XConfigManager::SendConfig(xmsg::XConfig *conf)
{
    LOGDEBUG("发送配置");
    SendMsg(MSG_SAVE_CONFIG_REQ, conf);
}


//获取配置请求 IP如果为NULL 则取连接配置中心的地址
void XConfigManager::LoadConfig(const char *ip, int port)
{
    LOGDEBUG("获取配置请求");
    if (port < 0 || port>65535)
    {
        LOGDEBUG("LoadConfig failed!port error");
        return;
    }
    XLoadConfigReq req;
    if (ip) //IP如果为NULL 则取连接配置中心的地址
        req.set_service_ip(ip);
    req.set_service_port(port);
    //config_.Clear();
    //config_.service_ip().empty();
    //发送消息到服务端
    SendMsg(MSG_LOAD_CONFIG_REQ, &req);
}

///响应获取自己的配置  存储到本地
void XConfigManager::LoadConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("获取配置响应");
    //XMutex mux(&config_mutex_);
    XConfig config;
    if (!config.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoadConfigRes conf.ParseFromArray failed!");
        return;
    }
    LOGDEBUG(config.DebugString().c_str());
    if (LoadConfigResCB)
    {
        LoadConfigResCB(config);
    }
    ////key ip_port
    //stringstream key;
    //key << conf.service_ip() << "_" << conf.service_port();
    ////更新配置
    //config_mutex_.lock();
    //conf_map[key.str()] = conf;
    //config_mutex_.unlock();

    ////没有本地配置
    //if (local_port_ <= 0 || !cur_service_conf)
    //    return;
    //stringstream local_key;
    //string ip = local_ip_;
    //if (ip.empty())
    //{
    //    ip = conf.service_ip();
    //}
    //local_key << ip << "_" << local_port_;
    //if (key.str() != local_key.str())
    //{
    //    return;
    //}
    //XMutex mux(&cur_service_conf_mutex);
    //if (!cur_service_conf && !cur_service_conf->ParseFromString(conf.private_pb()))
    //{
    //    return;
    //}
    //LOGDEBUG(cur_service_conf->DebugString().c_str());
    ////存储到本地文件
    //文件名 [port]_conf.cache  20030_conf.cache
    //stringstream ss;
    //ss << local_port_ << "_conf.cache";
    //ofstream ofs;
    //ofs.open(ss.str(), ios::binary);
    //if (!ofs.is_open())
    //{
    //    LOGDEBUG("save local config failed!");
    //    return;
    //}
    //cur_service_conf->SerializePartialToOstream(&ofs);
    //ofs.close();
}


XConfigManager::XConfigManager()
{
    RegMsgCallback();
}


XConfigManager::~XConfigManager()
{
}
