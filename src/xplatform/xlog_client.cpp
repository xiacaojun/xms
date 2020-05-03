#include "xlog_client.h"
#include "xtools.h"
#include "xmsg_com.pb.h"
using namespace xmsg;
using namespace std;
namespace xms
{
    void XLog(xmsg::XLogLevel level, std::string msg, const char *filename, int line)
    {
         XAddLogReq req;
         req.set_log_level(level);
         req.set_log_txt(msg);
         req.set_filename(filename);
         req.set_line(line);
         XLogClient::Get()->AddLog(&req);
    }
}

void XLogClient::AddLog(const xmsg::XAddLogReq *req)
{
    if (!req)return;
    if (req->log_level() < log_level_)
        return;

    
    string level_str = "Debug";
    switch (req->log_level())
    {
    case XLOG_DEBUG:
        level_str = "DEBUG";
        break;
    case XLOG_INFO:
        level_str = "INFO";
        break;
    case XLOG_ERROR:
        level_str = "ERROR";
        break;
    case XLOG_FATAL:
        level_str = "FATAL";
        break;
    default:
        break;
    }
    string log_time = XGetTime(0, "%F %T");

    // DEBUG:c:\\xdisk_lesson_test\\src\\xplatform\\xservice.cpp:33
    // accept client ip : 127.0.0.1 port : 58291
    stringstream log_text;
    log_text << "=============================================================\n";
    log_text << log_time<<" "<<level_str << "|" << req->filename() << ":" << req->line() << "\n";
    log_text << req->log_txt();

    if (is_print_)
    {
        //cout << "-------------------------------------------------------------" << endl;
        //cout << req->DebugString();
        cout << log_text.str() << endl;

    }
        
    if (log_ofs_)
    {
        log_ofs_ << "==========================================\n";
        log_ofs_ << req->DebugString();
    }
    XAddLogReq tmp = *req;
    if (tmp.log_time() <= 0)
    {
        tmp.set_log_time(time(0));
    }

    tmp.set_service_port(service_port_);
    tmp.set_service_name(service_name_);

    XMutex mux(&logs_mutex_);
    logs_.push_back(tmp);

}

void XLogClient::TimerCB()
{
    //任务过多，要考虑是否会阻塞其他任务事件
    //客户端是独立的线程池，不影响其他业务
    for (;;)
    {
        XAddLogReq log;
        {
        XMutex mux(&logs_mutex_);
        //取出一个
        if (logs_.empty())
        {
            return;
        }

        log = logs_.front();
        logs_.pop_front();
        }
        SendMsg(MSG_ADD_LOG_REQ,&log);
    }
    
}
bool XLogClient::StartLog()
{
    // 注册回调函数
    // 通过注册中心获取日志服务器
        //注册消息回调函数
    //发送消息到服务器
    //服务器连接是否成功？
    ///注册中心的IP，注册中心的端口
    //_CRT_SECURE_NO_WARNINGS
    if (strlen(server_ip()) == 0)
        set_server_ip("127.0.0.1");
    if (server_port() <= 0)
        set_server_port(XLOG_PORT);

    //设置自动重连
    set_auto_connect(true);

    //设定任务处理定时器
    set_timer_ms(100);

    //把任务加入到线程池中
    StartConnect();

    //LoadLocalFile();
    return true;
}
XLogClient::XLogClient()
{
}


XLogClient::~XLogClient()
{
}
