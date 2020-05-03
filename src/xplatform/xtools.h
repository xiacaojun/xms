#pragma once
#ifdef _WIN32
#ifdef XCOM_EXPORTS
#define XCOM_API __declspec(dllexport)
#else
#define XCOM_API __declspec(dllimport)
#endif
#else
#define XCOM_API
#endif
#include <string>
#include <iostream>
#include <mutex>
#include <sstream>
#include <vector>
#include <list>
#include "xlog_client.h"
using namespace std;

XCOM_API std::string GetDirData(std::string path);

struct XToolFileInfo
{
    std::string filename = "";
    long long filesize = 0;
    bool is_dir = false;
    long long time_write = 0;// 修改的时间戳
    std::string time_str = "";      //修改时间字符串 2020-01-22 19:30:13
};

//获取目录列表   //格式 文件名，文件大小（byte），是否是目录（0,1），文件修改时间（2020-01-22 19:30:13），
XCOM_API std::list< XToolFileInfo > GetDirList(std::string path);

//删除文件
XCOM_API void XDelFile(std::string path);

//创建目录
XCOM_API void XNewDir(std::string path);


XCOM_API void XStringSplit(std::vector<std::string> &vec, std::string str, std::string find);


XCOM_API void XStrReplace(std::string str, std::string str_find, std::string str_replace, std::string &sreturn);


XCOM_API std::string XFormatDir(const std::string &dir);

XCOM_API std::string XTrim(const std::string& s);

XCOM_API bool XFileExist(const std::string& s);

/////////////////////////////////////////////////////////////
///生成md5 128bit(16字节) 
///@para in_data 输入数据
///@para in_data_size 输入数据字节数
///@para out_md 输出的MD5数据 （16字节）
XCOM_API unsigned char *XMD5(const unsigned char *in_data, unsigned long in_data_size, unsigned char *out_md);

/////////////////////////////////////////////////////////////
///生成md5_base64  (24字节) 再经过base64转化为字符串
///@para in_data 输入数据
///@para in_data_size 输入数据字节数
///@return  输出的MD5 base64 数据 （24字节）
XCOM_API std::string XMD5_base64(const unsigned char *in_data, unsigned long in_data_size);


/////////////////////////////////////////////////////////////
///生成base64 返回编码后字节大小
XCOM_API  int Base64Encode(const unsigned char *in, int len, char *out_base64);

/////////////////////////////////////////////////////////////
///解码base64 返回解码字节大小
XCOM_API  int Base64Decode(const char *in, int len, unsigned char *out_data);


/*
%a 星期几的简写
%A 星期几的全称
%b 月分的简写
%B 月份的全称
%c 标准的日期的时间串
%C 年份的后两位数字
%d 十进制表示的每月的第几天
%D 月/天/年
%e 在两字符域中，十进制表示的每月的第几天
%F 年-月-日
%g 年份的后两位数字，使用基于周的年
%G 年分，使用基于周的年
%h 简写的月份名
%H 24小时制的小时
%I 12小时制的小时
%j 十进制表示的每年的第几天
%m 十进制表示的月份
%M 十时制表示的分钟数
%n 新行符
%p 本地的AM或PM的等价显示
%r 12小时的时间
%R 显示小时和分钟：hh:mm
%S 十进制的秒数
%t 水平制表符
%T 显示时分秒：hh:mm:ss
%u 每周的第几天，星期一为第一天 （值从0到6，星期一为0）
%U 第年的第几周，把星期日做为第一天（值从0到53）
%V 每年的第几周，使用基于周的年
%w 十进制表示的星期几（值从0到6，星期天为0）
%W 每年的第几周，把星期一做为第一天（值从0到53）
%x 标准的日期串
%X 标准的时间串
%y 不带世纪的十进制年份（值从0到99）
%Y 带世纪部分的十制年份
%z，%Z 时区名称，如果不能得到时区名称则返回空字符。
%% 百分号下面的程序则显示当前的完整日期：
*/
XCOM_API std::string XGetTime(int timestamp = 0, std::string fmt = "%F %R");

XCOM_API std::string XGetSizeString(long long size);

XCOM_API std::string XGetIconFilename(std::string filename,bool is_dir=false);

//通过域名 返回IP地址 只取第一个
// windows C:\Windows\System32\drivers\etc
// Linux /etc/hosts
// 127.0.0.1 xms_gateway_server
// 127.0.0.1 xms_register_server
XCOM_API std::string XGetHostByName(std::string host_name);



//得到目录字节大小
XCOM_API long long GetDirSize(const char * path);

/////////////////////////////////////////////////////////////////////
/// 得到磁盘空间大小
/// @para dir 磁盘路径 C:/test   /root
/// @para avail 用户可用空间 字节数
/// @para total 磁盘空间 字节数
/// @para free 磁盘剩余空间 字节数
XCOM_API bool GetDiskSize(const char *dir, unsigned long long *avail, unsigned long long *total, unsigned long long *free);



class XMsg;
namespace xmsg
{
    class XMsgHead;
}
XCOM_API  void PrintMsg(xmsg::XMsgHead *head, XMsg *msg);
#define XMUTEX(s) XMutex tmp_mutex(s,#s)
class XCOM_API XMutex
{
public:
     static bool is_debug;
     XMutex(std::mutex *mux);
     XMutex(std::mutex *mux, std::string msg);
     ~XMutex();
private:
    int index_ = 0;
    std::string msg_ = "";
    std::mutex *mux_ = 0;

};

//AES 秘钥
//if (bits != 128 && bits != 192 && bits != 256)


class XCOM_API XAES
{
public:
    static XAES* Create();
    ///////////////////////////////////////////////////////
    /// 设置加密秘钥 秘钥长度 128位（16字节） 192位 （24字节） 256位 (32字节)
    /// 长度不能超过32字节，返回失败
    /// 秘钥不足自动补充
    /// @key 秘钥
    /// @key_size 秘钥长度 字节 <=32 会自动补秘钥
    /// @is_enc true  加密 false 解密
    /// @return 设置成功失败
    virtual bool SetKey(const char *key, int key_byte_size, bool is_enc) = 0;
    
    ///清理空间，删除对象
    virtual void Drop() = 0;

    ///////////////////////////////////////////////////////
    /// 加解密
    /// @in 输入数据
    /// @in_size 输入数据大小
    /// @out 输出 数据空间要保证16字节的倍数
    /// @return  输出大小，失败返回<=0
    virtual long long Decrypt(const unsigned char *in, long long in_size, unsigned char *out) = 0;
    virtual long long Encrypt(const unsigned char *in, long long in_size, unsigned char *out) = 0;
};