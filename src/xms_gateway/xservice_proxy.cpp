#include "xservice_proxy.h"
#include "xmsg_com.pb.h"
#include "xtools.h"
#include "xlog_client.h"
#include "xregister_client.h"
#include <thread>
using namespace std;
using namespace xmsg;
///初始化微服务列表（注册中心获取），建立连接
bool XServiceProxy::Init()
{
    //1 从注册中心获取微服务列表
    //测试数据
    //XServiceMap service_map;
    //auto smap = service_map.mutable_service_map();
    //XServiceMap::XServiceList list;
    //{
    //auto service = list.add_service();
    //service->set_ip("127.0.0.1");
    //service->set_port(20011);
    //service->set_name("dir");
    //}
    //{
    //    auto service = list.add_service();
    //    service->set_ip("127.0.0.1");
    //    service->set_port(20012);
    //    service->set_name("dir");
    //}
    //{
    //    auto service = list.add_service();
    //    service->set_ip("127.0.0.1");
    //    service->set_port(20013);
    //    service->set_name("dir");
    //}
    //(*smap)["dir"] = list;
    //cout << service_map.DebugString() << endl;

    //// 与微服务建立连接
    ////遍历XServiceMap数据
    //for (auto m : (*smap))
    //{
    //    client_map_[m.first] = vector<XServiceProxyClient*>();
    //    for (auto s : m.second.service())
    //    {
    //        auto proxy = new XServiceProxyClient();
    //        proxy->set_server_ip(s.ip().c_str());
    //        proxy->set_server_port(s.port());
    //        proxy->StartConnect();
    //        client_map_[m.first].push_back(proxy);
    //        client_map_last_index_[m.first] = 0;
    //    }
    //}
    return true;
}

//清理消息回调
void XServiceProxy::DelEvent(XMsgEvent *ev)
{
    if (!ev)return;
    XMutex mux(&callbacks_mutex_);
    auto call = callbacks_.find(ev);
    if (call == callbacks_.end())
    {
        LOGDEBUG("callbacks_ not find!");
        return;
    }
    call->second->DelEvent(ev);
}
//负载均衡找到客户端连接，进行数据发送
bool XServiceProxy::SendMsg(xmsg::XMsgHead *head, XMsg *msg,XMsgEvent *ev)
{
    if (!head || !msg)return false;
    string service_name = head->service_name();
    
    //MSG_GET_SERVICE //获取微服务，只能获取is_find=true的微服务
    if (head->msg_type() == MSG_GET_OUT_SERVICE_REQ)
    {   
        //1 负载均衡找到客户端连接
        XServiceList services;
        services.set_name(service_name);
        auto client_list = client_map_.find(service_name);
        if (client_list == client_map_.end())
        {
            return ev->SendMsg(head, &services);
        }
        //找到is_find = true的; 和可以连接的
        for (auto c : client_list->second)
        {
            if (!c->is_find() || !c->is_connected())
                continue;
            auto ser = services.add_service();
            ser->set_ip(c->server_ip());
            ser->set_port(c->server_port());
        }
        head->set_msg_type(MSG_GET_OUT_SERVICE_RES);
        return ev->SendMsg(head, &services);
    }



    XMutex mux(&client_map_mutex_);

    //1 负载均衡找到客户端连接
    auto client_list = client_map_.find(service_name);
    if (client_list == client_map_.end())
    {
        stringstream ss;
        ss << service_name << " client_map_ not find!";
        LOGDEBUG(ss.str().c_str());
        return false;
    }

    // 轮询找到可用的微服务连接
    int cur_index = client_map_last_index_[service_name];
    int list_size = client_list->second.size();
    for (int i = 0; i < list_size; i++)
    {
        cur_index++;
        cur_index = cur_index % list_size;
        client_map_last_index_[service_name] = cur_index;
        auto client = client_list->second[cur_index];
        if (client->is_connected())
        {
            //用于退出清理
            XMutex mux(&callbacks_mutex_);
            callbacks_[ev] = client;

            //转发消息
            return client->SendMsg(head, msg, ev);
        }
    }
    LOGDEBUG("can't find proxy");
    return false;
}


//开启自动重连的线程
void XServiceProxy::Start()
{
    thread th(&XServiceProxy::Main, this);
    th.detach();
}

//停止线程
void XServiceProxy::Stop()
{

}

void XServiceProxy::Main()
{
    //自动重连
    while (!is_exit_)
    {

        // 从注册中心获取 微服务的列表更新
        //发送请求到注册中心
        XRegisterClient::Get()->GetServiceReq(0);
        this_thread::sleep_for(200ms);
        auto service_map = XRegisterClient::Get()->GetAllService();
        if (!service_map)
        {
            LOGDEBUG("GetAllService : service_map is NULL");
            this_thread::sleep_for(1s);
            continue;
        }
        auto smap = service_map->service_map();
        if (smap.empty())
        {
            LOGDEBUG("XServiceProxy : service_map->service_map is NULL");
            this_thread::sleep_for(1s);
            continue;
        }

        //LOGINFO("\n=============================\n"+service_map->DebugString());
        //遍历所有的微服务名称列表
        for (auto m : smap)
        {
            //遍历单个微服务
            for (auto s : m.second.service())
            {
                string service_name = m.first;

                //不连接自己
                if (service_name == API_GATEWAY_NAME)
                {
                    continue;
                }

                //此微服务是否已经连接
                XMutex mux(&client_map_mutex_);
                //第一个微服务，创建对象，开启连接
                if (client_map_.find(service_name) == client_map_.end())
                {
                    client_map_[service_name] = std::vector < XServiceProxyClient *>();
                }

                //列表中是否已有此微服务
                bool isfind = false;
                for (auto c : client_map_[service_name])
                {
                    if (s.ip() == c->server_ip() && s.port() == c->server_port())
                    {
                        isfind = true;
                        break;
                    }
                }
                if (isfind)
                    continue;

                //根据类型创建不同的proxy
                auto proxy = XServiceProxyClient::Create(service_name);// new XServiceProxyClient();
                proxy->set_is_find(s.is_find());
                proxy->set_server_ip(s.ip().c_str());
                proxy->set_server_port(s.port());
                //设置关闭后对象自动清理
                proxy->set_auto_delete(false);

                //连接任务加入线程池
                proxy->StartConnect();
                client_map_[service_name].push_back(proxy);
                client_map_last_index_[service_name] = 0;
            }
        }

        // 定时全部重新获取
        for (auto m : client_map_)
        {
            for (auto c : m.second)
            {
                if (c->is_connected())
                    continue;
                if (!c->is_connecting())
                {
                    LOGDEBUG("start conncet service");
                    c->Connect();
                }
            }
        }
        this_thread::sleep_for(3000ms);
    }
}
XServiceProxy::XServiceProxy()
{
}


XServiceProxy::~XServiceProxy()
{
}
