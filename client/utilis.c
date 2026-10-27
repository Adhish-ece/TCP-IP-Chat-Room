#include "client.h"

extern int sock_fd;
extern int is_running;


int verify_input(int option, int id)
{
    if (id == 1)
    {
        
        if (option == 1 || option == 2)
            return 0; 
        else
            return 1; 
    }
    else if (id == 2)
    {
       
        if (option == 1 || option == 2 || option == 3)
            return 0; 
        else
            return 1; 
    }
    return 1;
}

void *recive_handler(void *argc)
{
    ClientMessage msg;
    while(is_running)
    {
        ssize_t bytes = recv(sock_fd,&msg,sizeof(ClientMessage),0);
        if(bytes<=0)
        {
            break;
        }
        if(strcmp(msg.sender,"server") == 0)
        {
            printf("\nincoming message from server:\n%s\n\n",msg.message);
        }
        else if(msg.chat_option == CHAT_SINGLE )
        {
            printf("\n[Private chat] Incoming Message from %s:\n%s\n\n",msg.sender,msg.message);
        }
        else
        {
            printf("\nIncoming Message from %s:\n%s\n\n",msg.sender,msg.message);
        }
    }
    return NULL;
    
    

}