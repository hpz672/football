/*************************************************************************
	> File Name: udp_epoll.c
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Thu 09 Jul 2020 04:40:38 PM CST
 ************************************************************************/

#include "head.h"
extern int port;
extern int repollfd, bepollfd;
struct User *rteam, *bteam;

void add_event_ptr(int epollfd, int fd, int events, struct User *user) {
	struct epoll_event ev;
	ev.data.ptr = user;
	ev.events = events;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void del_event(int epollfd, int fd) {
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
}

int udp_connect(struct sockaddr_in *client) {
    int sockfd;
    if ((sockfd = socket_create_udp(port)) < 0) {
        perror("socket_create_udp");
        return -1;
    }
    if (connect(sockfd, (struct sockaddr *)client, sizeof(struct sockaddr)) < 0) {
        return -1;
    }
    return sockfd;
}


int check_online(struct LogRequest *request) {
	for(int i = 0; i < MAX; i++) {
		if(rteam[i].online && strcmp(request->name, rteam[i].name) == 0) return 1;
		if(bteam[i].online && strcmp(request->name, bteam[i].name) == 0) return 1;
	}
	return 0;
}

int udp_accept(int fd, struct User *user) {
    int new_fd, ret;
    struct sockaddr_in client;
    struct LogRequest request;
    struct LogResponse response;
    bzero(&request, sizeof(request));
    bzero(&response, sizeof(response));
    socklen_t len = sizeof(client);
    
    ret = recvfrom(fd, (void *)&request, sizeof(request), 0, (struct sockaddr *)&client, &len);
    
    if (ret != sizeof(request)) {
        response.type = 1;
        strcpy(response.msg, "Login failed with Data errors!");
        sendto(fd, (void *)&response, sizeof(response), 0, (struct sockaddr *)&client, len);
        return -1;
    }
    
    if (check_online(&request)) {
        response.type = 1;
        strcpy(response.msg, "You are Already Login!");
        sendto(fd, (void *)&response, sizeof(response), 0, (struct sockaddr *)&client, len);
        return -1;
    }

    response.type = 0;
    strcpy(response.msg, "Login Success. Enjoy yourself!");
    sendto(fd, (void *)&response, sizeof(response), 0, (struct sockaddr *)&client, len);
    
    if (request.team) {
        DBG(GREEN"Info"NONE" : "BLUE"%s login on %s:%d  <%s>\n", request.name, inet_ntoa(client.sin_addr), ntohs(client.sin_port), request.msg);
    } else {
        DBG(GREEN"Info"NONE" : "RED"%s login on %s:%d   <%s>\n", request.name, inet_ntoa(client.sin_addr), ntohs(client.sin_port), request.msg);
    }

    strcpy(user->name, request.name);
    user->team = request.team;
    new_fd = udp_connect(&client);
    user->fd = new_fd;
    return new_fd;
}

int find_sub(struct User *team) {
	for(int i = 0; i < MAX; i++) {
		if(team[i].online == 0)
			return i;
	}
	return -1;
}

void send_all(struct ChatMsg *msg) {
    for(int i = 0; i < MAX; i++) {
        if(rteam[i].online == 1) {
            DBG(BLUE"fd%d: %d\n", i, rteam[i].fd);
            send(rteam[i].fd, (void *)msg, sizeof(struct ChatMsg), 0);
            DBG(BLUE"size: %d\nlocation: %p\n"NONE, (int)sizeof(msg), msg);
        }
        if(bteam[i].online == 1) {
            DBG(BLUE"fd%d: %d\n", i, bteam[i].fd);
            send(bteam[i].fd, (void *)msg, sizeof(struct ChatMsg), 0);
        }
    }
}

void close_all() {
    for(int i = 0; i < MAX; i++) {
        if(rteam[i].online == 1) {
            rteam[i].online = 0;
            close(rteam[i].fd);
        }
        if(bteam[i].online == 1) {
            bteam[i].online = 0;
            close(bteam[i].fd);
        }
    }
}

int find_person(char *name) {
    for(int i = 0; i < MAX; i++) {
        if(rteam[i].online == 1 && strcmp(rteam[i].name, name) == 0) return rteam[i].fd;
        if(bteam[i].online == 1 && strcmp(bteam[i].name, name) == 0) return bteam[i].fd;
    }
    return -1;
}

int find_list(char *list) {
    strcpy(list, "# List #\n");
    int i, k = 0;
    for(i = 0; i < MAX; i++) {
        if(rteam[i].online == 1) {
            strcat(list, "(red)  ");
            strcat(list, rteam[i].name);
            strcat(list, "\n");
            k++;
        }
    }
    for(i = 0; i < MAX; i++) {
        if(bteam[i].online == 1) {
            strcat(list, "(blue) ");
            strcat(list, bteam[i].name);
            strcat(list, "\n");
            k++;
        }   
    }
    char str[32];
    sprintf(str, "Total %d player online.\n", k);
    strcat(list, str);
    return k;
}

void add_to_sub_reactor(struct User *user) {
	struct User *team = (user->team ? bteam : rteam);
	int sub = find_sub(team);
	if(sub < 0) {
		fprintf(stderr, "Team Full.\n");
		return ;
	}
	team[sub] = *user;
	team[sub].online = 1;
	team[sub].flag = 10;
	DBG(L_RED"sub = %d, name = %s\n", sub, team[sub].name);
	if(user->team)
		add_event_ptr(bepollfd, team[sub].fd, EPOLLIN | EPOLLET, &team[sub]);
	else
		add_event_ptr(repollfd, team[sub].fd, EPOLLIN | EPOLLET, &team[sub]);
}
