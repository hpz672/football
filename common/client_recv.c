/*************************************************************************
	> File Name: client_recv.c
	> Author: 
	> Mail: 
	> Created Time: Fri 10 Jul 2020 09:32:54 PM CST
 ************************************************************************/

#include "head.h"

extern int sockfd;

void *do_recv(void *arg)
{
    struct ChatMsg msg;
    while(1) {
        bzero(&msg, sizeof(msg));
        int ret = recv(sockfd, (void *)&msg, sizeof(msg), 0);
        DBG(L_PINK"Recv Datapack, size: %d, msg: %d.\n"NONE, ret, (int)sizeof(msg));
        if(ret != sizeof(msg)) {
            DBG(PINK"Not current datapack.\n"NONE);
            continue;
        }
        DBG(L_BLUE"Datapack OK.\n"NONE);
        DBG(L_BLUE"msg.type: %d, msg.name: %s\n"NONE, msg.type, msg.name);
        if(msg.type & CHAT_WALL) {
            printf(L_BLUE"<%s>"NONE": %s\n", msg.name, msg.msg);
        }
        else if(msg.type & CHAT_MSG) {
            printf(L_YELLOW"<%s>"NONE" $ %s\n", msg.name, msg.msg);
        }
        else if(msg.type & CHAT_HEART) {
            msg.type = CHAT_ACK;
            send(sockfd, (void *)&msg, sizeof(msg), 0);
        }
        else if(msg.type & CHAT_SYS) {
            printf(L_GREEN"Server Info"NONE" # %s\n", msg.msg);
        }
        else if(msg.type & CHAT_FIN) {
            printf(L_RED"Server Info"NONE" # Server closed.\n");
            close(sockfd);
            exit(0);
        }
    }
}
