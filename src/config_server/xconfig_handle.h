#pragma once
#include "xservice_handle.h"
class XConfigHandle :
    public XServiceHandle
{
public:
    XConfigHandle();
    ~XConfigHandle();

    //接收到保存配置的消息
    void SaveConfig(xmsg::XMsgHead *head, XMsg *msg);

    //下载配置
    void LoadConfig(xmsg::XMsgHead *head, XMsg *msg);


    //下载全部配置（有分页）
    void LoadAllConfig(xmsg::XMsgHead *head, XMsg *msg);

    //删除配置（
    void DeleteConfig(xmsg::XMsgHead *head, XMsg *msg);

    static void RegMsgCallback()
    {
        RegCB(xmsg::MSG_SAVE_CONFIG_REQ, (MsgCBFunc)&XConfigHandle::SaveConfig);
        RegCB(xmsg::MSG_LOAD_CONFIG_REQ, (MsgCBFunc)&XConfigHandle::LoadConfig);
        RegCB(xmsg::MSG_LOAD_ALL_CONFIG_REQ, (MsgCBFunc)&XConfigHandle::LoadAllConfig);
        RegCB(xmsg::MSG_DEL_CONFIG_REQ, (MsgCBFunc)&XConfigHandle::DeleteConfig);
    }
};

