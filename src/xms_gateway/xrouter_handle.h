#ifndef XROUTER_HANDLE_H
#define XROUTER_HANDLE_H
#include "xservice_handle.h"
class XRouterHandle : public XServiceHandle
{
public:
    XRouterHandle();
    ~XRouterHandle();
    
    virtual void ReadCB(xmsg::XMsgHead *head, XMsg *msg);

    //连接断开，超时，出错调用
    virtual void Close();

    static void InitMsgServer();

    static void CloseMsgServer();
    
    //重载 用于proxy回调
    bool  SendMsg(xmsg::XMsgHead *head, XMsg *msg);

};

#endif