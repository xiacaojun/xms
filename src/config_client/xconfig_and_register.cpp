#include "xconfig_and_register.h"
#include "xregister_client.h"
#include "xconfig_client.h"
#include <string>
using namespace std;
using namespace xmsg;
#define REG  XRegisterClient::Get()
#define CONF XConfigClient::Get()
static void ConfTimer()
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



bool XConfigAndRegister::Init(const char *service_name, 
    const char *service_ip,int service_port,
    const char *register_ip, int register_port, google::protobuf::Message *conf_message)
{ 
    //设置注册中心的IP和端口
    XRegisterClient::Get()->set_server_ip(register_ip);
    XRegisterClient::Get()->set_server_port(register_port);

    //注册到注册中心
    XRegisterClient::Get()->RegisterServer(AUTH_NAME, service_port, service_ip);

    //初始化配置中心
    //XDirConfig tmp_conf;
    CONF->StartGetConf(0, service_port, conf_message, ConfTimer);

    return true;
}