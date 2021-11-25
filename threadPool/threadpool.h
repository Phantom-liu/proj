#ifndef _THREADPOOL_H
#define _THREADPOOL_H

typedef struct ThreadPool ThreadPool;
//�����̳߳ز���ʼ��
ThreadPool* threadPoolCreate(int min, int max, int queueSize);

//�����̳߳�
int destroyPool(ThreadPool *pool);

//���̳߳��������
void addWork(ThreadPool *pool, void(*func)(void*), void *args);

//��ȡ�̳߳��й����̸߳���
int getBusyNum(ThreadPool *pool);

//��ȡ�̳߳��л��ŵ��̸߳���
int getLiveNum(ThreadPool *pool);


///////////////////////////
void* worker(void *args);
void* manager(void *args);
void* threadExit(ThreadPool *pool);

#endif