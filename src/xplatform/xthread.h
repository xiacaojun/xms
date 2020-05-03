#ifndef XTHREAD_H
#define XTHREAD_H

#include <list>
#include <mutex>
#ifdef _WIN32
#ifdef XCOM_EXPORTS
#define XCOM_API __declspec(dllexport)
#else
#define XCOM_API __declspec(dllimport)
#endif
#else
#define XCOM_API
#endif
class XTask;
class XCOM_API XThread
{
public:
    static XThread* Create();
    
	//启动线程
    virtual  void Start() = 0;

	//安装线程，初始化event_base和管道监听事件用于激活
    virtual  bool Setup() = 0;

	//收到主线程发出的激活消息（线程池的分发）
    virtual  void Notify(int fd, short which) = 0;

	//线程激活
    virtual  void Activate() = 0;

	//添加处理的任务，一个线程同时可以处理多个任务，共用一个event_base
	virtual void AddTask(XTask *t) = 0;
	
	~XThread();

	//线程编号
	int id = 0;

    //退出线程
    void Exit()
    {
        is_exit_ = true;
    }
protected:
    XThread();
    bool is_exit_ = false;

};

#endif
