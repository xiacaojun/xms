#include "config_dao.h"
#include "LXMysql.h"
#include "xtools.h"
#include <string>
using namespace LX;
using namespace std;
using namespace xmsg;
#define CONIG_TABLE "xms_service_config"

static mutex my_mutex;
//删除指定的配置
bool ConfigDao::DeleteConfig(const char *ip, int port)
{
    LOGDEBUG("DeleteConfig");
    XMutex mux(&my_mutex);
    if (!my_)
    {
        LOGERROR("mysql not init");
        return false;
    }
    if (!ip || port <= 0 || port > 65535 || strlen(ip) == 0)
    {
        LOGERROR("DeleteConfig failed!ip or port error");
        return false;
    }
    string table_name = CONIG_TABLE;
    stringstream ss;
    ss << "delete from " << table_name;
    ss << " where service_ip='" << ip << "' and service_port=" << port;
    return my_->Query(ss.str().c_str());
}
///////////////////////////////////////////////////////
///读取分页的配置列表
///@para page 从 1开始
///@para page_count 每页数量
xmsg::XConfigList ConfigDao::LoadAllConfig(int page, int page_count)
{
    XConfigList confs;
    LOGDEBUG("LoadAllConfig");
    XMutex mux(&my_mutex);
    if (!my_)
    {
        LOGERROR("mysql not init");
        return confs;
    }
    if (page <= 0 || page_count <= 0)
    {
        LOGERROR("LoadAllConfig para erorr");
        return confs;
    }
    string table_name = CONIG_TABLE;
    stringstream ss;
    ss << "select `service_name`,`service_ip`,`service_port` from " << table_name;
    //分页处理 select * from table limit 0,10
    //    select * from table limit 10,10      ... 20,10
    ss << " order by id desc";
    ss << " limit " << (page - 1)*page_count << "," << page_count;
    
    LOGDEBUG(ss.str());
    auto rows = my_->GetResult(ss.str().c_str());
    for (auto row : rows)
    {
        //遍历结果集插入到proto类型中
        auto conf = confs.add_config();
        conf->set_service_name(row[0].data);
        conf->set_service_ip(row[1].data);
        conf->set_service_port(atoi(row[2].data));
    }
    return confs;
}

//读取配置
xmsg::XConfig ConfigDao::LoadConfig(const char *ip, int port)
{
    XConfig conf;
    LOGDEBUG("ConfigDao::LoadConfig");
    XMutex mux(&my_mutex);
    if (!my_)
    {
        LOGERROR("mysql not init");
        return conf;
    }
    if (!ip || port <= 0 || port > 65535 || strlen(ip) == 0)
    {
        LOGERROR("LoadConfig failed!ip or port error");
        return conf;
    }
    string table_name = CONIG_TABLE;
    stringstream ss;
    ss << "select private_pb from " << table_name;
    ss << " where service_ip='" << ip << "' and service_port=" << port;
    auto rows = my_->GetResult(ss.str().c_str());
    if (rows.size() == 0)
    {
        stringstream ss;
        ss<<"download config failed!";
        ss<<" not result";
        ss<<ip<<":"<<port;
        LOGDEBUG(ss.str().c_str());
        return conf;
    }
    //只取第一条
    auto row = rows[0];
    if (!conf.ParseFromArray(row[0].data, row[0].size))
    {
        LOGDEBUG("download config failed! ParseFromArray failed!");
        return conf;
    }
    LOGDEBUG("download config success!");
    LOGDEBUG(conf.DebugString());
    return conf;
}

///保存配置，如果以有，就更新
bool ConfigDao::SaveConfig(const xmsg::XConfig *conf)
{
    LOGDEBUG("ConfigDao::SaveConfig");
    XMutex mux(&my_mutex);
    if (!my_)
    {
        LOGERROR("mysql not init");
        return false;
    }
    if (!conf || conf->service_ip().empty())
    {
        LOGERROR("ConfigDao::SaveConfig failed,conf value error!");
        return false;
    }
    string table_name = CONIG_TABLE;
    XDATA data;
    data["service_name"] = LXData(conf->service_name().c_str());
    int port = conf->service_port();
    data["service_port"] = LXData(&port);
    data["service_ip"] = LXData(conf->service_ip().c_str());

    //再序列化一次，把整个XConfig 存入到private_pb
    string private_pb;
    conf->SerializeToString(&private_pb);
    data["private_pb"].data = private_pb.c_str();
    data["private_pb"].size = private_pb.size();
    data["proto"].data = conf->proto().c_str();
    data["proto"].size = conf->proto().size();

    //如果已经由此条数据，则修改数据
    stringstream ss;
    ss<<" where service_ip='";
    ss << conf->service_ip() << "' and service_port=" << conf->service_port();

    string where = ss.str();
    string sql = "select id from ";
    sql += table_name;
    sql += where;
    LOGDEBUG(sql);
    auto rows = my_->GetResult(sql.c_str());
    bool re;
    if (rows.size() > 0)
    {
        int count  = my_->UpdateBin(data, table_name, where);
        if (count>=0)
        {
            LOGDEBUG("配置更新成功！");
            return true;
        }
        LOGDEBUG("配置更新失败！")
        return false;
    }

    re = my_->InsertBin(data, table_name);
    if (re)
    {
        LOGDEBUG("配置插入成功！")
    }
    else
    {
        LOGDEBUG("配置插入失败！")
    }
    return re;
}
///安装数据库的表
bool ConfigDao::Install()
{
    LOGDEBUG("ConfigDao::Install()");

    XMutex mux(&my_mutex);
    if (!my_)
    {
        LOGERROR("mysql not init");
        return false;
    }

    string sql = "";

    //如果表不存在则创建
    sql = "CREATE TABLE IF NOT EXISTS `xms_service_config` ( \
        `id` INT AUTO_INCREMENT,\
        `service_name` VARCHAR(16) ,\
        `service_port` INT ,\
        `service_ip` VARCHAR(16) ,\
        `private_pb` VARCHAR(4096) ,\
        `proto` VARCHAR(4096) ,\
        PRIMARY KEY(`id`));";

    if (!my_->Query(sql.c_str()))
    {
        LOGINFO("CREATE TABLE xms_service_config failed!");
        return false;
    }
    LOGINFO("CREATE TABLE xms_service_config success!");
    return true;
}
///初始化数据库
//bool ConfigDao::Init(const char *ip, const char *user, const char*pass, const char*db_name, int port)
bool ConfigDao::Init()
{
    XMutex mux(&my_mutex);

    if(!my_)
        my_ = new LXMysql();
    if (!my_->Init())
    {
        LOGDEBUG("my_->Init() failed!");
        return false;
    }

    //自动重连
    my_->SetReconnect(true);

    my_->SetConnectTimeout(3);
    //if (!my_->Connect(ip, user, pass, db_name, port))
    if (!my_->InputDBConfig())
    {
        LOGDEBUG("my_->Connect failed!");
        return false;
    }
    LOGDEBUG("my_->Connect success!");

    return true;
}


ConfigDao::ConfigDao()
{
}


ConfigDao::~ConfigDao()
{
}
