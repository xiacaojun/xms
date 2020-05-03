#include "xmsg_event.h"
#include "xmsg_com.pb.h"
#include "xtools.h"
#include "xlog_client.h"
#include <iostream>
#include <sstream>
#include <map>
using namespace std;
using namespace xmsg;
using namespace google;
using namespace protobuf;

//同一个类型只能有一个回调函数
static map< MsgType, XMsgEvent::MsgCBFunc> msg_callback;
void XMsgEvent::RegCB(xmsg::MsgType type, XMsgEvent::MsgCBFunc func)
{
    if (msg_callback.find(type) != msg_callback.end())
    {
        stringstream ss;
        ss << "RegCB is error," << type << " have been set " << endl;
        LOGERROR(ss.str().c_str());
        return;
    }
    msg_callback[type] = func;
}

void XMsgEvent::ReadCB(xmsg::XMsgHead *head, XMsg *msg)
{
    //回调消息函数
    auto ptr = msg_callback.find(head->msg_type());
    if (ptr == msg_callback.end())
    {
        Clear();
        stringstream ss;
        ss << "msg error func not set!";
        ss << head->msg_type();
        LOGDEBUG(ss.str().c_str());
        return;
    }
    auto func = ptr->second;
    (this->*func)(pb_head_, msg);
}
////接收消息，分发消息
void XMsgEvent::ReadCB()
{
    static int i = 0;
    i++;
    //cout << "<" << i << ">" << flush;
    //如果线程退出
    while (1)
    {
       // cout << "#" << flush;
        if (!RecvMsg())
        {
            Clear();
            break;
        }
        if (!pb_head_)break;
        auto msg = GetMsg();
        if (!msg)break;

        if (pb_head_)
        {
            //cout << "【MSG】" << pb_head_->service_name() << " " << msg->size << " " << msg->type << endl;
            stringstream ss;
            ss << "【RECV】"<<server_ip()<<":"<<server_port()<<"|"<< XGetPortName(server_port())<<" " <<client_ip()<<":"<<client_port()<<" " << pb_head_->DebugString();
            //cout << ss.str() << endl;
            if (pb_head_->msg_type() != xmsg::MSG_ADD_LOG_REQ)
                LOGDEBUG(ss.str().c_str());
        }

        //cout << "service_name = " << pb_head_->service_name() << endl;
        //不加锁访问不安全
        //LOGDEBUG(pb_head_->service_name());
        //msg->head = pb_head_;
        ReadCB(pb_head_, msg);

        Clear();
        if (is_drop_)
        {
            set_auto_delete(true);
            Close();
            return;
        }
    }
    //cout << "<" << 0 << ">" << flush;
}
//////////////////////////////////////////
/// 接收数据包，
/// 1 正确接收到消息  (调用消息处理函数)
/// 2 消息接收不完整 (等待下一次接收) 
/// 3 消息接收出错 （退出清理空间）
/// @return 1 2 返回true 3返回false
bool XMsgEvent::RecvMsg()
{

    //解包
    
    //接收消息头
    if (!head_.size)
    {
        //1 消息头大小
        int len = Read(&head_.size, sizeof(head_.size));//bufferevent_read(bev_, &head_.size, sizeof(head_.size));
        if (len <= 0 || head_.size <= 0)
        {
            return false;
        }

        //分配消息头空间 读取消息头（鉴权，消息大小）
        if (!head_.Alloc(head_.size))
        {
            stringstream ss;
            ss<< "head_.Alloc failed!" << head_.size;
            LOGDEBUG(ss.str().c_str());
            return false;
        }
    }
    //2 开始接收消息头（鉴权，消息大小）
    if (!head_.Recved())
    {
        int len = Read(
            head_.data + head_.recv_size,  //第二次进来 从上次的位置开始读
            head_.size - head_.recv_size
        );
        if (len <= 0)
        {
            return true;
        }
        head_.recv_size += len;

        if (!head_.Recved())
            return true;

        //完整的头部数据接收完成
        //反序列化
        if (!pb_head_)
        {
            pb_head_ = new XMsgHead();
        }
        if (!pb_head_->ParseFromArray(head_.data, head_.size))
        {
            stringstream ss;
            ss << "pb_head.ParseFromArray failed!" <<head_.size;

            LOGDEBUG(ss.str().c_str());
            return false;
        }
        
        //空包数据
        if (pb_head_->msg_size() == 0)
        {
           // cout << "0" << flush;
            //消息类型
            msg_.type = pb_head_->msg_type();
            msg_.size = 0;
            return true;
        }
        else
        {
            //鉴权
            //消息内容大小
            //分配消息内容空间
            if (!msg_.Alloc(pb_head_->msg_size()))
            {
                stringstream ss;
                ss<<"msg_.Alloc failed!msg_size="<<pb_head_->msg_size();

                LOGDEBUG(ss.str().c_str());
                return false;
            }
        }

        //消息类型
        msg_.type = pb_head_->msg_type();
    }

    //3 开始接收消息内容
    if (!msg_.Recved())
    {
        int len = Read(
            msg_.data + msg_.recv_size,  //第二次进来 从上次的位置开始读
            msg_.size - msg_.recv_size
        );
        if (len <= 0)
        {
            return true;
        }
        msg_.recv_size += len;
    }

    if (msg_.Recved())
    {
        cout << "+" << flush;
    }

    return true;
}

/////////////////////////////////////////
/// 获取接收到的数据包，（不包含头部消息）
/// 由调用者清理XMsg
/// @return 如果没有完整的数据包，返回NULL
XMsg *XMsgEvent::GetMsg()
{
    if (msg_.Recved())
        return &msg_;
    return NULL;
}


void XMsgEvent::Close()
{
    Clear();
    XComTask::Close();
}
/////////////////////////////////////
/// 清理缓存消息头和消息内容，用于接收下一次消息
void XMsgEvent::Clear()
{
    head_.Clear();
    msg_.Clear();
}

bool  XMsgEvent::SendMsg(xmsg::XMsgHead *head, XMsg *msg)
{
    //if (!head || !msg)
    if (!head ) //支持只发送头部
        return false;
    head->set_msg_size(msg->size);
    //消息头序列化
    string head_str = head->SerializeAsString();
    int headsize = head_str.size();
    //stringstream ss;
    //ss << "SendMsg" << head->msg_type();
    //LOGDEBUG(ss.str());
    if (head)
    {
        stringstream ss;
        ss << "【SEND】"<<server_ip()<<":"<<server_port() <<" "<< XGetPortName(server_port()) << " " << head->DebugString();
        //cout << ss.str() << endl;
        // 在记录日志会死循环
        if(head->msg_type() != xmsg::MSG_ADD_LOG_REQ)
            LOGDEBUG(ss.str().c_str());
    }
        

    //1 发送消息头大小 4字节 暂时不考虑字节序问题
    int re = Write(&headsize, sizeof(headsize));
    if (!re)return false;

    //2 发送消息头（pb序列化） XMsgHead （设置消息内容的大小）
    re = Write(head_str.data(), head_str.size());
    if (!re)return false;

   
    //支持发送空包
    if (msg->size > 0)
    {
        re = Write(msg->data, msg->size);
        if (!re)return false;
    }
    return true;
}

bool XMsgEvent::SendMsg(xmsg::XMsgHead *head, const Message *message)
{
    if (!message || !head)
        return false;
    ///封包
    //消息内容序列化
    string msg_str = message->SerializeAsString();
    int msg_size = msg_str.size();
    XMsg msg;
    msg.data = (char*)msg_str.data();
    msg.size = msg_size;
    return SendMsg(head, &msg);
    //head->set_msg_size(msg_size);

    ////消息头序列化
    //string head_str = head->SerializeAsString();
    //int headsize = head_str.size();

    ////1 发送消息头大小 4字节 暂时不考虑字节序问题
    //int re = Write(&headsize, sizeof(headsize));
    //if (!re)return false;

    ////2 发送消息头（pb序列化） XMsgHead （设置消息内容的大小）
    //re = Write(head_str.data(), head_str.size());
    //if (!re)return false;

    ////3 发送消息内容 （pb序列化） 业务proto
    //re = Write(msg_str.data(), msg_str.size());
    //if (!re)return false;
    //return true;
}
bool XMsgEvent::SendMsg(MsgType type, const Message *message)
{
    if (!message)
        return false;
    XMsgHead head;
    head.set_msg_type(type);
    return SendMsg(&head, message);
}
