#include "xthread_pool.h"
#include "xthread.h"
#include <thread>
#include <iostream>
#include "xtask.h"
#include <thread>
#include <sstream>
#include "xtools.h"
#include "xlog_client.h"
#ifdef _WIN32
//和protobuf头文件会有冲突 ，protobuf的头文件要在windows.h之前
#include <windows.h>
#else
#include <signal.h>
#endif
using namespace std;

///用于线程的循环退出判断
static bool is_exit_all = false;

///所有的线程对象
static vector<XThread *>all_threads;
static mutex all_threads_mutex;
//////////////////////////////////////////////////////
///退出所有的线程
void XThreadPool::ExitAllThread()
{
    is_exit_all = true;
    all_threads_mutex.lock();
    for (auto t : all_threads)
    {
        t->Exit();
    }
    all_threads_mutex.unlock();
    this_thread::sleep_for(200ms);
}

void XThreadPool::Wait()
{
    while (!is_exit_all)
    {
        this_thread::sleep_for(100ms);
    }
}
class CXThreadPool :public XThreadPool
{
public:
    //分发线程
    void Dispatch(XTask *task)
    {
        //轮询
        if (!task)return;
        int tid = (last_thread_ + 1) % thread_count_;
        last_thread_ = tid;
        XThread *t = threads_[tid];

        t->AddTask(task);

        //激活线程
        t->Activate();

    }
    //初始化所有线程并启动线程
    void Init(int thread_count)
    {
        this->thread_count_ = thread_count;
        this->last_thread_ = -1;
        for (int i = 0; i < thread_count; i++)
        {
            XThread *t = XThread::Create();// new XThread();
            t->id = i + 1;
            stringstream ss;
            ss << "Create thread " << i;
            LOGDEBUG(ss.str().c_str());
            //启动线程
            t->Start();
            threads_.push_back(t);
            all_threads_mutex.lock();
            all_threads.push_back(t);
            all_threads_mutex.unlock();
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    }
private:

    ///线程数量
    int thread_count_ = 0;

    ///上一次分发得到线程，用于轮询
    int last_thread_ = -1;

    ///程池线程队列
    std::vector<XThread *>threads_;
};

XThreadPool *XThreadPoolFactory::Create()
{
    //socket库初始化
    static mutex mux;
    static bool is_init = false;
    mux.lock();
    if (!is_init)
    {
#ifdef _WIN32 
        //初始化socket库
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
#else
        //使用断开连接socket，会发出此信号，造成程序退出
        if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
            return NULL;
#endif
        is_init = true;
    }
    mux.unlock();
    return new CXThreadPool();
}
