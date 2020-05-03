#pragma once
#include "xservice_client.h"

//前置声明
namespace google
{
    namespace protobuf 
    {
        namespace compiler
        {
            class Importer;
            class DiskSourceTree;
        }
    }
}

typedef void(*ConfigTimerCBFunc) ();
class XConfigClient :public XServiceClient
{
public:
    ~XConfigClient();
    static XConfigClient *Get()
    {
        static XConfigClient cc;
        return &cc;
    }

    ConfigTimerCBFunc ConfigTimerCB = 0;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    ///连接配置中心，开始定时器获取配置
    bool StartGetConf(const char *local_ip, int local_port,
        google::protobuf::Message *conf_message, ConfigTimerCBFunc func);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    ///连接配置中心，开始定时器获取配置
    ///@para local_ip  本地IP的地址，如果设置为空串或者NULL，则默认使用连接的地址，
    bool StartGetConf(const char *server_ip, int server_port,
        const char *local_ip, int local_port,
        google::protobuf::Message *conf_message, int timeout_sec = 10);

    ///获取下载的本地参数
    int GetInt(const char *key);
    bool GetBool(const char *key);
    std::string GetString(const char *key);

    ///定时器回调 定时获取配置请求
    virtual void TimerCB();

    ////先调用一次定时器，确保消息及时获取
    virtual bool Init();

    ///等待线程退出 
    void Wait();
   
    /////////////////////////////////////////////////////////////////////
    ///发出获取配置请求 IP如果为NULL 则取连接配置中心的地址
    void LoadConfig(const char *ip, int port);

    ///获取配置列表（已缓存）中的配置，会复制一份到out_conf
    bool GetConfig(const char *ip, int port, xmsg::XConfig *out_conf,int timeout_ms = 100);

    ///响应获取自己的配置  存储到本地
    void LoadConfigRes(xmsg::XMsgHead *head, XMsg *msg);

    //设置当前配置的对象
    void SetCurServiceMessage(google::protobuf::Message *message);

    /////////////////////////////////////////////////////////////////////
    //// 载入proto文件 线程不安全
    ///@para filename 文件路径
    ///@para class_name 配置的类型
    ///@para out_proto_code 读取到的代码，包含空间和版本
    ///@return 返回动态创建的message ，如果失败返回NULL，第二次调用会释放上一次空间
    google::protobuf::Message *LoadProto(std::string filename, std::string class_name,std::string &out_proto_code);


    static void RegMsgCallback()
    {
        //RegCB(xmsg::MSG_SAVE_CONFIG_RES, (MsgCBFunc)&XConfigClient::SendConfigRes);
        RegCB(xmsg::MSG_LOAD_CONFIG_RES, (MsgCBFunc)&XConfigClient::LoadConfigRes);
        //RegCB(xmsg::MSG_LOAD_ALL_CONFIG_RES, (MsgCBFunc)&XConfigClient::LoadAllConfigRes);
        //RegCB(xmsg::MSG_DEL_CONFIG_RES, (MsgCBFunc)&XConfigClient::DeleteConfigRes);
    }



private:
    xmsg::XLoginRes login_;
    std::mutex login_mutex_;
    XConfigClient();

    //本地微服务的ip和端口
    char local_ip_[16] = { 0 };
    int local_port_ = 0;

    //动态解析proto文件
    google::protobuf::compiler::Importer *importer_ = 0;

    //解析文件的管理对象
    google::protobuf::compiler::DiskSourceTree *source_tree_ = 0;

    //根据proto文件动态创建的的message
    google::protobuf::Message *message_ = 0;

    
};

