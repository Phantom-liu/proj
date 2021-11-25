#include <stdio.h>
#include "threadpool.h"
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

void taskfun(void *args) {
	printf("thread %d is working, tid = %ld\n", *((int*)args), pthread_self());
	usleep(1000);
}
int main()
{
	//创建线程池
	ThreadPool *pool = threadPoolCreate(3,10,100);
	for (int i = 0; i < 100; i++) {
		int *num = (int*)malloc(sizeof(int));
		*num = i + 100;
		addWork(pool, taskfun, num);
	}
	sleep(30);
	destroyPool(pool);
	return 0;
}