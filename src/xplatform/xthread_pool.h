#ifndef XTHREAD_POOL_H
#define XTHREAD_POOL_H

#ifdef _WIN32
#ifdef XCOM_EXPORTS
#define XCOM_API __declspec(dllexport)
#else
#define XCOM_API __declspec(dllimport)
#endif
#else
#define XCOM_API
#endif
#include <vector>
class XThread;
class XTask;
class XCOM_API XThreadPool
{
public:
    ///////////////////////////////////////////////////////////////////////////
    /// @brief 获取XThreadPool的静态对象 （静态函数）
    /// @return XThreadPool 静态对象的指针
    ///////////////////////////////////////////////////////////////////////////
	//static XThreadPool* Get()
	//{
	//	static XThreadPool p;
	//	return &p;
	//}
    ///////////////////////////////////////////////////////////////////////////
    /// @brief 初始化所有线程并启动线程，创建号event_base ,并在线程中开始接收消息
	virtual void Init(int thread_count) = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// @brief 分发任务到线程中执行，会调用task的Init进行任务初始化
    ///        任务会轮询分发到线程池中的各个线程
    /// @param task 任务接口对象，XTask需要用户自己继承并重载Init函数
	virtual void Dispatch(XTask *task) = 0;
    
    //////////////////////////////////////////////////////
    ///退出所有的线程
    static void ExitAllThread();

    ///阻塞 等待ExitAllThread
    static void Wait();

};

class XCOM_API XThreadPoolFactory
{
public:
    //创建线程池对象
    static XThreadPool *Create();
};

#endif