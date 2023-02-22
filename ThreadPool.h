#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_
#include "TaskQueue.h"
#include "TaskQueue.cpp"
#include <cstring>

template<class T>
class ThreadPool
{
public:
    ThreadPool(int min, int max);
    // destory threadpool
    ~ThreadPool();

    // add task in threadpool
    void AddTask(Task<T> t);

    // Get the thread number that is working
    int GetThreadNum();

    // Get the live thread
    int GetThreadAlive();
private:
    void ThreadExit();

    static const int ADDNUMTHREAD = 4;
    static const int DELNUMTHREAD = 2;
    static void *Manager(void *arg);
    static void *Worker(void *arg);

private:
    TaskQueue<T> *TaskQ;

    pthread_t ThreadManage;     // Manager thread ID；
    pthread_t *ThreadPoolArray; // threadpool ID

    int ThreadMin;     // Min thread
    int ThreadMax;     // Max thread
    int ThreadBusy;    // Busy thread
    int ThreadLive;    // Live thread
    int ThreadExitNum; // Number of thread to cut

    pthread_mutex_t Mutex;       // Task queue block
    pthread_cond_t TaskNotEmpty; // Whether the task queue is empty

    bool Shutdown; // Whether the thread pool is working
};

#endif