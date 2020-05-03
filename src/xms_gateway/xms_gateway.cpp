#include <iostream>
#include "xservice.h"
#include "xrouter_server.h"
#include "xservice_proxy.h"
#include "xregister_client.h"
#include "xmsg_com.pb.h"
#include "xconfig_client.h"
#include "xauth_proxy.h"
#include "xtools.h"
using namespace std;
using namespace xmsg;

#define REG  XRegisterClient::Get()
#define CONF XConfigClient::Get()
void ConfTimer()
{
    static string conf_ip = "";
    static int conf_port = 0;
    /////////////////////////////////////////////////////////////////
    //读取配置项
    //cout << "config root = " << CONF->GetString("root") << endl;

    if (conf_port <= 0)
    {
        //从注册中心获取配置中心的IP
        auto confs = REG->GetServcies(CONFIG_NAME, 1);
        cout << confs.DebugString();
        if (confs.service_size() <= 0)
            return;
        auto conf = confs.service()[0];

        if (conf.ip().empty() || conf.port() <= 0)
            return;
        conf_ip = conf.ip();
        conf_port = conf.port();
        CONF->set_server_ip(conf_ip.c_str());
        CONF->set_server_port(conf_port);
        CONF->Connect();
    }
}
int main(int argc, char *argv[])
{
    /// xms_gateway
    cout << "xms_gateway API_GATEWAY_PORT REGISTER_IP REGISTER_PORT" << endl;
    int server_port = API_GATEWAY_PORT;
    if (argc > 1)
        server_port = atoi(argv[1]);
    cout << "server port is " << server_port << endl;
    string register_ip = "";

    register_ip = XGetHostByName(API_REGISTER_SERVER_NAME);
    if (argc > 2)
        register_ip = argv[2];
    int register_port = REGISTER_PORT;
    if(argc>3)
        register_port = atoi(argv[3]);
    if(register_ip.empty())
        register_ip =  "127.0.0.1";

    XAuthProxy::InitAuth();

    //设置注册中心的IP和端口
    XRegisterClient::Get()->set_server_ip(register_ip.c_str());
    XRegisterClient::Get()->set_server_port(register_port);

    //注册到注册中心
    XRegisterClient::Get()->RegisterServer(API_GATEWAY_NAME, server_port, 0,true);
    //
    ////等待注册中心连接
    XRegisterClient::Get()->WaitConnected(3);
    /*for (;;)
    {
        XRegisterClient::Get()->GetServiceReq(0);
        auto tmp = XRegisterClient::Get()->GetAllService();
        if (tmp)
        {
            cout << tmp->DebugString();
        }
        this_thread::sleep_for(100ms);
    }*/
    XRegisterClient::Get()->GetServiceReq(0);

    ////微服务代理连接初始化
    XServiceProxy::Get()->Init();

    ////开启自动重连
    XServiceProxy::Get()->Start();

    static XGatewayConfig cur_conf;
    if (XConfigClient::Get()->StartGetConf(0, server_port, &cur_conf,ConfTimer))
        cout << "初始化配置中心成功" << cur_conf.DebugString() << endl;
    //连接配置中心，获取路由配置
    //等待配置获取成功
    //auto confs = XRegisterClient::Get()->GetServcies(CONFIG_NAME, 10);
    //cout << "=================================================" << endl;
    //cout << confs.DebugString() << endl;
    ////配置中心IP获取失败，读取本地配置
    //if (confs.service_size() <= 0)
    //{
    //    cout << "find config service failed!" << endl;
    //}
    //else
    //{
    //    //只取第一个配置中心
    //    auto conf = confs.service()[0];
        //static XGatewayConfig cur_conf;
        //if (XConfigClient::Get()->StartGetConf(
        //    conf.ip().c_str(), conf.port()
        //    , 0, server_port, &cur_conf))
        //    cout << "连接配置中心成功" << cur_conf.DebugString() << endl;
    //}


    //开启路由服务
    XRouterServer service;
    service.set_server_port(server_port);
    service.Start();
    XThreadPool::Wait();
    return 0;
}