#include<stdio.h>
#include "threadpool.h"
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#define ADD_THREAD_NUM 2
//任务结构体
typedef struct Task {
	void (*function)(void *args);
	void *args;
}Task;

//线程池结构体
struct ThreadPool {
	//任务队列	
	Task *taskQ;
	int queueCapacity;
	int queueSize;
	int queueFront;
	int queueRear;

	pthread_t managerID;
	pthread_t *threadIDs;
	int minNum;					//最小线程数
	int maxNum;					//最大线程数
	int busyNum;				//忙线程个树
	int liveNum;				//存活线程个数
	int exitNum;				//要销毁线程个数
	pthread_mutex_t mutexPool;	//整个线程池的锁
	pthread_mutex_t mutexBusy;	//BusyNum的锁
	pthread_cond_t notFull;		//任务队列是否未满
	pthread_cond_t notEmpty;	//任务队列是否为空
	int shutdown;				//销毁为1 否则为0

};

ThreadPool* threadPoolCreate(int min, int max, int queueSize) {
	ThreadPool *pool = (ThreadPool*)malloc(sizeof(ThreadPool));
	do {
		if (pool == NULL) {
			perror("malloc threadpool");
			break;
		}
		pool->threadIDs = (pthread_t*)malloc(sizeof(pthread_t)*max);
		if (pool->threadIDs == NULL) {
			perror("malloc threadIDs");
			break;
		}
		memset(pool->threadIDs, 0, sizeof(pthread_t));
		pool->minNum = min;
		pool->maxNum = max;
		pool->busyNum = 0;
		pool->liveNum = min;
		pool->exitNum = 0;
		if (pthread_mutex_init(&pool->mutexPool, NULL) != 0 ||
			pthread_mutex_init(&pool->mutexBusy, NULL) != 0 ||
			pthread_cond_init(&pool->notFull, NULL) != 0 ||
			pthread_cond_init(&pool->notEmpty, NULL) != 0) {
			perror("lock init");
			break;
		}
		pool->shutdown = 0;
		pool->taskQ = (Task*)malloc(sizeof(Task)*queueSize);
		pool->queueCapacity = queueSize;
		pool->queueSize = 0;
		pool->queueFront = 0;
		pool->queueRear = 0;

		pthread_create(&pool->managerID, NULL, manager, pool);
		for (int i = 0; i < min; i++) {
			pthread_create(&pool->threadIDs[i], NULL, worker, pool);
		}
		return pool;
	} while (0);
	if (pool->threadIDs)free(pool->threadIDs);
	if (pool->taskQ)free(pool->taskQ);
	if (pool)free(pool);
	return NULL; 
}


void* worker(void *args) {
	ThreadPool *pool = (ThreadPool*)args;

	while (1) {
		pthread_mutex_lock(&pool->mutexPool);
		//当前任务队列是否为空
		while (pool->queueSize == 0 && !pool->shutdown) {
			//阻塞工作线程
			pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);

			//判断是否要销毁
			if (pool->exitNum > 0) {
				pool->exitNum--;
				if (pool->liveNum > pool->minNum) {
					pool->liveNum--;
					pthread_mutex_unlock(&pool->mutexPool);
					threadExit(pool);
				}
			}
		}
		//判断线程池是否关闭
		if (pool->shutdown) {
			pthread_mutex_unlock(&pool->mutexPool);
			pthread_exit(NULL);
		}
		Task task;
		task.function = pool->taskQ[pool->queueFront].function;
		task.args = pool->taskQ[pool->queueFront].args;
		pool->queueFront = (pool->queueFront + 1) % pool->queueCapacity;
		pool->queueSize--;
		pthread_cond_signal(&pool->notFull);
		//解锁
		pthread_mutex_unlock(&pool->mutexPool);
		//busyNum锁
		pthread_mutex_lock(&pool->mutexBusy);
		pool->busyNum++;
		pthread_mutex_unlock(&pool->mutexBusy);

		task.function(task.args);
		free(task.args);
		task.args = NULL;
		pthread_mutex_lock(&pool->mutexBusy);
		pool->busyNum--;
		pthread_mutex_unlock(&pool->mutexBusy);
	}
	return NULL;
}

void* manager(void *args) {
	ThreadPool* pool = (ThreadPool*)args;
	while (!pool->shutdown) {
		sleep(3);
		
		//取出线程池中任务的数量和当前线程的数量
		pthread_mutex_lock(&pool->mutexPool);
		int queueSize = pool->queueSize;
		int liveNum = pool->liveNum;
		pthread_mutex_unlock(&pool->mutexPool);

		//取出忙线程的数量
		pthread_mutex_lock(&pool->mutexBusy);
		int busyNum = pool->busyNum;
		pthread_mutex_unlock(&pool->mutexBusy);

		//添加线程
		//任务的个数大于存活的线程数，并且存活的线程数小于最大线程数
		if (queueSize > liveNum&&liveNum < pool->maxNum) {
			pthread_mutex_lock(&pool->mutexPool);
			int cnt = 0;
			for (int i = 0; i < pool->maxNum&&cnt < ADD_THREAD_NUM&&pool->liveNum < pool->maxNum; i++) {
				if (pool->threadIDs[i] == 0) {
					pthread_create(&pool->threadIDs[i], NULL, worker, pool);
					cnt++;

					pool->liveNum++;
				}
			}
			pthread_mutex_unlock(&pool->mutexPool);
		}

		//销毁线程
		//忙线程个数×2小于存活的线程数，并且存活的线程大于最小线程数
		if (busyNum * 2 < liveNum&&liveNum > pool->minNum) {
			pthread_mutex_lock(&pool->mutexPool);
			pool->exitNum = ADD_THREAD_NUM;
			pthread_mutex_unlock(&pool->mutexPool);
			for (int i = 0; i < ADD_THREAD_NUM; i++) {
				pthread_cond_signal(&pool->notEmpty);
			}
		}
	}
}

void* threadExit(ThreadPool *pool) {
	pthread_t tid = pthread_self();
	for (int i = 0; i < pool->maxNum; i++) {
		if (pool->threadIDs[i] == tid) {
			pool->threadIDs[i] = 0;
			printf("threadExit() called,%ld exiting...", tid);
			break;
		}
	}
	return NULL;
}

void addWork(ThreadPool *pool, void(*func)(void*), void *args) {
	pthread_mutex_lock(&pool->mutexPool);
	while (pool->queueSize == pool->queueCapacity && !pool->shutdown) {
		//阻塞生产者线程
		pthread_cond_wait(&pool->notFull, &pool->mutexPool);
	}
	if (pool->shutdown) {
		pthread_mutex_unlock(&pool->mutexPool);
		return;
	}
	pool->taskQ[pool->queueRear].function = func;
	pool->taskQ[pool->queueRear].args = args;
	pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity;
	pool->queueSize++;
	pthread_cond_signal(&pool->notEmpty);
	pthread_mutex_unlock(&pool->mutexPool);
}

int getBusyNum(ThreadPool *pool) {
	pthread_mutex_lock(&pool->mutexBusy);
	return pool->busyNum;
	pthread_mutex_unlock(&pool->mutexBusy);
}

int getLiveNum(ThreadPool *pool) {
	pthread_mutex_lock(&pool->mutexPool);
	return pool->liveNum;
	pthread_mutex_unlock(&pool->mutexPool);
}

int destroyPool(ThreadPool *pool) {
	if (pool == NULL) {
		return -1;
	}
	pool->shutdown = 1;
	pthread_join(pool->managerID, NULL);
	for (int i = 0; i < pool->liveNum; i++) {
		pthread_cond_signal(&pool->notEmpty);
	}
	if (pool->taskQ) {
		free(pool->taskQ);
	}
	if (pool->threadIDs) {
		free(pool->threadIDs);
	}
	free(pool);
	pthread_mutex_destroy(&pool->mutexBusy);
	pthread_mutex_destroy(&pool->mutexPool);
	pthread_cond_destroy(&pool->notEmpty);
	pthread_cond_destroy(&pool->notFull);
	pool = NULL;
	return 1;
}