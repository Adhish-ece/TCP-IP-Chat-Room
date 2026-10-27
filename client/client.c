#include "client.h"


int sock_fd = -1;
char current_user[USERNAME_LEN];
int is_running = 1;



int main()
{
    struct sockaddr_in server_addr;
    //creating my socket 
    if((sock_fd = socket(AF_INET,SOCK_STREAM,0))<0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
    printf("Socket created\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    if(connect(sock_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
    {
        perror("Connection Failed");
        exit(EXIT_FAILURE);

    }
    printf("Connected\n");
    ClientRequest req;
    ServerReply reply;
    int logged_in = 0;

    while(!logged_in)
    {
        printf("Enter the option : \n1) Registration\n2 )Login\n");
        scanf("%d",&req.option);
        if(verify_input(req.option,1))
        {
            printf("Enter valid option\n");
            continue;
        }
      
        printf("Enter username: ");
        scanf("%s",req.username);
        printf("Enter password: ");
        scanf("%s",req.password);
        send(sock_fd,&req,sizeof(ClientRequest),0);
        recv(sock_fd,&reply,sizeof(ServerReply),0);
   

        printf("Sever reply :\nmessage : %s\n",reply.message);
        if(reply.success && req.option == REQ_LOGIN)
        {
            logged_in = 1;
            strcpy(current_user,req.username);
        }
        printf("\n");
    }
    printf("List of Online Users\n");

    pthread_t recv_thread;
    pthread_create(&recv_thread,NULL,recive_handler,NULL);

    while(is_running)
    {
        printf("Select\n1. Single chat\n2. Group chat\n3. Logout\n");

        int chat_opt;
        scanf("%d",&chat_opt);
        if(verify_input(chat_opt,2))
        {
            printf("Enter valid option\n");
            continue;
        }

        ClientMessage msg;
        memset(&msg,0,sizeof(ClientMessage));
        strcpy(msg.sender,current_user);
        msg.chat_option = chat_opt;

        if(chat_opt == CHAT_LOGOUT)
        {
            send(sock_fd,&msg,sizeof(ClientMessage),0);
            ClientMessage ack;
            ssize_t bytes = recv(sock_fd,&ack,sizeof(ClientMessage),0);
            if(bytes > 0 && strcmp(ack.sender,"Server")==0)
            {
                printf("Server reply : %s\n",ack.message);
            }
            
            is_running = 0;
            break;
        }
        else if(chat_opt == CHAT_SINGLE)
        {
            getchar();
            printf("Enter recipient username: ");
            scanf("%s",msg.reciever);
            getchar();
            printf("Enter the message to send : ");
            fgets(msg.message,sizeof(msg.message),stdin);
            msg.message[strcspn(msg.message,"\n")]= 0;
            
            send(sock_fd,&msg,sizeof(ClientMessage),0);
            printf("Single message send successufully\n");
        }
        else if(chat_opt == CHAT_GROUP)
        {
            getchar();
            printf("Enter the message to send : ");
            fgets(msg.message,sizeof(msg.message),stdin);
            msg.message[strcspn(msg.message,"\n")]= 0;

            send(sock_fd,&msg,sizeof(ClientMessage),0);
            printf("Single message send successufully\n");
            
            
        }
    }
    pthread_join(recv_thread,NULL);
    close(sock_fd);
    return 0;
}
