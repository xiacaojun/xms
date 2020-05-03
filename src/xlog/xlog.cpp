// xlog.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "xlog_dao.h"
#include "xlog_server.h"
#include "xlog_client.h"
using namespace std;
using namespace xmsg;
int main(int argc,char *argv[])
{
    cout << "XLog Server " << endl;
    //if (ConfigDao::Get()->Init("localhost", "root", "123456", "xms", 3306))
    if (XLogDAO::Get()->Init())
    {
        cout << "XLogDAO::Get()->Init Success!" << endl;
        //测试安装
        XLogDAO::Get()->Install();
    }
    
    //for (;;)
    //{
    //    XAddLogReq req;
    //    req.set_service_ip("0.0.0.0");
    //    XLogDAO::Get()->AddLog(&req);
    //    this_thread::sleep_for(1s);
    //}

    XLogServer xlog;
    xlog.main(argc, argv);
    xlog.Start();

    //this_thread::sleep_for(200ms);
    //auto log = XLogClient::Get();/*
    //log->set_server_ip("127.0.0.1");
    //log->set_server_port(XLOG_PORT);*/
    //log->StartLog();
    //while (!log->is_connected());
    //XAddLogReq req;
    //req.set_service_ip("0.0.0.1");
    //req.set_log_level(XLOG_ERROR);
    //XLogClient::Get()->AddLog(&req);
    //for (;;)
    //{
    //    cout << log->is_connected() << endl;;
    //    this_thread::sleep_for(3000ms);
    //}
    //for (;;)
    //{
    //    XAddLogReq req;
    //    //req.set_service_ip("0.0.0.1");
    //    req.set_log_level(XLOG_DEBUG);
    //    XLogClient::Get()->AddLog(&req);
    //    this_thread::sleep_for(1s);

    //    //req.set_service_ip("127.0.0.1");
    //    req.set_log_level(XLOG_INFO);
    //    XLogClient::Get()->AddLog(&req);
    //    this_thread::sleep_for(1s);
    //    cout << "$" << flush;
    //}

    XThreadPool::Wait();
    return 0;
}
