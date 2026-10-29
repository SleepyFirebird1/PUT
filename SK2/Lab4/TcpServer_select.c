#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include<sys/select.h>
#include <unistd.h>
#include <fcntl.h> 


void setnonblock(int socket) 
{
    int flag = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flag | O_NONBLOCK);

}

int main(){
    int serverSocket;
    int port = 1100;
    struct sockaddr_in serverAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serverAddr,0,sizeof serverAddr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    setnonblock(serverSocket);
    if (bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == -1)
    {
            perror("Cannot bind: ");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    listen(serverSocket,5);

    fd_set current_sockets, ready_sockets, write_sockets;

    //set fd_sets !!!
    FD_ZERO(&current_sockets); 
    FD_ZERO(&ready_sockets); 
    FD_ZERO(&write_sockets);

    FD_SET(serverSocket, &current_sockets);

    static struct timeval timeout;
    timeout.tv_sec=300;
    timeout.tv_usec=0;
    int client_socket;
    int maxfd;
    int select_res;
    maxfd=serverSocket;


    for(;;)
    {
        ready_sockets = current_sockets;
        write_sockets = current_sockets;
        timeout.tv_sec=300;
        select_res=select(maxfd+1, &ready_sockets, &write_sockets, NULL, &timeout);

        if (select_res == 0)
        {
            printf("timeout\n");
            timeout.tv_sec=300;
        }

        if(FD_ISSET(serverSocket, &ready_sockets))
        {
            client_socket=accept(serverSocket,NULL,NULL);
            printf("accepted: %i\n", client_socket);
            setnonblock(client_socket);
            FD_SET(client_socket, &current_sockets);
            if (client_socket > maxfd)
            {
                maxfd = client_socket;
            }
        }
        for (int i = serverSocket + 1; i <= maxfd; i++)
        {
            if(FD_ISSET(i, &write_sockets))
            {
                printf("sending \n");                 
                send(i,"hello world",12,0);         
                close(i);
                FD_CLR(i, &current_sockets);
                if (i == maxfd)
                {
                    while(maxfd>serverSocket && !FD_ISSET(maxfd, &current_sockets))
                    {
                        maxfd -= 1;
                    }
                }     
            }
        }
    }
}

