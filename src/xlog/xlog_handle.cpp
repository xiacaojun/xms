#include "xlog_handle.h"
#include "xlog_dao.h"
#include <iostream>
using namespace std;
using namespace xmsg;

void XLogHandle::AddLogReq(xmsg::XMsgHead *head, XMsg *msg)
{
    XAddLogReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        cout << "AddLogReq failed!" << endl;
        return;
    }
    cout << "L" << flush;   
    
    if (req.service_ip().empty())
    {
        req.set_service_ip(client_ip());
    }
    XLogDAO::Get()->AddLog(&req);
}



XLogHandle::~XLogHandle()
{
}
