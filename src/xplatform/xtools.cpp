#include "xtools.h"

#include <openssl/md5.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include "xmsg_com.pb.h"

#ifdef _WIN32
#include <io.h>
#include <windows.h>  //与protobuf有冲突，要放到后面调用
#include <direct.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/vfs.h>
#define _access access
#define _mkdir(d) mkdir(d,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

#endif

using namespace std;

using namespace xmsg;

bool XMutex::is_debug = false;
static int mutex_index = 0;
static int lock_count = 0;
static int unlock_count = 0;

XMutex::XMutex(std::mutex *mux)
{
    mux_ = mux;
    mux->lock();
    mutex_index++;
    lock_count++;
    this->index_ = mutex_index;
    if (is_debug)
        cout << index_ << "|" << msg_ << ":Lock" << "L/U(" << lock_count << "/" << unlock_count << ")" << endl;
}

XMutex::XMutex(std::mutex *mux, std::string msg)
{
    mux_ = mux;
    msg_ = msg;
    mux->lock();
    mutex_index++;
    lock_count++;
    this->index_ = mutex_index;
    if (is_debug)
        cout << index_<<"|"<< msg_ << ":Lock"<<"L/U("<< lock_count <<"/"<< unlock_count <<")" << endl;
}

XMutex::~XMutex()
{
    mux_->unlock();
    unlock_count++;
    if (is_debug)
        cout << index_ << "|" << msg_ << ":UnLock" << "L/U(" << lock_count << "/" << unlock_count << ")" << endl;
}

void PrintMsg(XMsgHead *head, XMsg *msg)
{
    //stringstream ss;
    //ss << "【MSG】";
    //if (head)
    //{
    //    ss << head->service_name();
    //}
    //cout << "【MSG】" << pb_head_->service_name() << " " << msg->size << " " << msg->type << endl;
    //if (msg)
    //{
    //    int32 msg_size = 1;

    //    //消息类型
    //    MsgType msg_type = 2;

    //    //令牌 如果时登陆消息则未空
    //    string token = 3;

    //    //微服务的名称，用于路由
    //    string service_name = 4;
    //}
}
int Base64Encode(const unsigned char *in, int len, char *out_base64)
{
    if (!in || len <= 0 || !out_base64)
        return 0;
    // 源 内存BIO
    auto mem_bio = BIO_new(BIO_s_mem());
    if (!mem_bio)return 0;

    //过滤 base64 BIO 
    auto b64_bio = BIO_new(BIO_f_base64());
    if (!b64_bio)
    {
        BIO_free(mem_bio);
        return 0;
    }
    //BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);

    //形成BIO链 b64-mem
    BIO_push(b64_bio, mem_bio);

    //往链头写入 base64处理后传到链尾
    BIO_write(b64_bio, in, len);
    
    //刷新缓冲，写入到链表的下一个BIO
    BIO_flush(b64_bio);

    BUF_MEM *p_data;

    //获取内存BIO中数据 
    BIO_get_mem_ptr(b64_bio, &p_data);

    memcpy(out_base64, p_data->data, p_data->length );
    //out_base64[p_data->length - 1] = 0;
    BIO_free_all(b64_bio);
    return p_data->length;
}

int Base64Decode(const char *in, int len, unsigned char *out_data)
{
    if (!in || len <= 0 || !out_data)
        return 0;
    // 源 内存BIO
    auto mem_bio = BIO_new_mem_buf(in, len+1);
    if (!mem_bio)return 0;

    //过滤 base64 BIO
    auto b64_bio = BIO_new(BIO_f_base64());
    if (!b64_bio)return 0;
   // if(in[len-1] !='\0')
    //BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);
    //形成BIO链 b64-mem
    b64_bio = BIO_push(b64_bio, mem_bio);
    BIO_flush(b64_bio);
    size_t size = 0;
    BIO_read_ex(b64_bio, out_data, len, &size);
    
    BIO_free_all(mem_bio);

    return size;
}

//生成md5 128bit(16字节) 
unsigned char *XMD5(const unsigned char *d, unsigned long n, unsigned char *md)
{
    return MD5(d, n, md);
   
}

//生成md5 128bit(16字节) 再经过base64转化为字符串
std::string XMD5_base64(const unsigned char *d, unsigned long n)
{
    unsigned char buf[16] = { 0 };
    char base64[25] = { 0 };
    XMD5(d, n, buf);
    Base64Encode(buf, 16, base64);
    base64[24] = '\0';
    return base64;
}

XCOM_API std::string XGetIconFilename(std::string filename, bool is_dir)
{
    string iconpath = "Other";
    ///文件类型
    string filetype = "";
    int pos = filename.find_last_of('.');
    if (pos > 0)
    {
        filetype = filename.substr(pos + 1);
    }
    //转换为小写 ，第三个参数是输出
    transform(filetype.begin(), filetype.end(), filetype.begin(), ::tolower);

    if (is_dir)
    {
        iconpath = "Folder";
    }
    else if (filetype == "jpg" || filetype == "png" || filetype == "gif")
    {
        iconpath = "Img";
    }
    else if (filetype == "doc" || filetype == "docx" || filetype == "wps")
    {
        iconpath = "Doc";
    }
    else if (filetype == "rar" || filetype == "zip" || filetype == "7z" || filetype == "gzip")
    {
        iconpath = "Rar";
    }
    else if (filetype == "ppt" || filetype == "pptx")
    {
        iconpath = "Ppt";
    }
    else if (filetype == "xls" || filetype == "xlsx")
    {
        iconpath = "Xls";
    }
    else if (filetype == "pdf")
    {
        iconpath = "Pdf";
    }
    else if (filetype == "doc" || filetype == "docx" || filetype == "wps")
    {
        iconpath = "Doc";
    }
    else if (filetype == "avi" || filetype == "mp4" || filetype == "mov" || filetype == "wmv")
    {
        iconpath = "Video";
    }
    else if (filetype == "mp3" || filetype == "pcm" || filetype == "wav" || filetype=="wma")
    {
        iconpath = "Music";
    }
    else
    {
        iconpath = "Other";
    }
    return iconpath;
}
XCOM_API std::string XGetSizeString(long long size)
{
    string filesize_str = "";
    if (size > 1024 * 1024 * 1024) //GB
    {
        double gb_size = (double)size / (double)(1024 * 1024 * 1024);
        long long tmp = gb_size * 100;

        stringstream ss;
        ss << tmp / 100;
        if (tmp % 100 > 0)
            ss << "." << tmp % 100;
        ss << "GB";
        filesize_str = ss.str();
    }
    else if (size > 1024 * 1024) //MB
    {
        double gb_size = (double)size / (double)(1024 * 1024);
        long long tmp = gb_size * 100;

        stringstream ss;
        ss << tmp / 100;
        if (tmp % 100 > 0)
            ss << "." << tmp % 100;
        ss << "MB";
        filesize_str = ss.str();
    }
    else if (size > 1024) //KB
    {
        float gb_size = (float)size / (float)(1024);
        long long tmp = gb_size * 100;
        stringstream ss;
        ss << tmp / 100;
        if (tmp % 100 > 0)
            ss << "." << tmp % 100;
        ss << "KB";
        filesize_str = ss.str();
    }
    else //B
    {
        float gb_size = size / (float)(1024);
        long long tmp = gb_size * 100;

        stringstream ss;
        ss << size;
        ss << "B";
        filesize_str = ss.str();
    }
    return filesize_str;
}
XCOM_API std::string XGetTime(int timestamp, std::string fmt )
{
    char time_buf[128] = { 0 };
    time_t tm = 0;
    if (timestamp > 0)
        tm = timestamp;
    else
        tm = time(0);
    
    strftime(time_buf, sizeof(time_buf), fmt.c_str(), gmtime(&tm));
    return time_buf;
}
XCOM_API  void XStringSplit(std::vector<string> &vec, std::string str, std::string find)
{
    int pos1 = 0;
    int pos2 = 0;
    vec.clear();
    while ((pos2 = str.find(find, pos1)) != (int)string::npos)
    {
        vec.push_back(str.substr(pos1, pos2 - pos1));
        pos1 = pos2 + find.length();
    }
    string strTemp = str.substr(pos1);
    if ((int)strTemp.size() > 0)
    {
        vec.push_back(str.substr(pos1));
    }
}

XCOM_API   void XStrReplace(string str, string str_find, string str_replace, string &sreturn)
{
    sreturn = "";
    int str_size = str.size();
    int rep_size = str_find.size();
    for (int i = 0; i < str_size - (rep_size - 1); i++)
    {
        bool is_find = true;
        for (int j = 0; j < rep_size; j++)
        {
            if (str[i + j] != str_find[j])
            {
                is_find = false;
                break;
            }
        }
        if (is_find)
        {
            sreturn += str_replace;
            i += (rep_size - 1);
        }
        else
        {
            sreturn += str[i];
        }
    }
}
XCOM_API  std::string XFormatDir(const std::string &dir)
{
    std::string re = "";
    bool is_sep = false; // 是否是/ 或者是 "\"
    for (int i = 0; i < dir.size(); i++)
    {
        if (dir[i] == '/' || dir[i] == '\\')
        {
            if (is_sep)
            {
                continue;
            }
            re += '/';
            is_sep = true;
            continue;
        }
        is_sep = false;
        re += dir[i];
    }
    return re;
}

XCOM_API string XTrim(const string& s)
{
    if (s.length() == 0)
        return s;
    int beg = s.find_first_not_of(" \a\b\f\n\r\t\v");
    int end = s.find_last_not_of(" \a\b\f\n\r\t\v");
    if (beg == (int)std::string::npos) // No non-spaces
        return "";
    return string(s, beg, end - beg + 1);
}

XCOM_API bool XFileExist(const std::string& s)
{
    if (_access(s.c_str(), 0) == -1)
    {
        return false;
    }
    return true;
}

XCOM_API void XNewDir(std::string path)
{
    string tmp = XFormatDir(path);

    vector<string>paths;
    XStringSplit(paths, tmp, "/");

    string tmpstr = "";
    for (auto s : paths)
    {
        tmpstr += s + "/";
        if (_access(tmpstr.c_str(), 0) == -1)
        {
            _mkdir(tmpstr.c_str());
        }
    }
}

XCOM_API void XDelFile(std::string path)
{
#ifdef _WIN32
    DeleteFileA(path.c_str());
#else
    remove(path.c_str());
#endif
}
//获取目录列表   //格式 文件名，文件大小（byte），是否是目录（0,1），文件修改时间（2020-01-22 19:30:13），
XCOM_API std::list< XToolFileInfo > GetDirList(std::string path)
{
    std::list< XToolFileInfo > file_list;

#ifdef _WIN32
    //存储文件信息
    _finddata_t file;
    string dirpath = path + "/*.*";
    //目录上下文
    intptr_t dir = _findfirst(dirpath.c_str(), &file);
    if (dir < 0)
        return file_list;
    char time_buf[128] = { 0 };
    do
    {
        XToolFileInfo file_info;
        if (file.attrib & _A_SUBDIR)
        {
            file_info.is_dir = true;
        }
        file_info.filename = file.name;
        file_info.filesize = file.size;
        file_info.time_write = file.time_write;
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
        time_t tm = file_info.time_write;
        strftime(time_buf, sizeof(time_buf), "%F %R", gmtime(&tm));
        file_info.time_str = time_buf;
        file_list.push_back(file_info);
    } while (_findnext(dir, &file) == 0);
    _findclose(dir);
#else
    const char *dir = path.c_str();
    DIR *dp = 0;
    struct dirent *entry = 0;
    struct stat statbuf;
    dp = opendir(dir);
    if (dp == NULL)
        return file_list;
    chdir(dir);
    char buf[1024] = { 0 };
    while ((entry = readdir(dp)) != NULL)
    {
        XToolFileInfo file_info;
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode))
        {
            file_info.is_dir = true;
        }
        file_info.filename = entry->d_name;
        file_info.filesize = statbuf.st_size;
        /*
               struct timespec st_atim;  // Time of last access 
               struct timespec st_mtim;  // Time of last modification 
               struct timespec st_ctim;  // Time of last status change 

           #define st_atime st_atim.tv_sec      // Backward compatibility 
           #define st_mtime st_mtim.tv_sec
           #define st_ctime st_ctim.tv_sec
        */
        file_info.time_write = statbuf.st_mtime;
        time_t tm = file_info.time_write;
        char time_buf[16] = {0};
        strftime(time_buf, sizeof(time_buf), "%F %R", gmtime(&tm));
        file_info.time_str = time_buf;
        file_list.push_back(file_info);

        //sprintf(buf, "%s,%ld;", entry->d_name, statbuf.st_size);
        //data += buf;
    }
    closedir(dp);
#endif

    return file_list;
}
XCOM_API std::string GetDirData(std::string path)
{
    string data = "";
#ifdef _WIN32
    //存储文件信息
    _finddata_t file;
    string dirpath = path + "/*.*";
    //目录上下文
    intptr_t dir = _findfirst(dirpath.c_str(), &file);
    if (dir < 0)
        return data;
    do
    {
        if (file.attrib & _A_SUBDIR) continue;
        char buf[1024] = { 0 };
        sprintf(buf, "%s,%u;", file.name, file.size);
        data += buf;
    } while (_findnext(dir, &file) == 0);
    _findclose(dir);
#else
    const char *dir = path.c_str();
    DIR *dp = 0;
    struct dirent *entry = 0;
    struct stat statbuf;
    dp = opendir(dir);
    if(dp == NULL)
	    return data;
    chdir(dir);
    char buf[1024] = {0};
    while((entry = readdir(dp))!=NULL)
    {
	    lstat(entry->d_name,&statbuf);
	    if(S_ISDIR(statbuf.st_mode))continue;
	    sprintf(buf,"%s,%ld;",entry->d_name,statbuf.st_size);
	    data += buf;
    }
    closedir(dp);
#endif
    //去掉结尾 ;
    if (!data.empty())
    {
        data = data.substr(0, data.size() - 1);
    }
    return data;
}

long long GetDirSize(const char * path)
{
    if (!path)return 0;
    long long dir_size = 0;
    string dir_new = path;
    string name = "";
#ifdef _WIN32
    //存储文件信息
    _finddata_t file;
    dir_new += "\\*.*";

    intptr_t dir = _findfirst(dir_new.c_str(), &file);
    if (dir < 0)
        return 0;
    do
    {
        // 是否是文件夹，并且名称不为"."或".." 
        if (file.attrib & _A_SUBDIR)
        {
            name = file.name;
            if (name == "." || name == "..")
                continue;
            // 将dirNew设置为搜索到的目录，并进行下一轮搜索 
            dir_new = path;
            dir_new += "/";
            dir_new += name;
            dir_size += GetDirSize(dir_new.c_str());

        }
        else
        {
            dir_size += file.size;
        }
    } while (_findnext(dir, &file) == 0);
    _findclose(dir);
#else
    DIR *dp = 0;
    struct dirent *entry = 0;
    struct stat statbuf;
    dp = opendir(dir_new.c_str());
    if (dp == NULL)
        return 0;
    chdir(dir_new.c_str());
    char buf[1024] = { 0 };
    while ((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode))
        {
            name = entry->d_name;
            if (name == "." || name == "..")
                continue;
            dir_new = path;
            dir_new += "/";
            dir_new += entry->d_name;
            dir_size += GetDirSize(dir_new.c_str());
        }
        else
        {
            dir_size += statbuf.st_size;
        }
    }
    closedir(dp);
#endif
    return dir_size;
}
bool GetDiskSize(const char *dir, unsigned long long *avail, unsigned long long *total, unsigned long long *free)
{
#ifdef _WIN32

    return GetDiskFreeSpaceExA(dir, (ULARGE_INTEGER *)avail, (ULARGE_INTEGER *)total, (ULARGE_INTEGER *)free);
#else
    struct statfs diskInfo;
    statfs(dir, &diskInfo);
    *total = diskInfo.f_blocks*diskInfo.f_bsize;
    *free = diskInfo.f_bfree*diskInfo.f_bsize;
    *avail = diskInfo.f_bavail*diskInfo.f_bsize;
    return true;
#endif
}
//通过域名 返回IP地址 只取第一个
std::string XGetHostByName(std::string host_name)
{
    #ifdef _WIN32
    static bool is_init = false;
    if (!is_init)
    {
        
        WORD sockVersion = MAKEWORD(2, 2);
        WSADATA wsaData;
        if (WSAStartup(sockVersion, &wsaData) != 0)
        {
            return "";
        }
        is_init = true;
    }
    #endif
    auto host = gethostbyname(host_name.c_str());
    auto addr = host->h_addr_list;
    if (!addr)
        return "";
    return inet_ntoa(*(struct in_addr*)*addr);
}


/*

void AES_ecb_encrypt(const unsigned char *in, unsigned char *out,
                     const AES_KEY *key, const int enc)
{

    assert(in && out && key);
    assert((AES_ENCRYPT == enc) || (AES_DECRYPT == enc));

    if (AES_ENCRYPT == enc)
        AES_encrypt(in, out, key);
    else
        AES_decrypt(in, out, key);
}
*/
class CXAES :public XAES
{
    ///////////////////////////////////////////////////////
    /// 设置加密秘钥 秘钥长度 128位（16字节） 192位 （24字节） 256位 (32字节)
    /// 长度不能超过32字节，返回失败
    /// 秘钥不足自动补充
    /// @key 秘钥
    /// @key_size 秘钥长度 字节
    /// @is_enc true  加密 false 解密
    /// @return 设置成功失败
    virtual bool SetKey(const char *key, int key_size, bool is_enc) override
    {
        if (key_size > 32 || key_size <= 0)
        {
            cerr << "AES key size error(>32 && <=0 )! key_size= " << key_size << endl;
            return false;
        }
        unsigned char aes_key[32] = { 0 };
        memcpy(aes_key, key, key_size);
        int bit_size = 0;
        if (key_size > 24)
        {
            bit_size = 32 * 8;
        }
        else if (key_size > 16)
        {
            bit_size = 24 * 8;
        }
        else
        {
            bit_size = 16 * 8;
        }

        /*
            if (bits != 128 && bits != 192 && bits != 256)
             return -2;
        */
        if (is_enc)
        {
            is_set_encode = true;
            if (AES_set_encrypt_key(aes_key, bit_size, &aes_) < 0)
            {
                return false;
            }
            return true;
        }
        is_set_decode = true;
        if (AES_set_decrypt_key(aes_key, bit_size, &aes_) < 0)
        {
            return false;
        }
        return true;
    }

    virtual void Drop() override
    {
        delete this;
    }

    ///////////////////////////////////////////////////////
    /// 解密
    /// @in 输入数据
    /// @in_size 输入数据大小
    /// @out 输出 数据空间要保证16字节的倍数
    /// @return  输出大小，失败返回<=0
    virtual long long Encrypt(const unsigned char *in, long long in_size, unsigned char *out) override
    {
        if (!in || in_size <= 0 || !out)
        {
            cerr << "Encrypt input data error" << endl;
            return 0;
        }

        if (!is_set_encode)
        {
            cerr << "Encrypt password not set" << endl;
            return 0;
        }
        long long enc_byte = 0;
        //AES_encrypt 默认 AES_ecb_encrypt 可分块并行加密  cbc加密强度更大，每段时间与上一段相关
        /*
        每次加密一个数据块16个字节，不全的需要补充
            s0 = GETU32(in     ) ^ rk[0];
            s1 = GETU32(in +  4) ^ rk[1];
            s2 = GETU32(in +  8) ^ rk[2];
            s3 = GETU32(in + 12) ^ rk[3];
        */
        unsigned char *p_in = 0;
        unsigned char *p_out = 0;
        unsigned char data[16] = { 0 };
        for (int i = 0; i < in_size; i += 16)
        {
            p_in = (unsigned char *)in + i;
            p_out = out + i;
            //处理不足16字节的补全
            if (in_size - i < 16)
            {
                memcpy(data, p_in, in_size - i);
                p_in = data;
            }
            enc_byte += 16;
            AES_encrypt(p_in, p_out, &aes_);
        }
        return enc_byte;
    }

    ///////////////////////////////////////////////////////
    /// 解密
    /// @in 输入数据
    /// @in_size 输入数据大小
    /// @out 输出 数据空间要保证16字节的倍数
    /// @return  输出大小，失败返回<=0
    virtual long long Decrypt(const unsigned char *in, long long in_size, unsigned char *out) override
    {

        if (!in || in_size <= 0 || !out || in_size % 16 != 0)
        {
            cerr << "Decrypt input data error" << endl;
            return 0;
        }

        if (!is_set_decode)
        {
            cerr << "Decrypt password not set" << endl;
            return 0;
        }

        long long enc_byte = 0;
        //AES_encrypt 默认 AES_ecb_encrypt 可分块并行加密  cbc加密强度更大，每段时间与上一段相关
        /*
        每次加密一个数据块16个字节，不全的需要补充
            s0 = GETU32(in     ) ^ rk[0];
            s1 = GETU32(in +  4) ^ rk[1];
            s2 = GETU32(in +  8) ^ rk[2];
            s3 = GETU32(in + 12) ^ rk[3];
        */
        unsigned char *p_in = 0;
        unsigned char *p_out = 0;
        unsigned char data[16] = { 0 };
        for (int i = 0; i < in_size; i += 16)
        {
            p_in = (unsigned char *)in + i;
            p_out = out + i;
            enc_byte += 16;
            AES_decrypt(p_in, p_out, &aes_);
        }
        return enc_byte;
    }
private:
    AES_KEY aes_;
    bool is_set_decode = false;
    bool is_set_encode = false;

};
XAES* XAES::Create()
{
    return new CXAES();
}