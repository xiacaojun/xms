#ifndef LXMYSQL_H
#define LXMYSQL_H

#include  <vector>
#include  "LXData.h"
//#include <mysql.h>
//struct MYSQL;
//struct MYSQL_RES;
namespace LX {

    //所有函数都不能保证线程安全
    class LXAPI LXMysql
    {
    public:
        bool InputDBConfig();
        int GetInsertID();

        //初始化Mysql API
        bool Init();

        //清理占用的所有资源
        void Close();

        //数据库连接（不考虑线程安全） flag设置支持多条语句
        bool Connect(const char*host, const char*user, const char*pass, const char*db, unsigned short port = 3306, unsigned long flag = 0);

        //执行sql语句  if sqllen=0 strlen获取字符长度
        bool Query(const char*sql, unsigned long sqllen = 0);

        //Mysql参数的设定 Connect之前调用
        bool Options(LX_OPT opt, const void *arg);

        //连接超时时间
        bool SetConnectTimeout(int sec);

        //自动重连，默认不自动
        bool SetReconnect(bool isre = true);

        //结果集获取
        //返回全部结果
        bool StoreResult();

        //开始接收结果，通过Fetch获取
        bool UseResult();

        //释放结果集占用的空间
        void FreeResult();

        //获取一行数据
        std::vector<LXData> FetchRow();

        //生成insert sql语句
        std::string GetInsertSql(XDATA kv, std::string table);

        //插入非二进制数据 字段名称前有@ 比如 @time ，其内容不加引号，一般用于调用功能函数
        bool Insert(XDATA kv, std::string table);

        //插入二进制数据
        bool InsertBin(XDATA kv, std::string table);

        //获取更新数据的sql语句 where语句中，用户要包含where
        std::string GetUpdateSql(XDATA kv, std::string table, std::string where);
        //返回更新数量，失败返回-1
        int Update(XDATA kv, std::string table, std::string where);
        int UpdateBin(XDATA kv, std::string table, std::string where);

        //事务接口
        bool StartTransaction();
        bool StopTransaction();
        bool Commit();
        bool Rollback();

        //简易接口,返回select的数据结果，每次调用清理上一次的结果集
        XROWS GetResult(const char *sql);


    protected:
        //mysql上下文
        void *mysql = 0;

        //结果集
        void *result = 0;

        //字段名称和类型
        //std::vector<LXData> cols;
    };

}

#endif