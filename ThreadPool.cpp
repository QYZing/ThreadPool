#include "ThreadPool.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <unistd.h>

template<class T>
ThreadPool<T>::ThreadPool(int min, int max)
{
    do
    {
        TaskQ = new TaskQueue<T>;
        if (TaskQ == nullptr)
        {
            std::cout << "new TaskQ error\n";
            break;
        }
        ThreadPoolArray = new pthread_t[max];
        if (ThreadPoolArray == nullptr)
        {
            std::cout << "ThreadPoolArray create error\n";
            break;
        }
        memset(ThreadPoolArray, 0, sizeof(pthread_t) * max);

        ThreadMin = min;
        ThreadMax = max;
        ThreadBusy = 0;
        ThreadExitNum = 0;
        ThreadLive = min;

        if (pthread_mutex_init(&Mutex, NULL) != 0 ||
            pthread_cond_init(&TaskNotEmpty, NULL) != 0)
        {
            std::cout << "mutex or condition error\n";
            break;
        }
        Shutdown = false;
        // create manage thread
        pthread_create(&ThreadManage, NULL, Manager, this);
        // create worker thread
        for (int i = 0; i < min; i++)
        {
            pthread_create(&ThreadPoolArray[i], NULL, Worker, this);
        }
        return;
    } while (0);

    if (TaskQ != nullptr)
        delete TaskQ;
    if (ThreadPoolArray != nullptr)
        delete[] ThreadPoolArray;
}

template<class T>
void *ThreadPool<T>::Manager(void *arg)
{
    ThreadPool<T> *pool = static_cast<ThreadPool<T> *>(arg);
    while (!pool->Shutdown)
    {
        sleep(1);

        pthread_mutex_lock(&pool->Mutex);
        int QueueSize = pool->TaskQ->size();
        int liveThread = pool->ThreadLive;
      //  int BusyThread = pool->ThreadBusy;
        pthread_mutex_unlock(&pool->Mutex);

        // add thread    if tasksize > livesize && livesize <maxthread
        printf("\nalive thread = %d\n",liveThread);
        if (QueueSize > liveThread && liveThread < pool->ThreadMax)
        {
            pthread_mutex_lock(&pool->Mutex);
            int count = 0;
            for (int i = 0; i < pool->ThreadMax && count < ADDNUMTHREAD && pool->ThreadLive < pool->ThreadMax; i++)
            {
                if (pool->ThreadPoolArray[i] == 0)
                {
                    pthread_create(&pool->ThreadPoolArray[i], NULL, Worker, pool);
                    pool->ThreadLive++;
                    count++;
                }
            }
            pthread_mutex_unlock(&pool->Mutex);
        }

        // delete thread  if tasksize < livesize * 2 && livesize > min
        if (QueueSize < liveThread * 2 && liveThread > pool->ThreadMin)
        {
            pthread_mutex_lock(&pool->Mutex);
            pool->ThreadExitNum = DELNUMTHREAD;
            pthread_mutex_unlock(&pool->Mutex);
            for (int i = 0; i < DELNUMTHREAD; i++)
            {
                pthread_cond_signal(&pool->TaskNotEmpty);
            }
        }
    }
    return nullptr;
}

template<class T>
void *ThreadPool<T>::Worker(void *arg)
{
    ThreadPool<T> *pool = static_cast<ThreadPool<T> *>(arg);
    while (true)
    {
        pthread_mutex_lock(&pool->Mutex);

        while (pool->TaskQ->size() == 0 && !pool->Shutdown) //  No tasks are blocked
        {
            pthread_cond_wait(&pool->TaskNotEmpty, &pool->Mutex);
            if (pool->ThreadExitNum > 0)
            {
                pool->ThreadExitNum--;
                if (pool->ThreadLive > pool->ThreadMin)
                {
                    pool->ThreadLive--;
                    pthread_mutex_unlock(&pool->Mutex);
                    pool->ThreadExit();
                }
            }
        }

        if (pool->Shutdown) // threadpool shutdown
        {
            pthread_mutex_unlock(&pool->Mutex);
            pool->ThreadExit();
        }

        Task<T> task(pool->TaskQ->TakeTask());

        pool->ThreadBusy++;

        pthread_mutex_unlock(&pool->Mutex);

        printf("thread %ld working\n", pthread_self());

        task.function(task.arg);
        delete task.arg;
        task.arg = nullptr;

        printf("thread %ld end working\n", pthread_self());

        pthread_mutex_lock(&pool->Mutex);
        pool->ThreadBusy--;
        pthread_mutex_unlock(&pool->Mutex);
    }
}

template<class T>
void ThreadPool<T>::ThreadExit()
{
    pthread_t tid = pthread_self();

    for (int i = 0; i < ThreadMax; i++)
    {
        if (ThreadPoolArray[i] == tid)
        {
            ThreadPoolArray[i] = 0;
            printf("\nthreadexit called \n");
            break;
        }
    }
    pthread_exit(NULL);
}

template<class T>
void ThreadPool<T>::AddTask(Task<T> task)
{
    if (Shutdown)
    {
        printf("shutdown \n");
        return;
    }
    // add task
    TaskQ->AddTask(task);
    // send signal task not empty
    pthread_cond_signal(&TaskNotEmpty);
}

// Get the thread number that is working
template<class T>
int ThreadPool<T>::GetThreadNum()
{
    pthread_mutex_lock(&Mutex);
    int busynum = this->ThreadBusy;
    pthread_mutex_unlock(&Mutex);
    return busynum;
}

// Get the live thread
template<class T>
int ThreadPool<T>::GetThreadAlive()
{
    pthread_mutex_lock(&Mutex);
    int alivenum = this->ThreadLive;
    pthread_mutex_unlock(&Mutex);
    return alivenum;
}
template<class T>
ThreadPool<T>::~ThreadPool()
{

    Shutdown = true;

    pthread_join(ThreadManage, NULL);

    if (TaskQ)
    {
        delete TaskQ;
    }
    printf("threadlive = %d\n",this->ThreadLive);
    for (int i = 0; i < this->ThreadLive; i++)
    {
        pthread_cond_signal(&TaskNotEmpty);
    }
    if (ThreadPoolArray)
    {
        delete[] ThreadPoolArray;
    }
    pthread_mutex_destroy(&Mutex);
    pthread_cond_destroy(&TaskNotEmpty);
}