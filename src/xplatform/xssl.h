#ifndef XSSL_H
#define XSSL_H

#ifdef _WIN32
#ifdef XCOM_EXPORTS
#define XCOM_API __declspec(dllexport)
#else
#define XCOM_API __declspec(dllimport)
#endif
#else
#define XCOM_API
#endif
/////////////////////////////////////////////////////////////////////////// 
/// @brief 具体的ssl连接对象
/// @details 处理每一个ssl连接
///////////////////////////////////////////////////////////////////////////
class XCOM_API XSSL
{
public:
    XSSL();
    ~XSSL();

    //空对象
    bool IsEmpty() { return ssl_ == 0; }

    //服务端接收ssl连接
    bool Accept();

    //客户端处理ssl握手
    bool Connect();

    //打印通信使用的算法
    void PrintCipher();

    //打印对方证书信息
    void PrintCert();

    //加密发送数据
    int Write(const void *data, int data_size);

    //读取密文并解密
    int Read(void *buf, int buf_size);

    //设定ssl的上下文
    void set_ssl(struct ssl_st *ssl) { this->ssl_ = ssl; }

    //释放SSL
    void Close();

    //返回ssl上下文
    struct ssl_st *ssl() { return ssl_; }

private:
    ///ssl 上下文
    struct ssl_st *ssl_ = 0;
};

#endif