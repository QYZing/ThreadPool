#include<unistd.h>
#include<stdio.h>
#include"ThreadPool.h"
#include"ThreadPool.cpp"
void  TaskFun(void *arg)
{
    printf("new thread tid = %ld num = %d", pthread_self(), *(int *)arg);
    usleep(1000);
}
int main()
{
    ThreadPool<int> pool(3,10);

    for (int i = 0; i < 10000; i++)
    {
        int *num = new int(i+100);
        pool.AddTask(Task<int>(TaskFun,num));
    }
    sleep(2);
    return 0;
}