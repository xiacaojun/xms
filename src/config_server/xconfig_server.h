#pragma once
#include "xservice.h"
class XConfigServer :public XService
{
public:
    XConfigServer();
    ~XConfigServer();
    
    ///根据参数 初始化服务，需要先调用
    void main(int argc, char *argv[]);

    //等待线程退出
    void Wait();

    XServiceHandle* CreateServiceHandle();

};

