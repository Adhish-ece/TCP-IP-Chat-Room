#ifndef COMMON_H
#define COMMON_H

#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include<stdlib.h>

#define PORT 633
#define SERVER_IP "127.0.0.1"
#define MAX_BUFFER 1024
#define USERNAME_LEN 32
#define PASSWD_LEN 32
#define MAX_CLIENT 50
#define DB_FILE "database.txt"


typedef enum
{
    REQ_REGISTER = 1,
    REQ_LOGIN = 2
}RequestOption;

typedef enum
{
    CHAT_SINGLE = 1,
    CHAT_GROUP = 2,
    CHAT_LOGOUT = 3
}ChatOption;

typedef enum
{
    OFFLINE = 0,
    ONLINE = 1
}UserStatus;

typedef struct
{
    char username[USERNAME_LEN];
    char password[PASSWD_LEN];
    int option;
}ClientRequest;

typedef struct 
{
    int success;
    char message[128];
}ServerReply;

typedef struct
{
    char sender[USERNAME_LEN];
    char reciever[USERNAME_LEN];
    char message[MAX_BUFFER];
    int chat_option;
}ClientMessage;

typedef struct 
{
    char username[USERNAME_LEN];
    char password[PASSWD_LEN];
    int status;
    int sock_fd;
}UserRecord;

#endif
