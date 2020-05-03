#ifndef XROUTERSERVER_H
#define XROUTERSERVER_H
#include "xservice.h"
class XSSLCtx;

class XRouterServer :public XService
{
public:
    XServiceHandle* CreateServiceHandle();
private:
    //是否采用了ssl
    //bool is_ssl_ = false;
    XSSLCtx *ssl_ctx_ = 0; //没有添加清理的代码
};

#endif
