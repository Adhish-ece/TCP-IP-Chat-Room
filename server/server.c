#include "server.h"
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

int main()
{
    int server_fd,client_fd;
    struct sockaddr_in server_addr,client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if((server_fd = socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    int opt = 1;

    setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    if(bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    

    if(listen(server_fd,10)<0)
    {
        perror("Listen Failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("[SERVER] TCP Chat Sever is running on port %d....\n",PORT);

    pthread_mutex_lock(&db_mutex);
    FILE *fp = fopen(DB_FILE,"r");
    if(fp)
    {
        UserRecord users[MAX_CLIENT];
        int count = 0;
        while(fscanf(fp,"%s %s %d %d",users[count].username, users[count].password, &users[count].status, &users[count].sock_fd)==4)
        {
            users[count].status = OFFLINE;
            users[count].sock_fd = -1;
            count++;
        }
        fclose(fp);
        fp = fopen(DB_FILE,"w");
        for(int i = 0;i<count; i++)
        {
            fprintf(fp,"%s %s %d %d\n",users[i].username, users[i].password, users[i].status, users[i].sock_fd);
        }
        fclose(fp);
    }
    pthread_mutex_unlock(&db_mutex);

    while(1)
    {
        socklen_t addr_len = sizeof(client_addr);
        client_fd = accept(server_fd,(struct sockaddr *)&client_addr,&addr_len);
        if(client_fd<0)
        {
            perror("Accept failed");
            continue;
        }
        int *new_sock = malloc(sizeof(int));
        *new_sock = client_fd;
        pthread_t tid;
        pthread_create(&tid,NULL,handle_client,(void * )new_sock);
        pthread_detach(tid);
    }
close(server_fd);
return 0;

}