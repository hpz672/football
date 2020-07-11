/*************************************************************************
	> File Name: thread_pool.c
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Thu 09 Jul 2020 02:50:28 PM CST
 ************************************************************************/

#include "head.h"
extern int repollfd, bepollfd;

void do_work(struct User *user){
    struct ChatMsg msg;
    bzero(&msg, sizeof(struct ChatMsg));
    int rec = recv(user->fd, (void*)&msg, sizeof(msg), 0);
    //printf("msg: %s\nname: %s\ntype: %d\n", msg.msg, msg.name, msg.type);
    //printf("msg: %s\nname: %s\ntype: %d\n", msg.msg, user->name, msg.type);
    DBG(L_PINK"Recv Datapack, size: %d, msg: %d\n"NONE, rec, (int)sizeof(msg));
    if(msg.type & CHAT_WALL) {
        DBG(L_BLUE"msg.type: %d, msg.name: %s\n"NONE, msg.type, msg.name); 
        send_all(&msg);
        //send(user->fd, (void *)&msg, sizeof(msg), 0);
        DBG(L_BLUE"msg.type: %d, msg.name: %s, msg.msg: %s\n"NONE, msg.type, msg.name, msg.msg); 
        printf("<%s> ~ %s\n", user->name, msg.msg);
    } else if(msg.type & CHAT_MSG) {
        char target_name[32] = {0};
        int name_len = 0;
        for(int k = 0; k < 31; k++) {
            if(msg.msg[k + 1] == 0x09 || msg.msg[k + 1] == 0x20 || msg.msg[k + 1] == 0x00) {
                target_name[k] = '\0';
                name_len = k;
                break;
            }
            target_name[k] = msg.msg[k + 1];
        }
        DBG(BLUE"Name: %s\n"NONE, target_name);
        int fd_sent = find_person(target_name);
        if(fd_sent < 0) {
            strcpy(msg.msg, "Target Not Online.");
            strcpy(msg.name, "System #");
            msg.type = CHAT_SYS;
            send(user->fd, (void *)&msg, sizeof(msg), 0);
        } else {
            char target_word[1024] = {0};
            strcpy(target_word, msg.msg);
            strcpy(msg.msg, target_word + name_len + 2);
            DBG(BLUE"Send Data: "NONE"%s\n", msg.msg);
            send(fd_sent, (void *)&msg, sizeof(msg), 0);
            printf("<%s> $ %s\n", user->name, msg.msg);
        }
    } else if(msg.type & CHAT_FUNC) {
        if(msg.msg[0] != '#' || msg.msg[2] != '\0') {
            strcpy(msg.msg, "Invalid Common.");
            strcpy(msg.name, "System #");
            msg.type = CHAT_SYS;
            send(user->fd, (void *)&msg, sizeof(msg), 0);
        } else if(msg.msg[1] == '1') {
            char list[1024];
            find_list(list);
            strcpy(msg.msg, list);
            DBG(BLUE"Send Data :"NONE"%s\n", msg.msg);
            msg.type = CHAT_SYS;
            send(user->fd, (void *)&msg, sizeof(msg), 0);
        } else if(msg.msg[1] == '2') {
            //DBG(BLUE"Send Data :"NONE"%s\n", msg.msg);
            msg.type = CHAT_SYS;
            strcpy(msg.msg, "Anonymous Function: Not finished.");
            DBG(BLUE"Send Data :"NONE"%s\n", msg.msg);
            send(user->fd, (void *)&msg, sizeof(msg), 0);
        } else {
            strcpy(msg.msg, "Invalid Common.");
            strcpy(msg.name, "System #");
            msg.type = CHAT_SYS;
            send(user->fd, (void *)&msg, sizeof(msg), 0);
        }
    } else if(msg.type & CHAT_FIN) {
        user->online = 0;
        int epollfd = user->team ? bepollfd : repollfd;
        del_event(epollfd, user->fd);
        printf(GREEN"Server Info"NONE" # %s logout.\n", user->name);
        close(user->fd);
    }
}

void task_queue_init(struct task_queue *taskQueue, int sum, int epollfd) {
    taskQueue->sum = sum;
    taskQueue->epollfd = epollfd;
    taskQueue->team = calloc(sum, sizeof(void *));
    taskQueue->head = taskQueue->tail = 0;
    pthread_mutex_init(&taskQueue->mutex, NULL);
    pthread_cond_init(&taskQueue->cond, NULL);
}

void task_queue_push(struct task_queue *taskQueue, struct User *user) {
    pthread_mutex_lock(&taskQueue->mutex);
    taskQueue->team[taskQueue->tail] = user;
    DBG(L_GREEN"Thread Pool"NONE" : Task push %s\n", user->name);
    if (++taskQueue->tail == taskQueue->sum) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");
        taskQueue->tail = 0;
    }
    pthread_cond_signal(&taskQueue->cond);
    pthread_mutex_unlock(&taskQueue->mutex);
}


struct User *task_queue_pop(struct task_queue *taskQueue) {
    pthread_mutex_lock(&taskQueue->mutex);
    while (taskQueue->tail == taskQueue->head) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue Empty, Waiting For Task\n");
        pthread_cond_wait(&taskQueue->cond, &taskQueue->mutex);
    }
    struct User *user = taskQueue->team[taskQueue->head];
    DBG(L_GREEN"Thread Pool"NONE" : Task Pop %s\n", user->name);
    if (++taskQueue->head == taskQueue->sum) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");
        taskQueue->head = 0;
    }
    pthread_mutex_unlock(&taskQueue->mutex);
    return user;
}

void *thread_run(void *arg) {
    pthread_detach(pthread_self());
    struct task_queue *taskQueue = (struct task_queue *)arg;
    while (1) {
        struct User *user = task_queue_pop(taskQueue);
        //bzero(user, sizeof(struct User));
        do_work(user);
    }
}

