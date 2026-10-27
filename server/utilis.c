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
                

            }
        }

    }
}

void broadcast_system_message()
{

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