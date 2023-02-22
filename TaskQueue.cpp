#include "TaskQueue.h"

template<class T>
TaskQueue<T>::TaskQueue()
{
    pthread_mutex_init(&m_Mutex, NULL);
}
template<class T>
TaskQueue<T>::~TaskQueue()
{
    pthread_mutex_destroy(&m_Mutex);
}

// add task
template<class T>
void TaskQueue<T>::AddTask(Task<T> task)
{
    pthread_mutex_lock(&m_Mutex);
    m_TaskQ.push(task);
    pthread_mutex_unlock(&m_Mutex);
}
template<class T>
void TaskQueue<T>::AddTask(callback c, void *arg)
{
    pthread_mutex_lock(&m_Mutex);
    m_TaskQ.push(Task<T>(c, arg));
    pthread_mutex_unlock(&m_Mutex);
}

// take task
template<class T>
Task<T> TaskQueue<T>::TakeTask()
{
    Task<T> t;
    pthread_mutex_lock(&m_Mutex);
    if (!m_TaskQ.empty())
    {
        t = m_TaskQ.front();
        m_TaskQ.pop();
    }
    pthread_mutex_unlock(&m_Mutex);
    return t;
}
