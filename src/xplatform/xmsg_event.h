#pragma once
#include "xmsg_type.pb.h"
#include "xmsg.h"
#include "xcom_task.h"
#include "xmsg_com.pb.h"


//不调用bufferevent接口，直接调用XComTask的封装
class XCOM_API XMsgEvent:public XComTask
{
public:
   
    virtual void DropInMsg() { is_drop_ = true; };

    ////接收消息，分发消息
    virtual void ReadCB();

    //消息回调函数，默认发送到用户注册的函数，路由重载
    //返回false 则退出本次消息循环
    virtual void ReadCB(xmsg::XMsgHead *head, XMsg *msg);





    //////////////////////////////////
    /// 发送消息，包含头部（自动创建）
    /// @type 消息类型
    /// @message 消息内容
    /// @return 发送错误，比如bev未设置
    virtual bool  SendMsg(xmsg::MsgType type, const google::protobuf::Message *message);
    virtual bool  SendMsg(xmsg::XMsgHead *head, const google::protobuf::Message *message);
    virtual bool  SendMsg(xmsg::XMsgHead *head, XMsg *msg);
    //virtual bool  SendLog(const google::protobuf::Message *message);

    /////////////////////////////////////
    /// 清理缓存消息头和消息内容，用于接收下一次消息
    void Clear();

    void Close();

    typedef void (XMsgEvent::*MsgCBFunc) (xmsg::XMsgHead *head, XMsg *msg);
    
    ////////////////////////////////////////////////////
    /// 添加消息处理的回调函数，根据消息类型分发 ,同一个类型只能有一个回调函数
    /// @para type 消息类型
    /// @para func 消息回调函数
    static void RegCB(xmsg::MsgType type, MsgCBFunc func);


private:

    //////////////////////////////////////////
    /// 接收数据包，
    /// 1 正确接收到消息  (调用消息处理函数)
    /// 2 消息接收不完整 (等待下一次接收) 
    /// 3 消息接收出错 （退出清理空间）
    /// @return 1 2 返回true 3返回false
    bool RecvMsg();

    /////////////////////////////////////////
    /// 获取接收到的数据包，（不包含头部消息）
    /// 由调用者清理XMsg
    /// @return 如果没有完整的数据包，返回NULL
    XMsg *GetMsg();

    bool is_drop_ = false;
    XMsg head_; //消息头
    XMsg msg_;  //消息内容

    //pb消息头
    xmsg::XMsgHead *pb_head_ = 0;
};

