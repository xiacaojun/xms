#include "xthread.h"
#include "xlog_client.h"
#include <thread>
#include <iostream>
#include <event2/event.h>
#include "xtask.h"
#include "xtools.h"
#ifdef _WIN32
#else
#include <unistd.h>
#endif

using namespace std;

//激活线程任务的回调函数
static void NotifyCB(int fd, short which, void *arg)
{
	XThread *t = (XThread *)arg;
	t->Notify(fd, which);
}

class CXThread :public XThread
{
public:
    void Notify(int fd, short which)
    {
        //水平触发 只要没有接受完成，会再次进来
        char buf[2] = { 0 };
#ifdef _WIN32
        int re = recv(fd, buf, 1, 0);
#else
        //linux中是管道，不能用recv
        int re = read(fd, buf, 1);
#endif
        if (re <= 0)
            return;

        stringstream ss;
        ss << id << " thread " << buf;
        LOGDEBUG(ss.str().c_str());
        XTask *task = NULL;
        //获取任务，并初始化任务
        tasks_mutex_.lock();
        if (tasks_.empty())
        {
            tasks_mutex_.unlock();
            return;
        }
        task = tasks_.front(); //先进先出
        tasks_.pop_front();
        tasks_mutex_.unlock();
        task->Init();
    }

    void AddTask(XTask *t)
    {
        if (!t)return;
        t->set_base(this->base_);
        tasks_mutex_.lock();
        tasks_.push_back(t);
        tasks_mutex_.unlock();
    }
    //线程激活
    void Activate()
    {
#ifdef _WIN32
        int re = send(this->notify_send_fd_, "c", 1, 0);
#else
        int re = write(this->notify_send_fd_, "c", 1);
#endif
        if (re <= 0)
        {
            //stringstream ss;
            //ss << "XThread::Activate() failed!" ;
            LOGERROR("XThread::Activate() failed!");
        }
    }
    //启动线程
    void Start()
    {
        Setup();
        //启动线程
        thread th(&CXThread::Main, this);

        //断开与主线程联系
        th.detach();
    }
    //安装线程，初始化event_base和管道监听事件用于激活
    bool Setup()
    {
        //windows用配对socket linux用管道
#ifdef _WIN32
    //创建一个socketpair 可以互相通信 fds[0] 读 fds[1]写 
        evutil_socket_t fds[2];
        if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, fds) < 0)
        {
            //cout << "evutil_socketpair failed!" << endl;
      //      stringstream ss;
      //      ss << "XThread::Activate() failed!";
            LOGERROR("evutil_socketpair failed!");

            return false;
        }
        //设置成非阻塞
        evutil_make_socket_nonblocking(fds[0]);
        evutil_make_socket_nonblocking(fds[1]);
#else
    //创建的管道 不能用send recv读取 read write
        int fds[2];
        if (pipe(fds))
        {
            //cerr << "pipe failed!" << endl;
            LOGERROR("pipe failed!");
            return false;
        }
#endif

        //读取绑定到event事件中，写入要保存
        notify_send_fd_ = fds[1];

        //创建libevent上下文（无锁）
        event_config *ev_conf = event_config_new();
        //需要考虑打开锁，多线程调用时
        event_config_set_flag(ev_conf, EVENT_BASE_FLAG_NOLOCK);
        this->base_ = event_base_new_with_config(ev_conf);
        event_config_free(ev_conf);
        if (!base_)
        {
            //cerr << "event_base_new_with_config failed in thread!" << endl;
            LOGERROR("event_base_new_with_config failed in thread!");
            return false;
        }

        //添加管道监听事件，用于激活线程执行任务
        event *ev = event_new(base_, fds[0], EV_READ | EV_PERSIST, NotifyCB, this);
        event_add(ev, 0);

        return true;
    }
private:
    //线程入口函数
    void Main()
    {
        stringstream ss;
        ss << id << " XThread::Main() begin" << endl;
        LOGDEBUG(ss.str().c_str());
        if (!base_)
        {
            cerr << "XThread::Main faield! base_ is null " << endl;
            cerr << "In windows set WSAStartup(MAKEWORD(2, 2), &wsa)" << endl;
            return;
        }

        //设置为不阻塞分发消息
        while (!is_exit_)
        {
            //一次处理多条消息
            event_base_loop(base_, EVLOOP_NONBLOCK);
            this_thread::sleep_for(1ms);
        }


        //event_base_dispatch(base_);

        event_base_free(base_);

        //cout << id << " XThread::Main() end" << endl;
        ss.str("");
        ss << id << " XThread::Main() end" << endl;
        LOGDEBUG(ss.str().c_str());
    }


    bool is_exit_ = false;
    int notify_send_fd_ = 0;
    struct event_base *base_ = 0;

    //任务列表
    std::list<XTask*> tasks_;
    //线程安全 互斥
    std::mutex tasks_mutex_;
};

XThread * XThread::Create()
{
    return new CXThread();
}

XThread::XThread()
{
}


XThread::~XThread()
{
}
