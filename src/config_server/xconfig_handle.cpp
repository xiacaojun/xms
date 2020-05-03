#include "xconfig_handle.h"
#include "xtools.h"
#include "config_dao.h"
using namespace xmsg;
using namespace std;
//接收到保存配置的消息
void XConfigHandle::SaveConfig(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("接收到保存配置的消息");
    XMessageRes res;
    XConfig conf;
    XMsgHead h;
    h.set_msg_id(head->msg_id());

    if (!conf.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XConfigHandle::SaveConfig failed! format error!");
        res.set_return_(XMessageRes::ERROR);
        res.set_msg(" format erro");

        h.set_msg_type(MSG_SAVE_CONFIG_RES);
        SendMsg(&h, &res);
        return;
    }

    if (conf.service_ip().empty())
    {
        string ip = client_ip();
        conf.set_service_ip(ip);
    }

    if (ConfigDao::Get()->SaveConfig(&conf))
    {
        res.set_return_(XMessageRes::OK);
        res.set_msg("OK");
        //SendMsg(MSG_SAVE_CONFIG_RES, &res);
        h.set_msg_type(MSG_SAVE_CONFIG_RES);
        SendMsg(&h, &res);
        return;
    }
    res.set_return_(XMessageRes::ERROR);
    res.set_msg("insert db failed!");
    //SendMsg(MSG_SAVE_CONFIG_RES, &res);
    h.set_msg_type(MSG_SAVE_CONFIG_RES);
    SendMsg(&h, &res);
}
//下载配置
void XConfigHandle::LoadConfig(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("接收到下载配置的消息");
    XLoadConfigReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoadConfig ParseFromArray failed!");
        return;
    }
    //如果没有IP地址，则采用客户端IP
    string ip = req.service_ip();
    if (ip.empty())
    {
        ip = client_ip();
    }


    //根据IP和端口获取配置项
    XConfig conf = ConfigDao::Get()->LoadConfig(ip.c_str(), req.service_port());
    XMsgHead h;
    h.set_msg_id(head->msg_id());
    h.set_msg_type(MSG_LOAD_CONFIG_RES);
    SendMsg(&h, &conf);
    //发送给客户端
    //SendMsg(MSG_LOAD_CONFIG_RES, &conf);
}
//删除配置
void XConfigHandle::DeleteConfig(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("删除配置");
    XLoadConfigReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("DeleteConfig ParseFromArray failed!");
        return;
    }
    XMessageRes res;

    //根据IP和端口获取配置项
    if (ConfigDao::Get()->DeleteConfig(req.service_ip().c_str(), req.service_port()))
    {
        res.set_return_(XMessageRes::OK);
        res.set_msg("OK");
        //SendMsg(MSG_DEL_CONFIG_RES, &res);

        XMsgHead h;
        h.set_msg_id(head->msg_id());
        h.set_msg_type(MSG_DEL_CONFIG_RES);
        SendMsg(&h, &res);

        return;
    }
    res.set_return_(XMessageRes::ERROR);
    res.set_msg(" ConfigDao::Get()->DeleteConfig failed!");
    //发送给客户端
    //SendMsg(MSG_DEL_CONFIG_RES, &res);
    XMsgHead h;
    h.set_msg_id(head->msg_id());
    h.set_msg_type(MSG_DEL_CONFIG_RES);
    SendMsg(&h, &res);
}
//下载全部配置（有分页）
void XConfigHandle::LoadAllConfig(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("下载全部配置（有分页）");
    XLoadAllConfigReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoadAllConfig ParseFromArray failed!");
        return;
    }
    auto confs=ConfigDao::Get()->LoadAllConfig(req.page(), req.page_count());
    //发送给客户端
    //SendMsg(MSG_LOAD_ALL_CONFIG_RES, &confs);

    XMsgHead h;
    h.set_msg_id(head->msg_id());
    h.set_msg_type(MSG_LOAD_ALL_CONFIG_RES);
    SendMsg(&h, &confs);
}
XConfigHandle::XConfigHandle()
{
}


XConfigHandle::~XConfigHandle()
{
}
