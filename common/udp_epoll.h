/*************************************************************************
	> File Name: udp_epoll.h
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Thu 09 Jul 2020 04:40:49 PM CST
 ************************************************************************/

#ifndef _UDP_EPOLL_H
#define _UDP_EPOLL_H

void add_event_ptr(int epollfd, int fd, int events, struct User *user);
void del_event(int epollfd, int fd);

int udp_connect(struct sockaddr_in *client);
int udp_accept(int fd, struct User *user);

int check_online(struct LogRequest *request);
int find_sub(struct User *team);

void send_all(struct ChatMsg *msg);
void close_all();

int find_person(char *name);
void find_list(char *list);

void add_to_sub_reactor(struct User *user);

#endif
