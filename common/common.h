/*************************************************************************
	> File Name: common.h
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Mon 06 Jul 2020 04:26:44 PM CST
 ************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H

char *get_conf_value(const char *path, const char *key);
int socket_create(int port);
void make_block(int fd);
void make_non_block(int fd);
#ifdef _D
#define DBG(fmt, args...) printf(fmt, ##args)
#else 
#define DBG(fmt, args...) 
#endif
#endif
