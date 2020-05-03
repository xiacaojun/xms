#include "xconfig_server.h"
#include "xtools.h"
#include "xregister_client.h"
#include "xconfig_handle.h"
using namespace std;

XConfigServer::XConfigServer()
{
}


XConfigServer::~XConfigServer()
{
}
///根据参数 初始化服务，需要先调用
void XConfigServer::main(int argc, char *argv[])
{
    ///注册中心的配置
    LOGDEBUG("config_server register_ip register_port  service_port ");

    ///注册回调函数
    XConfigHandle::RegMsgCallback();
    int service_port = CONFIG_PORT;
    int register_port = REGISTER_PORT;
    //string register_ip = "127.0.0.1";
    string register_ip = XGetHostByName(API_REGISTER_SERVER_NAME);
    if (argc > 1)
        register_ip = argv[1];
    if(argc>2)
        register_port = atoi(argv[2]);
    if (argc > 3)
        service_port = atoi(argv[3]);

    //设置服务器监听端口
    set_server_port(service_port);

    
    XRegisterClient::Get()->set_server_ip(register_ip.c_str());
    XRegisterClient::Get()->set_server_port(register_port);

    //向注册中心注册
    XRegisterClient::Get()->RegisterServer(CONFIG_NAME, service_port, 0);
}
XServiceHandle* XConfigServer::CreateServiceHandle()
{
    return new XConfigHandle();
}

//等待线程退出
void XConfigServer::Wait()
{
    XThreadPool::Wait();
}