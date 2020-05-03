#ifndef XSERVICES_CLIENT_H
#define XSERVICES_CLIENT_H
#include "xmsg_event.h"
#include "xthread_pool.h"
#include <thread>

/*
使用示例

class XMyServiceClient :public XServiceClient
{
public:
    //发送消息给服务端
    void SendMsgToServer()
    {
        //序列化并发送protbuf消息
        XDirReq req;
        req.set_path("test");
        SendMsg(MSG_DIR_REQ, &req);
    }
    //接收服务端消息
    void SendConfigRes(xmsg::XMsgHead *head, XMsg *msg)
    {
        //反序列化protobuf消息
        XDirRes res;
        if (!res.ParseFromArray(msg->data, msg->size))
        {
            cout<<"res.ParseFromArray failed!"<<endl;
            return;
        }
    }
    //注册接收服务器的消息类型
    static void RegMsgCallback()
    {
        RegCB(xmsg::MSG_SAVE_CONFIG_RES, (MsgCBFunc)&XMyServiceClient::SendConfigRes);
    }
};
int main()
{
    //注册接收服务器的消息类型
    XMyServiceClient::RegMsgCallback();

    //设定服务器IP和端口
    XMyServiceClient client;
    client.set_server_ip("127.0.0.1");
    client.set_server_port(20010);

    //开启连接任务到线程池
    client.StartConnect();

    //发送消息到服务端
    client.SendMsgToServer();

    XThreadPool::Wait();
    return 0;
}

**/

class XCOM_API XServiceClient :public XMsgEvent
{
public:
    XServiceClient();
    virtual ~XServiceClient();
    
    //////////////////////////////////////////////////
    /// 将任务加入到线程池中，进行连接
    virtual void StartConnect();


    //发送消息
    virtual bool SendMsg(xmsg::XMsgHead *head, const google::protobuf::Message *message) override;


    //发送消息
    virtual bool SendMsg(xmsg::MsgType type, const google::protobuf::Message *message) override;

    //发送文件用
    virtual bool SendMsg(xmsg::XMsgHead *head, XMsg *msg) override;

    virtual void set_service_name(std::string name) { service_name_ = name; }
    virtual void set_login(xmsg::XLoginRes *login);

    xmsg::XLoginRes *login() { return login_; }

protected:
    //获取添加了服务名称和登录信息的head 容易产生 dll和调用者内存空间问题
    // xmsg::XMsgHead GetHeadByType(xmsg::MsgType type);



private:

    //设置head的登录信息
    xmsg::XMsgHead *SetHead(xmsg::XMsgHead *head);

    XThreadPool *thread_pool_ = 0;
    
    //微服务名称
    std::string service_name_;

    //登录信息
    xmsg::XLoginRes *login_ = 0;

    std::mutex login_mutex_;


};


#endif