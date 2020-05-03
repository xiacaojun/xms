#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
using namespace std;
int main(int argc,char *argv[])
{
    //3 程序退出后过多长时间重启
    int timeval = 3;
    if(argc<3)
    {
        cout<<"Useage: ./watchdog 3 ./register_server 20011"<<endl;
        return -1;
    }
    //启动子进程，主进程退出，交给1进程（守护进程后台 ）
    int pid = 0;
    pid = fork();
    if(pid > 0) exit(0);//父进程退出 交给1进程
    setsid();   //产生新的会话 与父进程脱离
    umask(0);   //掩码
    
    //进入守护进程

    //重启间隔
    timeval = atoi(argv[1]);

    //参数传递
    string cmd = "";
    for(int i = 2; i<argc; i++)
    {
        cmd +=" ";
        cmd +=argv[i];
    }
    cout<<cmd<<endl;
    for(;;)
    {
        //启动进程，并等待进程退出
        int ret = system(cmd.c_str());
        sleep(timeval);
    }

    return 0;
}