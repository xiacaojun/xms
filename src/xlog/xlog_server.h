#pragma once

#include "xservice.h"
class XLogServer:public XService
{
public:
    ///根据参数 初始化服务，需要先调用
    void main(int argc, char *argv[]);


    XServiceHandle* CreateServiceHandle();
    XLogServer();
    ~XLogServer();
};

