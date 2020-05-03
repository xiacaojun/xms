#pragma once

namespace LX {
    class  LXMysql;
}
namespace xmsg {
    class  XAddLogReq;
}
class XLogDAO
{
public:
    ~XLogDAO();
    static XLogDAO *Get()
    {
        static XLogDAO xd;
        return &xd;
    }
    bool Init();

    ///安装数据库的表
    bool Install();

    bool AddLog(const xmsg::XAddLogReq *req);
private:
    //mysql数据库的上下文
    LX::LXMysql *my_ = 0;

    XLogDAO();
};

