#include "xconfig_client.h"
#include "xtools.h"
#include "xlog_client.h"
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>
#include <map>
#include <thread>
#include <fstream>
using namespace std;
using namespace google;
using namespace protobuf;
using namespace compiler;
using namespace xmsg;
#define PB_ROOT "root/"

//key ip_port
static map<string, XConfig> conf_map;
static mutex conf_map_mutex;

//存储当前微服务配置
static google::protobuf::Message *cur_service_conf = 0;
static mutex cur_service_conf_mutex;

//存储获取的配置列表
static XConfigList *all_config = 0;
static mutex all_config_mutex;

///获取下载的本地参数
int XConfigClient::GetInt(const char *key)
{
    XMutex mux(&cur_service_conf_mutex);
    if (!cur_service_conf)return 0;
    auto field = cur_service_conf->GetDescriptor()->FindFieldByName(key);
    if (!field)
    {
        return 0;
    }
    return cur_service_conf->GetReflection()->GetInt32(*cur_service_conf, field);
}
bool XConfigClient::GetBool(const char *key)
{
    XMutex mux(&cur_service_conf_mutex);
    if (!cur_service_conf) return false;
    cout<<"begin *****************************************"<<endl;
    cout<<cur_service_conf->DebugString();
    cout<<"end *****************************************"<<endl;
    //获取字段
    auto field = cur_service_conf->GetDescriptor()->FindFieldByName(key);
    if (!field)
    {
        return false;
    }
    return cur_service_conf->GetReflection()->GetBool(*cur_service_conf, field);
}
std::string XConfigClient::GetString(const char *key)
{

    XMutex mux(&cur_service_conf_mutex);
    if (!cur_service_conf)return "";
    //获取字段
    auto field = cur_service_conf->GetDescriptor()->FindFieldByName(key);
    if (!field)
    {
        return "";
    }
    return cur_service_conf->GetReflection()->GetString(*cur_service_conf, field);
}
//设置当前配置的对象
void XConfigClient::SetCurServiceMessage(google::protobuf::Message *message)
{
    XMutex mux(&cur_service_conf_mutex);
    cur_service_conf = message;
}
bool XConfigClient::StartGetConf(const char *local_ip, int local_port,
    google::protobuf::Message *conf_message, ConfigTimerCBFunc func)
{
    //注册消息回调函数
    RegMsgCallback();

    //_CRT_SECURE_NO_WARNINGS
    if (local_ip)
        strncpy(local_ip_, local_ip, 16);
    local_port_ = local_port;

    //设置当前配置类型
    SetCurServiceMessage(conf_message);

    this->ConfigTimerCB = func;

    this->set_timer_ms(3000);

    //读取本地缓存
    stringstream ss;
    ss << local_port_ << "_conf.cache";
    ifstream ifs;
    ifs.open(ss.str(), ios::binary);
    if (!ifs.is_open())
    {
        LOGDEBUG("load local config failed!");
    }
    else
    {
        if(conf_message)
            cur_service_conf->ParseFromIstream(&ifs);
        ifs.close();
    }



    //连接配置中心任务加入到线程池
    StartConnect();
    return true;
}
///连接配置中心，开始定时器获取配置
bool XConfigClient::StartGetConf(
    const char *server_ip, int server_port,
    const char *local_ip, int local_port, 
    google::protobuf::Message *conf_message, int timeout_sec)
{
    //注册消息回调函数
    RegMsgCallback();

    //设置配置中心的IP和端口
    set_server_ip(server_ip);
    set_server_port(server_port);

    //_CRT_SECURE_NO_WARNINGS
    if(local_ip)
        strncpy(local_ip_,local_ip,16);
    local_port_ = local_port;

    //设置当前配置类型
    SetCurServiceMessage(conf_message);

    //连接配置中心任务加入到线程池
    StartConnect();


    //等待连接配置中心成功 如果第一次连接失败，不会把定时配置加入线程池，需要调整为定时自动重连，发送配置获取消息？
    if (!WaitConnected(timeout_sec))
    {
        cout << "连接配置中心失败" << endl;
        return false;
    }
    if (local_port_ > 0)
        LoadConfig(local_ip_, local_port_);
    //设定获取配置的定时时间（毫秒）
    SetTimer(3000);

    return true;
}
bool XConfigClient::Init()
{
    XServiceClient::Init();
    
    //先调用一次定时器，确保消息及时获取
    TimerCB();
    return true;
}
void XConfigClient::TimerCB()
{
    if (ConfigTimerCB)
        ConfigTimerCB();
    //发出获取配置的请求，需要考虑自动重连问题？
    if(local_port_ > 0)
        LoadConfig(local_ip_, local_port_);
}


void XConfigClient::Wait()
{
    XThreadPool::Wait();
}


//获取配置请求 IP如果为NULL 则取连接配置中心的地址
void XConfigClient::LoadConfig(const char *ip, int port)
{
    LOGDEBUG("获取配置请求");
    if (port < 0 || port>65535)
    {
        LOGDEBUG("LoadConfig failed!port error");
        return;
    }
    {
        //清理了上一次的配置 不清理可能照成获取的是旧数据
        //stringstream key;
        //key << ip << "_" << port;
        //XMutex mux(&cur_service_conf_mutex);
        //conf_map.erase(key.str());
    }
    XLoadConfigReq req;
    if(ip) //IP如果为NULL 则取连接配置中心的地址
        req.set_service_ip(ip);
    req.set_service_port(port);
    //发送消息到服务端
    SendMsg(MSG_LOAD_CONFIG_REQ, &req);
}

///获取配置列表（已缓存）中的配置，会复制一份到out_conf
bool XConfigClient::GetConfig(const char *ip, int port, xmsg::XConfig *out_conf,int timeout_ms)
{
    //十毫秒判断一次
    int count = timeout_ms / 10;
    stringstream key;
    key << ip << "_" << port;

    for (int i = 0; i < count; i++)
    {
        XMutex mutex(&conf_map_mutex);
        //查找配置
        auto conf = conf_map.find(key.str());
        if (conf == conf_map.end())
        {
            this_thread::sleep_for(10ms);
            continue;
        }
        //复制配置
        out_conf->CopyFrom(conf->second);
        return true;
    }
    LOGDEBUG("Can`t find conf");
    return false;
}

///响应获取自己的配置  存储到本地
void XConfigClient::LoadConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("获取配置响应");
    XConfig conf;
    if (!conf.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoadConfigRes conf.ParseFromArray failed!");
        return;
    }
    LOGDEBUG(conf.DebugString().c_str());
    //key ip_port
    stringstream key;
    key<<conf.service_ip()<<"_"<<conf.service_port();
    //更新配置
    conf_map_mutex.lock();
    conf_map[key.str()] = conf;
    conf_map_mutex.unlock();
   
    //没有本地配置
    if (local_port_ <= 0 || !cur_service_conf)
        return;
    stringstream local_key;
    string ip = local_ip_;
    if (ip.empty())
    {
        ip = conf.service_ip();
    }
    local_key << ip << "_" << local_port_;
    if (key.str() != local_key.str())
    {
        return;
    }
    XMutex mux(&cur_service_conf_mutex);
    if (!cur_service_conf && !cur_service_conf->ParseFromString(conf.private_pb()))
    {
        return;
    }
    LOGDEBUG(cur_service_conf->DebugString().c_str());
    //存储到本地文件
    //文件名 [port]_conf.cache  20030_conf.cache
    stringstream ss;
    ss << local_port_ << "_conf.cache";
    ofstream ofs;
    ofs.open(ss.str(), ios::binary);
    if (!ofs.is_open())
    {
        LOGDEBUG("save local config failed!");
        return;
    }
    cur_service_conf->SerializePartialToOstream(&ofs);
    ofs.close();
}

//显示解析的语法错误
class ConfError:public MultiFileErrorCollector
{
public:
    void AddError(const std::string& filename, int line, int column,
        const std::string& message)
    {
        stringstream ss;
        ss << filename << "|" << line << "|" << column << "|" << message;
        LOGDEBUG(ss.str().c_str());
    }
};
static ConfError conf_error;
/////////////////////////////////////////////////////////////////////
//// 载入proto文件 线程不安全
///@para filename 文件路径
///@para class_name 配置的类型
Message *XConfigClient::LoadProto(std::string filename, std::string class_name, std::string &out_proto_code)
{
    //需要空间清理 
    delete importer_;
    importer_ = new Importer(source_tree_, &conf_error);
    if (!importer_)
    {
        return NULL;
    }
    //1 加载proto文件
    string path = PB_ROOT;
    path += filename;
    //path = filename;
    //返回proto文件描述符
    auto file_desc = importer_->Import(path);
    if (!file_desc)
    {
        return NULL;
    }
    LOGDEBUG(file_desc->DebugString());
    stringstream ss;
    ss << filename << "proto 文件加载成功";
    LOGDEBUG(ss.str().c_str());
    
    //获取类型描述符
    //如果class_name为空，则使用第一个类型
    const Descriptor *message_desc = 0;
    if (class_name.empty())
    {
        if (file_desc->message_type_count() <= 0)
        {
            LOGDEBUG("proto文件中没有message");
            return NULL;
        }
        //取第一个类型
        message_desc = file_desc->message_type(0);
    }
    else
    {
        //包含命名空间的类名 xmsg.XDirConfig
        string class_name_pack = "";
        //查找类型 命名空间，是否要用户提供

        //用户没有提供命名空间 
        if (class_name.find('.') == class_name.npos)
        {
            if (file_desc->package().empty())
            {
                class_name_pack = class_name;
            }
            else
            {
                class_name_pack = file_desc->package();
                class_name_pack += ".";
                class_name_pack += class_name;
            }
        }
        else
        {
            class_name_pack = class_name;
        }
        message_desc = importer_->pool()->FindMessageTypeByName(class_name_pack);

        if (!message_desc)
        {
            string log = "proto文件中没有指定的message ";
            log += class_name_pack;
            LOGDEBUG(log.c_str());
            return NULL;
        }
    }
   
    LOGDEBUG(message_desc->DebugString());

    //反射生成message对象

    //动态创建消息类型的工厂，不能销毁，销毁后由此创建的message也失效
    static DynamicMessageFactory factory;
    
    //创建一个类型原型
    auto message_proto = factory.GetPrototype(message_desc);
    delete message_;
    message_ = message_proto->New();
    LOGDEBUG(message_->DebugString());
    /*
    syntax="proto3";	//版本号
    package xmsg;		//命名空间
    message XDirConfig
    {
        string root = 1;
    }
    */
    
    //auto enum_type = message_desc->enum_type(0);
    //cout << enum_type->DebugString();
    //syntax="proto3";	//版本号
    out_proto_code = "syntax=\"";
    out_proto_code += file_desc->SyntaxName(file_desc->syntax());
    out_proto_code += "\";\n";
    //package xmsg;		//命名空间
    out_proto_code += "package ";
    out_proto_code += file_desc->package();
    out_proto_code += ";\n";

    map<string, const EnumDescriptor*> enum_descs;
    //添加依赖的枚举
    for (int i = 0; i < message_desc->field_count(); i++)
    {
        auto field = message_desc->field(i);
        if (field->type() == FieldDescriptor::TYPE_ENUM)
        {
            if (enum_descs.find(field->enum_type()->name()) != enum_descs.end())
                continue;
            enum_descs[field->enum_type()->name()] = field->enum_type();
            out_proto_code += field->enum_type()->DebugString();
            out_proto_code += "\n";
        }

    }

    //message XDirConfig
    out_proto_code += message_desc->DebugString();
    return message_;
}

XConfigClient::XConfigClient()
{
    //文件加载路径
   source_tree_ = new DiskSourceTree();
   source_tree_->MapPath("", "");
   //使用绝对路径时，不加root会失败
   source_tree_->MapPath(PB_ROOT, "");
}

XConfigClient::~XConfigClient()
{
}
