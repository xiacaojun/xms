
#include <iostream>
#include "xservice.h"
#include "xauth_handle.h"
#include "xconfig_and_register.h"
#include "xauth_dao.h"
#include <thread>
#include "xtools.h"
using namespace std;
using namespace xmsg;
class XAuthService :public XService
{
public:
    XServiceHandle* CreateServiceHandle()
    {
        return new XAuthHandle();
    }
};

#include "xtools.h"
int main(int argc,char *argv[])
{
    cout << "xauth_server SERVER_PORT REGISTER_IP REGISTER_PORT" << endl;
    cout << "xauth_server install" << endl;
    if (!XAuthDao::Get()->Init())
    {
        cout << "DB init failed!" << endl;
        return -1;
    }
    if (!XAuthDao::Get()->Install())
    {
        cout << "DB create table failed!" << endl;
        return -2;
    }

    if (argc > 1)
    {
        string cmd = argv[1];
        //if (cmd == "install")
        //{
        //    if (!XAuthDao::Get()->Install())
        //    {
        //        cout << "DB create table failed!" << endl;
        //        return -2;
        //    }
        //    cout << "xauth_server install success!" << endl;
        //    return 0;
        //}
    }
    //注册消息回调函数
    XAuthHandle::RegMsgCallback();


    int server_port = AUTH_PORT;
    if (argc > 1)
        server_port = atoi(argv[1]);
    cout << "server port is " << server_port << endl;


    string register_ip = "";
    register_ip =  XGetHostByName(API_REGISTER_SERVER_NAME);
    
    if (argc > 2)
        register_ip = argv[2];

    if (register_ip.empty())
        register_ip = "127.0.0.1";

    int register_port = REGISTER_PORT;
    if (argc > 3)
        register_port = atoi(argv[3]);
    static XAuthConfig config;
    XConfigAndRegister::Init(AUTH_NAME, 0, server_port,
        register_ip.c_str(), register_port,&config
        );


    XAuthService service;
    service.set_server_port(server_port);
    service.Start();
    XThreadPool::Wait();
    return 0;
}

