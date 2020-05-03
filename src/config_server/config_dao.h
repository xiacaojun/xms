#pragma once

#include "xmsg_com.pb.h"
namespace LX {
    class  LXMysql;
}
class ConfigDao
{
public:
    static ConfigDao *Get()
    {
        static ConfigDao dao;
        return &dao;
    }
    ///初始化数据库
    //bool Init(const char *ip, const char *user, const char*pass, const char*db_name, int port = 3306);
    bool Init();

    ///安装数据库的表
    bool Install();

    ///保存配置，如果以有，就更新
    bool SaveConfig(const xmsg::XConfig *conf);

    //读取配置
    xmsg::XConfig LoadConfig(const char *ip, int port);

    ///////////////////////////////////////////////////////
    ///读取分页的配置列表
    ///@para page 从 1开始
    ///@para page_count 每页数量
    xmsg::XConfigList LoadAllConfig(int page, int page_count);

    //删除指定的配置
    bool DeleteConfig(const char *ip, int port);

    virtual ~ConfigDao();
private:
    //mysql数据库的上下文
    LX::LXMysql *my_ = 0;
    ConfigDao();
};

