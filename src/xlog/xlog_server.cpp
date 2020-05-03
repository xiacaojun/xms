#include "xlog_server.h"
#include "xlog_handle.h"
#include "xlog_dao.h"
#include "xregister_client.h"
#include "xlog_client.h"
#include <iostream>
#include "xtools.h"

using namespace std;
///根据参数 初始化服务，需要先调用
void XLogServer::main(int argc, char *argv[])
{
    XLogClient::Get()->set_is_print(false);
    cout << "xlog register_ip  register_port service_port" << endl;
    ///注册回调函数
    XLogHandle::RegMsgCallback();
    int service_port = XLOG_PORT;
    int register_port = REGISTER_PORT;
    //string register_ip = "127.0.0.1";
    string register_ip = XGetHostByName(API_REGISTER_SERVER_NAME);
    if (argc > 1)
        register_ip = argv[1];
    if (argc > 2)
        register_port = atoi(argv[2]);
    if (argc > 3)
        service_port = atoi(argv[3]);

    //设置服务器监听端口
    set_server_port(service_port);


    XRegisterClient::Get()->set_server_ip(register_ip.c_str());
    XRegisterClient::Get()->set_server_port(register_port);

    //向注册中心注册
    XRegisterClient::Get()->RegisterServer(CONFIG_NAME, service_port, 0);

    XLogClient::Get()->set_server_ip("127.0.0.1");
    XLogClient::Get()->set_server_port(service_port);
    XLogClient::Get()->set_service_name(XLOG_NAME);
    XLogClient::Get()->set_service_port(service_port);
    XLogClient::Get()->set_log_level(xmsg::XLOG_INFO);
   

    XLogClient::Get()->StartLog();

}


XServiceHandle* XLogServer::CreateServiceHandle()
{
    return new XLogHandle();
}

XLogServer::XLogServer()
{
}


XLogServer::~XLogServer()
{
}
