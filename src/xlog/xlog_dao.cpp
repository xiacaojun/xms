#include "xlog_dao.h"
#include "xmsg_com.pb.h"
#include "LXMysql.h"
#include "xtools.h"
#include <thread>
using namespace std;
using namespace LX;
using namespace xmsg;
static mutex my_mutex;
bool XLogDAO::AddLog(const xmsg::XAddLogReq *req)
{
    if (!req)return false;
    XDATA data;

    data["service_name"] = req->service_name().c_str();
    data["service_ip"] = req->service_ip().c_str();
    int service_port = req->service_port();
    data["service_port"] = &service_port;
    data["log_txt"] = req->log_txt().c_str();
    int log_time = req->log_time();
    data["log_time"] = &log_time;
    int log_level = req->log_level();
    data["log_level"] = &log_level;
    
    XMutex mux(&my_mutex);
    if (!my_)
    {
        cout << "mysql not init" << endl;
        return false;
    }
    return my_->InsertBin(data, "xms_log");
}
bool XLogDAO::Init()
{
    XMutex mux(&my_mutex);

    if (!my_)
        my_ = new LXMysql();
    if (!my_->Init())
    {
        cout << "my_->Init() failed!" << endl;
        return false;
    }

    //自动重连
    my_->SetReconnect(true);

    my_->SetConnectTimeout(3);
    //if (!my_->Connect(ip, user, pass, db_name, port))
    if (!my_->InputDBConfig())
    {
        cout << "my_->Connect failed!" << endl;
        return false;
    }
    cout << "my_->Connect success!" << endl;
    return true;
}
///安装数据库的表
bool XLogDAO::Install()
{
    cout << "ConfigDao::Install()" << endl;

    XMutex mux(&my_mutex);
    if (!my_)
    {
        cout << "mysql not init" << endl;
        return false;
    }

    string sql = "";
    string table_name = "xms_log";

    //如果表不存在则创建
    sql = "CREATE TABLE IF NOT EXISTS `"+ table_name +"` ( \
        `id` INT AUTO_INCREMENT,\
        `service_name` VARCHAR(16) ,\
        `service_port` INT ,\
        `service_ip` VARCHAR(16) ,\
        `log_txt` VARCHAR(4096) ,\
        `log_time` INT ,\
        `log_level` INT ,\
        PRIMARY KEY(`id`));";

    if (!my_->Query(sql.c_str()))
    {
        cout << "CREATE TABLE "<< table_name <<" failed!" << endl;
        return false;
    }
    cout << "CREATE TABLE " << table_name << " success!" << endl;
    return true;
}
XLogDAO::XLogDAO()
{
}


XLogDAO::~XLogDAO()
{
}
