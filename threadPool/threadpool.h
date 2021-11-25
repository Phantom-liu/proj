#ifndef _THREADPOOL_H
#define _THREADPOOL_H

typedef struct ThreadPool ThreadPool;
//创建线程池并初始化
ThreadPool* threadPoolCreate(int min, int max, int queueSize);

//销毁线程池
int destroyPool(ThreadPool *pool);

//给线程池添加任务
void addWork(ThreadPool *pool, void(*func)(void*), void *args);

//获取线程池中工作线程个数
int getBusyNum(ThreadPool *pool);

//获取线程池中活着的线程个数
int getLiveNum(ThreadPool *pool);


///////////////////////////
void* worker(void *args);
void* manager(void *args);
void* threadExit(ThreadPool *pool);

#endif