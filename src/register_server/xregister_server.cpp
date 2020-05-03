#include "xregister_server.h"
#include "xregister_handle.h"
#include "xlog_client.h"
///根据参数 初始化服务，需要先调用
void XRegisterServer::main(int argc, char *argv[])
{
    //注册消息回调函数
    XRegisterHandle::RegMsgCallback();

    int port = REGISTER_PORT;
    if (argc > 1)
        port = atoi(argv[1]);

    //需要考虑从注册中心自己获取日志模块
    XLogClient::Get()->set_service_name("REGISTER");
    XLogClient::Get()->set_server_port(XLOG_PORT);
    XLogClient::Get()->set_auto_connect(true);
    XLogClient::Get()->StartLog();

    //设置服务器监听端口
    set_server_port(port);
}
XServiceHandle* XRegisterServer::CreateServiceHandle()
{
    auto handle = new XRegisterHandle();
    //设定超时，用于接收心跳包
    handle->set_read_timeout_ms(5000);
    return handle;
}

//等待线程退出
void XRegisterServer::Wait()
{
    XThreadPool::Wait();
}
