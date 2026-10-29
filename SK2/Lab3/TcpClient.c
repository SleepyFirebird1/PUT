#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include<pthread.h>

void * clientThread(void *arg)
{
    printf("In thread\n");

    pthread_exit(NULL);
}

int main(){

    char message[256];
    char buffer[256];
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;
    int port = 1100;

    // Create the socket. 
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    //Configure settings of the server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //Connect the socket to the server using the address
    addr_size = sizeof serverAddr;
    connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
    bzero(message, 256);
    scanf("%s", message);

    if (send(clientSocket, message, sizeof(message), 0) < 0)
    {
        printf("Send failed\n");
    }    
    close(clientSocket);

    return 0;
}