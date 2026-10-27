#include "server.h"

extern pthread_mutex_t db_mutex;
void *handle_client(void *argc)
{
    int sock_fd = *(int *)argc;
    free(argc);
    ClientRequest req;
    ServerReply reply;

    char logged_in_user[USERNAME_LEN] = "";
    int authenticated = 0;
    while(!authenticated)
    {
        ssize_t bytes_read = recv(sock_fd,&req,sizeof(ClientRequest),0);
        if(bytes_read<=0)
        {
            close(sock_fd);
            return NULL;
        }
        memset(&reply,0,sizeof(ServerReply));
        if(req.option == REQ_REGISTER)
        {
            if(register_user(req.username,req.password,reply.message))
            {
                reply.success = 1;
            }
            else
            {
                reply.success = 0;
            }
            send(sock_fd,&reply,sizeof(ServerReply),0);
        }
        else if(req.option == REQ_LOGIN)
        {
            if(login_user(req.username,req.password,sock_fd,reply.message))
            {
                reply.success = 1;
                authenticated = 1;
                strcpy(logged_in_user,req.username);
                send(sock_fd,&reply,sizeof(ServerReply),0);
                send_online_users_list(sock_fd);

                char notif[MAX_BUFFER];
                snprintf(notif,sizeof(notif),"User %s is ONLINE",logged_in_user);
                broadcast_system_message(notif,logged_in_user);
            }
            else
            {
                reply.success = 0;
                send(sock_fd,&reply,sizeof(ServerReply),0);
            }
        }

    }

    ClientMessage cmsg;
    while(1)
    {
        ssize_t bytes = recv(sock_fd,&cmsg,sizeof(ClientMessage),0);
        if(bytes <=0 || cmsg.chat_option == CHAT_LOGOUT)
        {
            logout_user(logged_in_user);
            ClientMessage logout_reply;
            memset(&logout_reply,0,sizeof(ClientMessage));
            strcpy(logout_reply.sender,"Sever");
            strcpy(logout_reply.message,"Logged out successfully");
            send(sock_fd,&logout_reply,sizeof(ClientMessage),0);

            char notif[MAX_BUFFER];
            snprintf(notif,sizeof(notif),"User %s is OFFLINE",logged_in_user);
            broadcast_system_message(notif,logged_in_user);
            break;
        }

        if(cmsg.chat_option == CHAT_GROUP)
        {
            multicast_group_message(logged_in_user,cmsg.message);
        }
        else if(cmsg.chat_option == CHAT_SINGLE)
        {

        }
    }
}

void unicast_single_message(const char *sender,const char *receiver,const char *msg, int sender_sock)
{
    pthread_mutex_lock(&db_mutex);
    FILE *fp = fopen(DB_FILE,"r");
    if(!fp)
    {
        pthread_mutex_lock(&db_mutex);
        return;
    }
    char u[USERNAME_LEN],p[PASSWD_LEN];
    int st,fd;
    
    int receiver_found = 0;
    int receiver_fd = -1;
    int receiver_online = 0;
    while(fscanf(fp,"%s %s %d %d",u,p,&st,&fd)==4)
    {
        if(strcmp(u,receiver)==0)
        {
            receiver_found = 1;
            receiver_fd = fd;
            receiver_online = (st==ONLINE);
            break;
        }
    }
    flcose(fp);
    pthread_mutex_unlock(&db_mutex);

    ClientMessage cmsg;
    memset(&cmsg,0,sizeof(ClientMessage));
    strcpy(cmsg.sender,sender);
    strcpy(cmsg.reciever,receiver);
    strcpy(cmsg.message,msg);
    cmsg.chat_option = CHAT_SINGLE;
    if(!receiver_found)
    {
        ClientMessage err_msg;
        memset(&err_msg,0,sizeof(ClientMessage));
        strcpy(cmsg.sender,"Server");
        snprintf(err_msg.message,sizeof(err_msg.message),"Error User %s not found",receiver);
        send(sender_sock,&err_msg,sizeof(ClientMessage),0);
    }
    else if(!receiver_online)
    {
        ClientMessage err_msg;
        memset(&err_msg,0,sizeof(ClientMessage));
        strcpy(cmsg.sender,"Server");
        snprintf(err_msg.message,sizeof(err_msg.message),"Error User %s offline",receiver);
        send(sender_sock,&err_msg,sizeof(ClientMessage),0);
    }
    else
    {
        send(receiver_fd,&cmsg,sizeof(ClientMessage),0);
    }

}

void multicast_group_message(const char *sender,char *msg)
{
    pthread_mutex_lock(&db_mutex);
    FILE *fp = fopen(DB_FILE,"r");
    if(!fp)
    {
        pthread_mutex_unlock(&db_mutex);
        return;
    }
    char u[USERNAME_LEN],p[PASSWD_LEN];
    int st,fd;
    ClientMessage cmsg;
    strcpy(cmsg.sender,sender);
    strcpy(cmsg.message,msg);
    while(fscanf(fp,"%s %s %d %d",u,p,&st,&fd)==4)
    {
        if(st==ONLINE && strcmp(u,sender) != 0 && fd>0)
        {
            send(fd,&cmsg,sizeof(ClientMessage),0);
        }
    }
    fclose(fp);
    pthread_mutex_unlock(&db_mutex);
}

void logout_user(const char *username)
{
    pthread_mutex_lock(&db_mutex);
    FILE *fp = fopen(DB_FILE,"r");
    if(!fp)
    {
        pthread_mutex_unlock(&db_mutex);
        return;
    }
    UserRecord users[MAX_CLIENT];
    int count = 0;
    while(fscanf(fp,"%s %s %d %d",users[count].username, users[count].password, &users[count].status, &users[count].sock_fd) == 4)
    {
        if(strcmp(users[count].username,username)==0)
        {
            users[count].status = ONLINE;
            users[count].sock_fd = -1;          
        
        }
        count++;
    }
    fclose(fp);
    fp = fopen(DB_FILE,"w");
    for(int i =0;i<count;i++)
    {
        fprintf(fp,"%s %s %d %d",users[count].username, users[count].password, users[count].status, users[count].sock_fd);
    }
    fclose(fp);
    pthread_mutex_unlock(&db_mutex);
}

void broadcast_system_message(const char *msg,char *exclude_user)
{
    pthread_mutex_lock(&db_mutex);
    FILE *fp = fopen(DB_FILE,"r");
    if(!fp)
    {
        pthread_mutex_unlock(&db_mutex);
        return;
    }
    char u[USERNAME_LEN],p[PASSWD_LEN];
    int st,fd;
    ClientMessage sys_msg;

    memset(&sys_msg,0,sizeof(sys_msg));
    strcpy(sys_msg.sender,"Server");
    strcpy(sys_msg.message,msg);
    while(fscanf(fd,"%s %s %d %d",u,p,st,fd)==4)
    {
        if(st==ONLINE && strcmp(u,exclude_user)!=0)
        {
            send(fd,&sys_msg,sizeof(ClientMessage),0);
        }
    }
    fclose(fp);
    pthread_mutex_unlock(&db_mutex);

    
}


void send_online_users_list(int sock_fd)
{
    pthread_mutex_lock(&db_mutex);

    FILE *fp = fopen(DB_FILE,"r");
    if(!fp)
    {
        pthread_mutex_unlock(&db_mutex);
        return;
    }
    char u[USERNAME_LEN],p[PASSWD_LEN];
    int st,fd;
    ClientMessage sys_msg;
    while(fscanf("%s %s %d %d",u,p,st,fd))
    {
        memset(&sys_msg,0,sizeof(ClientMessage));
        snprintf(sys_msg.message,sizeof(sys_msg.message),"User %s is online",u);
        send(sock_fd,&sys_msg,sizeof(ClientMessage),0);
    }
    fclose(fp);
    pthread_mutex_unlock(&db_mutex);
}

int login_user(const char *username,const char *password,int sock_fd, char *reply_msg)
{
    pthread_mutex_lock(&db_mutex);
    FILE *fp = fopen(DB_FILE,"r");
    if(!fp)
    {
        pthread_mutex_unlock(&db_mutex);
        strcpy(reply_msg,"user name not found");
        return 0;
    }
    UserRecord users[MAX_CLIENT];
    int count = 0,found = 0,auth_success =0;
    while(fscanf(fp,"%s %s %d %d",users[count].username, users[count].password, &users[count].status, &users[count].sock_fd) == 4)
    {
        if(strcmp(users[count].username,username)==0)
        {
            found = 1;
            if(strcmp(users[count].password,password)==0)
            {
                users[count].status = ONLINE;
                users[count].sock_fd = sock_fd;
                auth_success = 1;
            }
            else
            {
                strcpy(reply_msg,"Password not matching");
            }
        }
        count++;
    }
    fclose(fp);
    if(!found)
    {
        strcpy(reply_msg,"user name not found");
    }
    else if(auth_success)
    {
        fp = fopen(DB_FILE,"w");
        for(int i =0;i<count;i++)
        {
            fprintf(fp,"%s %s %d %d\n",users[count].username, users[count].password, &users[count].status, &users[count].sock_fd);
        }
        fclose(fp);
        strcpy(reply_msg,"Loggin successful");
    }
    pthread_mutex_unlock(&db_mutex);
    return auth_success;
}

int register_user(const char *username,const char *password, char *reply_msg)
{
    pthread_mutex_lock(&db_mutex);
    FILE *fp = fopen(DB_FILE,"a+");
    if(!fp)
    {
        pthread_mutex_unlock(&db_mutex);
        strcpy(reply_msg,"Database error");
        return 0;
    }
    fseek(fp,0,SEEK_SET);
    char u[USERNAME_LEN],p[PASSWD_LEN];
    int st,fd;
    while(fscanf(fp,"%s %s %d %d",u,p,&st,&fd)==4)
    {
        if(strcmp(username,u)==0)
        {
            fclose(fp);
            pthread_mutex_unlock(&db_mutex);
            strcpy(reply_msg,"Username already exist..");
            return 0;
        }
    }
    fprintf(fd,"%s %s %d %d",username,password,OFFLINE,-1);
    fclose(fp);
    pthread_mutex_unlock(&db_mutex);
    strcpy(reply_msg,"Registration successful");
    return 1;
}