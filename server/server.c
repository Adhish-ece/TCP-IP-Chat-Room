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
    
}