#include "xregister_handle.h"
#include "xtools.h"
#include "xmsg_com.pb.h"
#include "xlog_client.h"
using namespace xmsg;
using namespace std;

///注册服务列表的缓存
static XServiceMap * service_map = 0;

//多线程访问的锁
static mutex service_map_mutex;

//接收服务的发现请求
void XRegisterHandle::GetServiceReq(xmsg::XMsgHead *head, XMsg *msg)
{
    //暂时只发送全部
    LOGDEBUG("接收服务的发现请求");
    XGetServiceReq req;

    //错误处理
    xmsg::XServiceMap res;
    res.mutable_res()->set_return_(XMessageRes_XReturn_ERROR);

    if (!req.ParseFromArray(msg->data, msg->size))
    {
        stringstream ss;
        ss << "req.ParseFromArray failed!";
        LOGINFO(ss.str().c_str());
        res.mutable_res()->set_msg(ss.str().c_str());
        SendMsg(MSG_GET_SERVICE_RES, &res);
        return;
    }

    string service_name = req.name();
    stringstream ss;
    ss << "GetServiceReq : service name " << service_name;
    LOGDEBUG(ss.str().c_str());
    XServiceMap *send_map = &res;

    //发送全部微服务数据
    service_map_mutex.lock();

    if (!service_map)
        service_map = new XServiceMap();
    //LOGINFO("service_map=" + service_map->DebugString());
    //返回全部
    if (req.type() == XServiceType::ALL)
    {
        send_map = service_map;
        send_map->set_type(XServiceType::ALL);
    }
    else //返回单种
    {
        auto smap = service_map->mutable_service_map();
        if (smap && smap->find(service_name) != smap->end())
        {
            (*send_map->mutable_service_map())[service_name] = (*smap)[service_name];
        }
    }
    service_map_mutex.unlock();

    LOGDEBUG(send_map->DebugString());
    //LOGINFO("service_map2=" + service_map->DebugString());
    //LOGINFO("service_name=" + service_name);

    //LOGINFO("send_map=" + send_map->DebugString());

    //返回单种还是全部
    send_map->set_type(req.type());
    send_map->mutable_res()->set_return_(XMessageRes_XReturn_OK);
    SendMsg(MSG_GET_SERVICE_RES, send_map);
}

//接收服务的注册请求
void XRegisterHandle::RegisterReq(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("服务端接收到用户的注册请求");

    //回应的消息
    XMessageRes res;
    
    //解析请求
    XServiceInfo req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGERROR("XRegisterReq ParseFromArray failed!");
        res.set_return_(XMessageRes::ERROR);
        res.set_msg("XRegisterReq ParseFromArray failed!");
        SendMsg(MSG_REGISTER_RES, &res);
        return;
    }
    
    //接收到用户的服务名称、服务IP、服务端口
    string service_name = req.name();
    if (service_name.empty())
    {
        string error = "service_name is empty!";
        LOGERROR(error.c_str());
        res.set_return_(XMessageRes::ERROR);
        res.set_msg(error);
        SendMsg(MSG_REGISTER_RES, &res);
        return;
    }

    string service_ip = req.ip();
    if (service_ip.empty())
    {
        LOGERROR("service_ip is empty : client ip");
        service_ip = this->client_ip();
    }

    int service_port = req.port();
    if (service_port <= 0 || service_port > 65535)
    {
        stringstream ss;
        //string error = "service_port is error!";
        ss << "service_port is error!" << service_port;
        LOGERROR(ss.str().c_str());
        res.set_return_(XMessageRes::ERROR);
        res.set_msg(ss.str());
        SendMsg(MSG_REGISTER_RES, &res);
        return;
    }
    
    //接收用户注册信息正常
    stringstream ss;
    ss << "接收到用户注册信息:" << service_name << "|" << service_ip << ":" << service_port;
    LOGINFO(ss.str().c_str());

    //存储用户注册信息，如果已经注册需要更新
    {
        XMutex mutex(&service_map_mutex);
        if (!service_map)
            service_map = new XServiceMap();
        //map的指针
        auto smap = service_map->mutable_service_map();

        //是否由同类型已经注册
        //集群微服务
        auto service_list = smap->find(service_name);
        if (service_list == smap->end())
        {
            //没有注册过
            (*smap)[service_name] = XServiceList();
            service_list = smap->find(service_name);
        }
        auto services = service_list->second.mutable_service();
        //查找是否用同ip和端口的
        for (auto service : (*services))
        {
            if (service.ip() == service_ip && service.port() == service_port)
            {
                stringstream ss;
                ss <<service_name<<"|"<< service_ip << ":"
                    << service_port << "微服务已经注册过";
                LOGERROR(ss.str().c_str());
                res.set_return_(XMessageRes::ERROR);
                res.set_msg(ss.str());
                SendMsg(MSG_REGISTER_RES, &res);
                return;
            }
        }
        //添加新的微服务
        auto ser  = service_list->second.add_service();
        ser->set_ip(service_ip);
        ser->set_port(service_port);
        ser->set_name(service_name);
        ser->set_is_find(req.is_find());
        stringstream ss;
        ss << service_name << "|" << service_ip << ":"
            << service_port << "新的微服务注册成功！";
        LOGERROR(ss.str().c_str());
    }

    res.set_return_(XMessageRes::OK);
    res.set_msg("OK");
    SendMsg(MSG_REGISTER_RES, &res);
}

XRegisterHandle::XRegisterHandle()
{
}


XRegisterHandle::~XRegisterHandle()
{
}
