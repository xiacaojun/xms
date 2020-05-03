#include "xregister_client.h"
#include "xlog_client.h"
#include "xmsg_com.pb.h"
#include "xtools.h"
#include <thread>
#include <fstream>
using namespace xmsg;

using namespace std;

///注册服务列表的缓存
static XServiceMap * service_map = 0;
static XServiceMap * client_map = 0;

//多线程访问的锁
static mutex service_map_mutex;


/////////////////////////////////////////////////////////////
///发出有获取微服务列表的请求
///@para service_name == NULL 则取全部
void XRegisterClient::GetServiceReq(const char *service_name)
{
    LOGDEBUG("GetServiceReq");
    XGetServiceReq req;
    if (service_name)
    {
        req.set_type(XServiceType::ONE);
        req.set_name(service_name);
    }
    else
    {
        req.set_type(XServiceType::ALL);
    }

    SendMsg(MSG_GET_SERVICE_REQ, &req);
}

//读取本地缓存
bool XRegisterClient::LoadLocalFile()
{
    if (!service_map)
    {
        service_map = new XServiceMap();
    }
    LOGDEBUG("Load local register data");
    stringstream ss;
    ss << "register_" << service_name_ << service_ip_ << service_port_ << ".cache";
    ifstream ifs;
    ifs.open(ss.str(), ios::binary);
    if (!ifs.is_open())
    {
        stringstream log;
        log << "Load local register data failed!";
        log << ss.str();
        LOGDEBUG(log.str().c_str());
        return false;
    }
    service_map->ParseFromIstream(&ifs);
    ifs.close();
    return true;

}

/////////////////////////////////////////////////////////////////////////////
/// 获取指定服务名称的微服务列表 （阻塞函数）
/// 1 等待连接成功 2 发送获取微服务的消息 3 等待微服务列表消息反馈（有可能拿到上一次的配置）
/// @para service_name 服务名称
/// @para timeout_sec 超时时间
/// @return 服务列表
xmsg::XServiceList XRegisterClient::GetServcies(const char *service_name, int timeout_sec)
{
    xmsg::XServiceList re;
    //十毫米判断一次
    int totoal_count = timeout_sec * 100;
    int count = 0;
    //1 等待连接成功
    while (count < totoal_count)
    {
        //cout << "@" << flush;
        if (is_connected())
            break;
        this_thread::sleep_for(chrono::milliseconds(10));
        count++;
    }
    if (!is_connected())
    {
        LOGDEBUG("连接等待超时");
        XMutex mutex(&service_map_mutex);
        //只有第一次读取缓存
        if (!service_map)
        {
            LoadLocalFile();
        }
        return re;
    }

    //2 发送获取微服务的消息
    GetServiceReq(service_name);
    
    //3 等待微服务列表消息反馈（有可能拿到上一次的配置）
    while (count < totoal_count)
    {
        cout << "." << flush;
        XMutex mutex(&service_map_mutex);
        if (!service_map)
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            count++;
            continue;
        }
        auto m = service_map->mutable_service_map();
        if (!m)
        {
            //cout << "#" << flush;
            //没有找到指定的微服务
            GetServiceReq(service_name);
            this_thread::sleep_for(chrono::milliseconds(100));
            count+=10;
            continue;
        }
        auto s = m->find(service_name);
        if (s == m->end())
        {
           // cout << "+" << flush;
            //没有找到指定的微服务
            GetServiceReq(service_name);
            this_thread::sleep_for(chrono::milliseconds(100));
            count += 10;
            continue;
        }
        re.CopyFrom(s->second);
        return re;
    }
    return re;

    //XMutex mutex(&service_map_mutex);
}

xmsg::XServiceMap *XRegisterClient::GetAllService()
{
    XMutex mutex(&service_map_mutex);
    //LoadLocalFile();
    if (!service_map)
    {
        return NULL;
    }
    if (!client_map)
    {
        client_map = new XServiceMap();
    }
    client_map->CopyFrom(*service_map);
    return client_map;
}

//获取服务列表的响应
void XRegisterClient::GetServiceRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("GetServiceRes");

    XMutex mutex(&service_map_mutex);
    //是否替换全部缓存
    bool is_all = false;
    XServiceMap *cache_map;
    XServiceMap tmp;
    cache_map = &tmp;
    if (!service_map)
    {
        service_map = new XServiceMap();
        cache_map = service_map;
        is_all = true;
    }
    if (!cache_map->ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("service_map.ParseFromArray failed!");
        return;
    }
    
    if(cache_map->type() == XServiceType::ALL)
    {
        is_all = true;
    }

    ///////////////////////////////////////////////////////////
    //内存缓存刷新
    if (cache_map == service_map)
    {
        //存储缓存已经刷新
    }
    else
    {
        if (is_all)
        {
            service_map->CopyFrom(*cache_map);
        }
        else
        {
            /// 将刚读取的cmap数据存入  service_map 内存缓冲
            auto cmap = cache_map->mutable_service_map();

            //取第一个
            if (!cmap || cmap->empty())return;
            auto one = cmap->begin();

            auto smap = service_map->mutable_service_map();
            //修改缓存
            (*smap)[one->first] = one->second;
        }
    }


    ///////////////////////////////////////////////////////////
    //磁盘缓存刷新 后期要考虑刷新频率
    stringstream ss;
    ss << "register_" << service_name_ << service_ip_ << service_port_ << ".cache";
    LOGDEBUG("Save local file!");
    if (!service_map)return;

    ofstream ofs;
    ofs.open(ss.str(), ios::binary);
    if (!ofs.is_open())
    {
        LOGDEBUG("save local file faield!");
        return;
    }
    //缓存要设定有效期
    service_map->SerializePartialToOstream(&ofs);
    ofs.close();

    //LOGDEBUG(service_map->DebugString());
    //区分是获取一种还是全部 刷新缓存
    //一种 只刷新此种微服务列表缓存数据
    
    //全部 刷新所有缓存数据
}

//接收服务的注册响应
void XRegisterClient::RegisterRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("RegisterRes");
    XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XRegisterClient::RegisterRes failed!res.ParseFromArray failed!");
        return;
    }
    if (res.return_() == XMessageRes::OK)
    {
        LOGINFO("RegisterRes success");
        return;
    }
    stringstream ss;
    ss << "RegisterRes failed! " << res.msg();
    LOGINFO(ss.str().c_str());
}

void XRegisterClient::ConnectedCB()
{
    //发送注册消息
    LOGDEBUG("connected start send MSG_REGISTER_REQ ");
    XServiceInfo req;
    req.set_name(service_name_);
    req.set_ip(service_ip_);
    req.set_port(service_port_);
    req.set_is_find(is_find_);
    SendMsg(MSG_REGISTER_REQ, &req);
}
//定时器，用于发送心跳
void XRegisterClient::TimerCB()
{
    static long long count = 0;
    count++;
    XMsgHeart req;
    req.set_count(count);
    SendMsg(MSG_HEART_REQ, &req);
}

///////////////////////////////////////////////////////////////////////
//// 向注册中心注册服务 
/// @para service_name 微服务名称
/// @para port 微服务接口
/// @para ip 微服务的ip，如果传递NULL，则采用客户端连接地址
void XRegisterClient::RegisterServer(const char *service_name, int port, const char *ip, bool is_find)
{
    is_find_ = is_find;
    //注册消息回调函数
    RegMsgCallback();
    //发送消息到服务器
    //服务器连接是否成功？
    ///注册中心的IP，注册中心的端口
    //_CRT_SECURE_NO_WARNINGS
    if (service_name)
        strcpy(service_name_, service_name);
    if (ip)
        strcpy(service_ip_, ip);
    service_port_ = port;

    //设置自动重连
    set_auto_connect(true);

    //设定心跳定时器
    set_timer_ms(3000);

    //添加默认的IP和端口
    if (server_ip()[0] == '\0')
    {
        set_server_ip("127.0.0.1");
    }
    if (server_port() <= 0)
    {
        set_server_port(REGISTER_PORT);
    }

    //把任务加入到线程池中
    StartConnect();

    LoadLocalFile();
}

XRegisterClient::XRegisterClient()
{
}


XRegisterClient::~XRegisterClient()
{
}
