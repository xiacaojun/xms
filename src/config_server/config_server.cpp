#include <iostream>
#include "config_dao.h"
#include "xmsg_com.pb.h"
#include "xtools.h"
#include "xconfig_server.h"
using namespace std;
using namespace xmsg;
int main(int argc, char *argv[])
{
    cout << "Config Server" << endl;
    //if (ConfigDao::Get()->Init("localhost", "root", "123456", "xms", 3306))
    if (ConfigDao::Get()->Init())
    { 
        cout << "ConfigDao::Get()->Init Success!" << endl;
        //测试安装
        ConfigDao::Get()->Install();
    }
    auto re = ConfigDao::Get()->LoadAllConfig(1, 10000);
    cout << re.DebugString();

    XConfigServer config;
    config.main(argc, argv);
    config.Start();
    config.Wait();

    ////先测试DAO
    ////初始化数据库
    //if (ConfigDao::Get()->Init("127.0.0.1", "root", "123456", "xms", 3306))
    //{
    //    cout << "ConfigDao::Get()->Init Success!" << endl;
    //    //测试安装
    //    ConfigDao::Get()->Install();


    //    //测试配置保持
    //    XConfig conf;
    //    conf.set_service_name("test1");
    //    conf.set_service_ip("127.0.0.1");
    //    conf.set_service_port(20020);
    //    conf.set_proto("message Test{string name=1;}");
    //    string pb = conf.SerializeAsString();
    //    conf.set_private_pb(pb);

    //    ConfigDao::Get()->SaveConfig(&conf);
    //    auto config = ConfigDao::Get()->LoadConfig("127.0.0.1", 20020);
    //    cout << "========================================" << endl;
    //    LOGDEBUG(config.DebugString());
    //}
    //else
    //{
    //    cout << "ConfigDao::Get()->Init Failed!" << endl;
    //}
    //getchar();

    return 0;
}