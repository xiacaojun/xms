#ifndef XLOG_CLIENT_H
#define XLOG_CLIENT_H

#include "xservice_client.h"
#include <list>
#include<thread>
#include <fstream>
#ifdef _WIN32
#ifdef XCOM_EXPORTS
#define XCOM_API __declspec(dllexport)
#else
#define XCOM_API __declspec(dllimport)
#endif
#else
#define XCOM_API
#endif

//#define LOG(level,level_info,msg) if(level>=CUR_LEVEL)std::cout<<level_info<<":"<<__FILE__<<":"<<__LINE__<<"\n"<<msg<<std::endl;
//#define LOGDEBUG(msg) LOG(XMS_LOG_LEVEL_DEBUG,"DEBUG",msg);
//#define LOGINFO(msg) LOG(XMS_LOG_LEVEL_INFO,"INFO",msg);
//#define LOGERROR(msg) LOG(XMS_LOG_LEVEL_ERROR,"ERROR",msg);
namespace xms
{
    XCOM_API void XLog(xmsg::XLogLevel level, std::string msg,const char *filename,int line);
}
#define LOGDEBUG(msg) xms::XLog(xmsg::XLOG_DEBUG,msg,__FILE__,__LINE__);
#define LOGINFO(msg) xms::XLog(xmsg::XLOG_INFO,msg,__FILE__,__LINE__);
#define LOGERROR(msg) xms::XLog(xmsg::XLOG_ERROR,msg,__FILE__,__LINE__);
#define LOGFATAL(msg) xms::XLog(xmsg::XLOG_FATAL,msg,__FILE__,__LINE__);

class  XCOM_API XLogClient :public XServiceClient
{
public:
    static XLogClient *Get()
    {
        static XLogClient xc;
        return &xc;
    }
    void AddLog(const xmsg::XAddLogReq *req);

    //初始化日志客户端 在注册中心客户端初始化好后注册
    bool StartLog();
    /////////////////////////////////////////
    ///定时器回调函数
    virtual void TimerCB();
    XLogClient();
    ~XLogClient();
    void set_log_level(xmsg::XLogLevel level) { log_level_ = level; }
    void set_service_name(std::string name) { service_name_ = name; }
    void set_service_port(int port) { service_port_ = port; }
    void set_is_print(bool isp) { is_print_ = isp; }
    void set_local_file(std::string local_file) 
    {
        log_ofs_.open(local_file);
    }

private:
    std::string service_name_ = "";
    int service_port_ = 0;
    std::list<xmsg::XAddLogReq> logs_;
    std::mutex logs_mutex_;
    xmsg::XLogLevel log_level_ = xmsg::XLOG_INFO;
    std::ofstream log_ofs_;

    //是否输出屏幕
    bool is_print_ = true;

};

#endif