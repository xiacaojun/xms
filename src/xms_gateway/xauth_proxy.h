#pragma once
#include <string>
#include "xservice_proxy_client.h"
class XAuthProxy:public XServiceProxyClient
{
public:
    static void InitAuth();
    virtual void ReadCB(xmsg::XMsgHead *head, XMsg *msg);

    //static XAuthProxy* Get();
    static bool CheckToken(const xmsg::XMsgHead *head);
    //bool CheckToken(std::string username,std::string token);

    //~XAuthProxy();
};

