#include "xssl.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>
using namespace std;

//释放SSL
void XSSL::Close()
{
    if (ssl_)
    {
        SSL_shutdown(ssl_);
        SSL_free(ssl_);
        ssl_ = 0;
    }
}

int  XSSL::Write(const void *data, int data_size)
{
    if (!ssl_)return 0;
    return SSL_write(ssl_, data, data_size);
}

int  XSSL::Read(void *buf, int buf_size)
{
    if (!ssl_)return 0;
    return SSL_read(ssl_,buf, buf_size);
}
//打印对方证书信息
void XSSL::PrintCert()
{
    if (!ssl_) return;
    //获取到证书
    auto cert = SSL_get_peer_certificate(ssl_);
    if (cert == NULL)
    {
        cout << "no certificate" << endl;
        return;
    }
    char buf[1024] = { 0 };
    auto sname = X509_get_subject_name(cert);
    auto str = X509_NAME_oneline(sname, buf, sizeof(buf));
    if (str)
    {
        cout << "subject:"<<str << endl;
    }
    //发行
    auto issuer = X509_get_issuer_name(cert);
    str = X509_NAME_oneline(issuer, buf, sizeof(buf));
    if (str)
    {
        cout << "issuer:" << str << endl;
    }
    X509_free(cert);
}

//打印通信使用的算法
void XSSL::PrintCipher()
{
    if (!ssl_)return ;
    cout << SSL_get_cipher(ssl_) << endl;
}

//客户端处理ssl握手
bool XSSL::Connect()
{
    //socket 的connect已经连接完成
    if (!ssl_)
        return false;
    int re = SSL_connect(ssl_);
    if (re <= 0)
    {
        cout << "XSSL::Connect() failed!" << endl;
        ERR_print_errors_fp(stderr);
        return false;
    }
    cout << "SSL_connect success!" << endl;
    PrintCipher();
    PrintCert();
    return true;
}
//服务端接收ssl连接
bool XSSL::Accept()
{
    if (!ssl_)
        return false;
    //建立ssl连接验证，密钥协商
    int re = SSL_accept(ssl_);
    if (re <= 0)
    {
        cout << "XSSL::Accept() failed!" << endl;
        ERR_print_errors_fp(stderr);
        return false;
    }
    cout << "SSL_accept success!" << endl;
    PrintCipher();
    return true;
}

XSSL::XSSL()
{
}


XSSL::~XSSL()
{
}
