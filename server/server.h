#ifndef SERVER_H
#define SERVER_H

#include "common.h"
void *handle_client(void *argc);
int register_user(const char *username,const char *password, char *reply_msg);
int login_user(const char *username,const char *password,int sock_fd, char *reply_msg);
void send_online_users_list(int sock_fd);
#endif