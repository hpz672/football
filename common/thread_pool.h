/*************************************************************************
	> File Name: thread_pool.h
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Thu 09 Jul 2020 02:49:00 PM CST
 ************************************************************************/

#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H
struct task_queue {
    int sum;
    int epollfd; //从反应堆
    struct User **team;
    int head;
    int tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

void task_queue_init(struct task_queue *taskQueue, int sum, int epollfd);
void task_queue_push(struct task_queue *taskQueue, struct User *user);
struct User *task_queue_pop(struct task_queue *taskQueue);
void *thread_run(void *arg);
#endif
