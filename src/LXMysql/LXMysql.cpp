// LXMysql.cpp : 定义 DLL 应用程序的导出函数。
//
#include <mysql.h>
#include "LXMysql.h"

#include <iostream>
#include <fstream>
using namespace std;

#ifdef _WIN32
#include <conio.h>
#define MYSQL_CONFIG_PATH "xms_mysql_init.conf"
static int GetPassword(char *out,int out_size)
{
    for (int i = 0; i < out_size; i++)
    {
        char p = _getch();
        if (p == '\r' || p == '\n')
        {
            return i;
        }
            
        cout << "*" << flush;
        out[i]= p;
    }
    return 0;
}
#else
#define MYSQL_CONFIG_PATH "/etc/xms_mysql_init.conf"
static int GetPassword(char *out, int out_size)
{
    bool is_begin = false;
    for (int i = 0; i < out_size; )
    {
        system("stty -echo");
        char p = cin.get();
        if (p != '\r' && p != '\n')
            is_begin = true;
        if (!is_begin)continue;
        system("stty echo");
        if (p == '\r' || p == '\n')
            return i;
        cout << "*" << flush;
        out[i] = p;
        i++;
    }
    return 0;
}
#endif
namespace LX {

    struct MysqlInfo
    {
        char host[128] = { 0 };
        char user[128] = {0};
        char pass[128] = { 0 };
        char db_name[128] = { 0 };
        int port = 3306;
    };
    //接收用户输入数据库配置
    bool LXMysql::InputDBConfig()
    {
        if (!mysql && !Init())
        {
            cerr << "InputDBConfig failed! msyql is not init!" << endl;
            return false;
        }
        //string config_path = MYSQL_CONFIG_PATH;
        //如果输入过就不用输入
        ifstream ifs;
        MysqlInfo db;
        ifs.open(MYSQL_CONFIG_PATH, ios::binary);
        if (ifs.is_open())
        {
            ifs.read((char *)&db, sizeof(db));
            if (ifs.gcount() == sizeof(db))
            {
                ifs.close();
                return Connect(db.host, db.user, db.pass, db.db_name,db.port);
            }
            ifs.close();
        }
        cout << "input the db set" << endl;
        cout << "input db host:";
        cin >> db.host;
        cout << "input db user:";
        cin >> db.user;
        cout << "input db pass:";
        //string pass = "";
        GetPassword(db.pass, sizeof(db.pass) - 1);
        //memcpy(db.pass, pass.c_str(), pass.size());
        cout << endl;
        //cin >> db.pass;
        cout << "input db dbname(xms):";
        cin >> db.db_name;
        cout << "input db port(3306):";
        cin >> db.port;
        ofstream ofs;
        ofs.open(MYSQL_CONFIG_PATH, ios::binary);
        if (ofs.is_open())
        {
            ofs.write((char *)&db, sizeof(db));
            ofs.close();
        }

        return Connect(db.host, db.user, db.pass, db.db_name, db.port);
    }
	bool LXMysql::Init()
	{
		Close();
		cout << "LXMysql::Init()" << endl;
		//新创建一个MYSQL 对象
		mysql = mysql_init(0);
		if (!mysql)
		{
			cerr << "mysql_init failed!" << endl;
			return false;
		}
		return true;
	}

	//清理占用的所有资源
	void  LXMysql::Close()
	{
		FreeResult();

		if (mysql)
		{
			mysql_close((MYSQL*)mysql);
			mysql = NULL;
		}
		cout << "LXMysql::Close()" << endl;
	}


	//数据库连接（不考虑线程安全） flag设置支持多条语句
	bool LXMysql::Connect(const char*host, const char*user, const char*pass, const char*db, unsigned short port, unsigned long flag)
	{
		if (!mysql && !Init())
		{
			cerr << "Mysql connect failed! msyql is not init!" << endl;
			return false;
		}
		if (!mysql_real_connect((MYSQL*)mysql, host, user, pass, db, port, 0, flag))
		{
			cerr << "Mysql connect failed!" << mysql_error((MYSQL*)mysql) << endl;
			return false;
		}
		cout << "mysql connect success!" << endl;
		return true;
	}


	bool LXMysql::Query(const char*sql, unsigned long sqllen)
	{
		if (!mysql)
		{
			cerr << "Query failed:mysql is NULL" << endl;
			return false;
		}
		if (!sql)
		{
			cerr << "sql is null" << endl;
			return false;
		}
		if (sqllen <= 0)
			sqllen = (unsigned long)strlen(sql);
		if (sqllen <= 0)
		{
			cerr << "Query sql is empty or wrong format!" << endl;
			return false;
		}

		int re = mysql_real_query((MYSQL*)mysql, sql, sqllen);
		if (re != 0)
		{
			cerr << "mysql_real_query failed!" << mysql_error((MYSQL*)mysql) << endl;
			return false;
		}
		return true;

	}

	//Mysql参数的设定
	bool LXMysql::Options(LX_OPT opt, const void *arg)
	{
		if (!mysql)
		{
			cerr << "Option failed:mysql is NULL" << endl;
			return false;
		}
		int re = mysql_options((MYSQL*)mysql, (mysql_option)opt, arg);
		if (re != 0)
		{
			cerr << "mysql_options failed!" << mysql_error((MYSQL*)mysql) << endl;
			return false;
		}
		return true;
	}

	//连接超时时间
	bool LXMysql::SetConnectTimeout(int sec)
	{
		return Options(LX_OPT_CONNECT_TIMEOUT, &sec);
	}

	//自动重连，默认不自动
	bool LXMysql::SetReconnect(bool isre)
	{
		return Options(LX_OPT_RECONNECT, &isre);
	}

	//返回全部结果
	bool LXMysql::StoreResult()
	{
		if (!mysql)
		{
			cerr << "StoreResult failed:mysql is NULL" << endl;
			return false;
		}
		FreeResult();
		result = mysql_store_result((MYSQL*)mysql);
		if (!result)
		{
			cerr << "mysql_store_result failed!" << mysql_error((MYSQL*)mysql) << endl;
			return false;
		}
		return true;
	}

	//开始接收结果，通过Fetch获取
	bool LXMysql::UseResult()
	{
		if (!mysql)
		{
			cerr << "UseResult failed:mysql is NULL" << endl;
			return false;
		}
		FreeResult();
		result = mysql_use_result((MYSQL*)mysql);
		if (!result)
		{
			cerr << "mysql_use_result failed!" << mysql_error((MYSQL*)mysql) << endl;
			return false;
		}
		return true;
	}

	//释放结果集占用的空间
	void LXMysql::FreeResult()
	{
		if (result)
		{
			mysql_free_result((MYSQL_RES*)result);
			result = NULL;
		}
	}

	//获取一行数据
	std::vector<LXData> LXMysql::FetchRow()
	{
		std::vector<LXData> re;
		if (!result)
		{
			return re;
		}
		MYSQL_ROW row = mysql_fetch_row((MYSQL_RES*)result);
		if (!row)
		{
			return re;
		}

		//列数
		int num = mysql_num_fields((MYSQL_RES*)result);
		
		unsigned long *lens = mysql_fetch_lengths((MYSQL_RES*)result);
		for (int i = 0; i < num; i++)
		{
			LXData data;
			data.data = row[i];
			data.size = lens[i];
			//获取列的类型
			auto field = mysql_fetch_field_direct((MYSQL_RES*)result, i);
			data.type = (FIELD_TYPE)field->type;
			re.push_back(data);
		}
		return re;
	}

	////插入非二进制数据 字段名称前有@ 比如 @time ，其内容不加引号，一般用于调用功能函数
	//生成insert sql语句
	std::string LXMysql::GetInsertSql(XDATA kv, std::string table)
	{
		string sql = "";
		if (kv.empty() || table.empty())
			return "";
		sql = "insert into `";
		sql += table;
		sql += "`";
		//insert into t_video (name,size) values("name1","1024")
		string keys = "";
		string vals = "";

		//迭代map
		for (auto ptr = kv.begin(); ptr != kv.end(); ptr++)
		{
			//字段名
			keys += "`";
			//去掉@
			if (ptr->first[0] == '@')
				keys += ptr->first.substr(1, ptr->first.size() - 1);
			else
				keys += ptr->first;
			keys += "`,";
			if (ptr->first[0] == '@')
			{
				vals += ptr->second.data;
			}
			else
			{
				vals += "'";
				vals += ptr->second.data;
				vals += "'";
			}
			vals += ",";

		}
		//去除多余的逗号
		keys[keys.size() - 1] = ' ';
		vals[vals.size() - 1] = ' ';

		sql += "(";
		sql += keys;
		sql += ")values(";
		sql += vals;
		sql += ")";
		return sql;
	}
	//插入非二进制数据
	bool LXMysql::Insert(XDATA kv, std::string table)
	{
		if (!mysql)
		{
			cerr << "Insert failed:mysql is NULL" << endl;
			return false;
		}
		string sql = GetInsertSql(kv, table);
		cout << sql << endl;
		if (sql.empty())
			return false;
		if (!Query(sql.c_str()))
			return false;
		int num = mysql_affected_rows((MYSQL*)mysql);
		if (num <= 0)
			return false;
		return true;
	}

    //获取上一次插入的ID号
    int LXMysql::GetInsertID()
    {
        if (!mysql)
        {
            cerr << "GetInsertID failed:mysql is NULL" << endl;
            return 0;
        }
        return mysql_insert_id((MYSQL*)mysql);
    }
	//插入二进制数据
	bool LXMysql::InsertBin(XDATA kv, std::string table)
	{
		string sql = "";
		if (kv.empty() || table.empty() || !mysql)
			return false;
		sql = "insert into `";
		sql += table;
		sql += "`";
		//insert into t_video (name,size) values(?,?)
		string keys = "";
		string vals = "";
		//绑定字段
		MYSQL_BIND bind[256] = { 0 };
		int i = 0;
		//迭代map
		for (auto ptr = kv.begin(); ptr != kv.end(); ptr++)
		{
			//字段名
			keys += "`";
			keys += ptr->first;
			keys += "`,";

			vals += "?,";
			bind[i].buffer = (char*)ptr->second.data;
			bind[i].buffer_length = ptr->second.size;
			bind[i].buffer_type = (enum_field_types)ptr->second.type;
			i++;
		}
		//去除多余的逗号
		keys[keys.size() - 1] = ' ';
		vals[vals.size() - 1] = ' ';

		sql += "(";
		sql += keys;
		sql += ")values(";
		sql += vals;
		sql += ")";
		//预处理SQL语句
		MYSQL_STMT *stmt = mysql_stmt_init((MYSQL*)mysql);
		if (!stmt)
		{
			cerr << "mysql_stmt_init failed!" << mysql_error((MYSQL*)mysql) << endl;
			return false;
		}
		if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0)
		{
			mysql_stmt_close(stmt);
			cerr << "mysql_stmt_prepare failed!" << mysql_stmt_error(stmt) << endl;
			return false;
		}

		if (mysql_stmt_bind_param(stmt, bind) != 0)
		{
			mysql_stmt_close(stmt);
			cerr << "mysql_stmt_bind_param failed!" << mysql_stmt_error(stmt) << endl;
			return false;
		}
		if (mysql_stmt_execute(stmt) != 0)
		{
			mysql_stmt_close(stmt);
			cerr << "mysql_stmt_execute failed!" << mysql_stmt_error(stmt) << endl;
			return false;
		}
		mysql_stmt_close(stmt);
		return true;
	}

	//获取更新数据的sql语句 where语句中，用户要包含where
	std::string LXMysql::GetUpdateSql(XDATA kv, std::string table, std::string where)
	{
		//update t_video set name='update001',size=1000 where id=10
		string sql = "";
		if (kv.empty() || table.empty())
			return "";
		sql = "update `";
		sql += table;
		sql += "` set ";
		for (auto ptr = kv.begin(); ptr != kv.end(); ptr++)
		{
			sql += "`";
			sql += ptr->first;
			sql += "`='";
			sql += ptr->second.data;
			sql += "',";
		}
		//去除多余的逗号
		sql[sql.size() - 1] = ' ';
		sql += " ";
		sql += where;
		return sql;
	}

	int LXMysql::Update(XDATA kv, std::string table, std::string where)
	{
		if (!mysql)return -1;
		string sql = GetUpdateSql(kv, table, where);
		if (sql.empty())
			return -1;
		if (!Query(sql.c_str()))
		{
			return -1;
		}
		return mysql_affected_rows((MYSQL*)mysql);
	}

	int LXMysql::UpdateBin(XDATA kv, std::string table, std::string where)
	{
		if (!mysql || kv.empty() || table.empty())
		{
			return -1;
		}
		string sql = "";
		sql = "update `";
		sql += table;
		sql += "` set ";
		MYSQL_BIND bind[256] = { 0 };
		int i = 0;
		for (auto ptr = kv.begin(); ptr != kv.end(); ptr++)
		{
			sql += "`";
			sql += ptr->first;
			sql += "`=?,";
			bind[i].buffer = (char*)ptr->second.data;
			bind[i].buffer_length = ptr->second.size;
			bind[i].buffer_type = (enum_field_types)ptr->second.type;
			i++;
		}
		//去除多余的逗号
		sql[sql.size() - 1] = ' ';
		sql += " ";
		sql += where;

		//预处理SQL语句上下文
		MYSQL_STMT *stmt = mysql_stmt_init((MYSQL*)mysql);
		if (!stmt)
		{
			cerr << "mysql_stmt_init failed!" << mysql_error((MYSQL*)mysql) << endl;
			return -1;
		}
		if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0)
		{
			mysql_stmt_close(stmt);
			cerr << "mysql_stmt_prepare failed!" << mysql_error((MYSQL*)mysql) << endl;
			return -1;
		}

		if (mysql_stmt_bind_param(stmt, bind) != 0)
		{
			mysql_stmt_close(stmt);
			cerr << "mysql_stmt_bind_param failed!" << mysql_stmt_error(stmt) << endl;
			return -1;
		}
		if (mysql_stmt_execute(stmt) != 0)
		{
			mysql_stmt_close(stmt);
			cerr << "mysql_stmt_execute failed!" << mysql_stmt_error(stmt) << endl;
			return -1;
		}
        int count = mysql_stmt_affected_rows(stmt);
		mysql_stmt_close(stmt);
		return count;
	}



	bool LXMysql::StartTransaction()
	{
		return Query("set autocommit=0");
	}

	bool LXMysql::StopTransaction()
	{
		return Query("set autocommit=1");
	}

	bool LXMysql::Commit()
	{
		return Query("commit");
			 
	}
	bool LXMysql::Rollback()
	{
		return Query("rollback");
	}
	//简易接口,返回select的数据结果，每次调用清理上一次的结果集
	XROWS LXMysql::GetResult(const char *sql)
	{
		FreeResult();
		XROWS rows;
		if (!Query(sql))
			return rows;
		if (!StoreResult())
			return rows;
		for (;;)
		{
			auto row = FetchRow();
			if (row.empty())break;
			rows.push_back(row);
		}
		return rows;
	}
}