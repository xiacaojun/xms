#pragma once
#include "xservice_handle.h"
class XLogHandle :public XServiceHandle
{
public:
    XLogHandle() { RegMsgCallback(); };
    ~XLogHandle();
    void AddLogReq(xmsg::XMsgHead *head, XMsg *msg);
    static void RegMsgCallback()
    {
        RegCB(xmsg::MSG_ADD_LOG_REQ, (MsgCBFunc)&XLogHandle::AddLogReq);
    }
};

