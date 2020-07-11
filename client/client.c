/*************************************************************************
	> File Name: client.c
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Wed 08 Jul 2020 04:32:12 PM CST
 ************************************************************************/

#include "head.h"

int server_port = 0;
char server_ip[20] = {0};
int team = -1;
char name[20] = {0};
char log_msg[512] = {0};
char *conf = "./football.conf";
int sockfd = -1;

void logout(int signum) {
    struct ChatMsg msg;
    msg.type = CHAT_FIN;
    send(sockfd, (void *)&msg, sizeof(msg), 0);
    close(sockfd);
    DBG("\nBye.\n");
    exit(0);
}

int main(int argc, char **argv) {
    int opt;
    struct LogRequest request;        //HERE
    struct LogResponse response;     //HERE
    bzero(&request, sizeof(request));
    bzero(&response, sizeof(response));
    while ((opt = getopt(argc, argv, "h:p:t:m:n:")) != -1) {
        switch (opt) {
            case 't':
                request.team = atoi(optarg); //HERE
                break;
            case 'h':
                strcpy(server_ip, optarg);
                break;
            case 'p':
                server_port = atoi(optarg);
                break;
            case 'm':
                strcpy(request.msg, optarg); //HERE
                break;
            case 'n':
                strcpy(request.name, optarg); //HERE
                break;
            default:
                fprintf(stderr, "Usage : %s [-hptmn]!\n", argv[0]);
                exit(1);
        }
    }


    if (!server_port) server_port = atoi(get_conf_value(conf, "SERVERPORT"));
    if (!request.team) team = atoi(get_conf_value(conf, "TEAM")); //HERE
    if (!strlen(server_ip)) strcpy(server_ip, get_conf_value(conf, "SERVERIP"));
    if (!strlen(request.name)) strcpy(request.name, get_conf_value(conf, "NAME"));//HERE
    if (!strlen(request.msg)) strcpy(request.msg, get_conf_value(conf, "LOGMSG"));//HERE

    printf("ip: %s\n", server_ip);
    printf("name: %s\n", request.name);
    printf("log_msg: %s\n", request.msg);
    for(int j = 0; j < 20; j++) {
        if(server_ip[j] == 13)
            server_ip[j] = 0;
        if(request.name[j] ==13)
            request.name[j] = 0;
    }
    for(int j = 0; j < 512; j++) {
        if(request.msg[j] == 13) {
            request.msg[j] = 0;
            break;
        }
    }

    DBG("<""Conf Show""> : server_ip = %s, port = %d, team = %s, name = %s\nlog_msg: %s\n", server_ip, server_port,(request.team ? "BLUE": "RED"), request.name, request.msg);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr.s_addr = inet_addr(server_ip);

    socklen_t len = sizeof(server);

    if ((sockfd = socket_udp()) < 0) {
        perror("socket_udp()");
        exit(1);
    }

    sendto(sockfd, (void *)&request, sizeof(request), 0, (struct sockaddr *)&server, len);//HERE
    DBG("<"L_BLUE"Sent Data"NONE">: Send data to server.\n");

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    
    int retval;
    if((retval = select(sockfd + 1, &rfds, NULL, NULL, &tv)) < 0) {
        perror("select()");
        exit(1);
    }    
    else if(retval > 0) {
        int ret = recvfrom(sockfd, (void *)&response, sizeof(response), 0, (struct sockaddr *)&server, &len);
        if(sizeof(response) != ret || response.type == 1) {
            perror("response()");
            printf("Response msg: %s\n", response.msg);
            //close(sockfd);
            exit(1);
        }
    }
    else {
        DBG(RED"Error"NONE": Server not connected.\n");
        exit(1);
    }
    connect(sockfd, (struct sockaddr*)&server, len);
    DBG(GREEN"Server"NONE" ; %s.\n", response.msg);

    pthread_t recv_t;
    pthread_create(&recv_t, NULL, do_recv, NULL);

    signal(SIGINT, logout);
    while(1) {
        //char buff[512] = {0};
        struct ChatMsg msg;
        bzero(&msg, sizeof(struct ChatMsg));
        //msg.type = CHAT_WALL;
        printf(RED"Please Input: \n"NONE);
        strcpy(msg.name, request.name);
        for(int j = 0; j < 20; j++) {
            //printf("%d: %c, %d;\n", j, msg.name[j], msg.name[j]);
            if(msg.name[j] == 13)
                msg.name[j] = 0;
        }
        scanf("%[^\n]s", msg.msg);
        getchar();
        //buff[strlen(buff)] = '\0';
        if(strlen(msg.msg)) {
            if(msg.msg[0] == '@') {
                msg.type = CHAT_MSG;
            }
            else if(msg.msg[0] == '#') {
                msg.type = CHAT_FUNC;
            }
            else {
                msg.type = CHAT_WALL;
            }
            DBG(BLUE"sockfd: %d\n"NONE, sockfd); 
            send(sockfd, &msg, sizeof(struct ChatMsg), 0);
        }
        //send(sockfd, &msg, sizeof(struct ChatMsg), 0);
    }

    return 0;
}
