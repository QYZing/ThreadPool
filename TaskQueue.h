#ifndef _TASKQUEUE_H_
#define _TASKQUEUE_H_
#include<queue>
#include<pthread.h>

using callback = void (*)(void *arg);

template<class T>
struct  Task
{
    Task()
    {
        function = nullptr;
        arg = nullptr;
    }
    Task(callback function , T *arg)
    {
        this->function = function;
        this->arg  = arg;
    }
    callback function;
    T *arg;
};

template<class T>
class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();

    // add task
    void AddTask(Task<T> task);
    void AddTask(callback c , void *arg);

    // take task

    Task<T> TakeTask();

    inline size_t size()
    {
        return m_TaskQ.size();
    }
private:
    std::queue<Task<T>> m_TaskQ;
    pthread_mutex_t m_Mutex;
};

#endif