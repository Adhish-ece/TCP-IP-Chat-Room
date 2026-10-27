#ifndef SERVER_H
#define SERVER_H

#include "common.h"
void *handle_client(void *argc);
int register_user(const char *username,const char *password, char *reply_msg);
int login_user(const char *username,const char *password,int sock_fd, char *reply_msg);
void send_online_users_list(int sock_fd);
void broadcast_system_message(const char *msg,char *exclude_user);
void logout_user(const char *username);
void multicast_group_message(const char *sender,char *msg);
void unicast_single_message(const char *sender,const char *receiver,const char *msg, int sender_sock);
#endif