 #include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

int main(void)
{
    struct pollfd poll_set[5];
    struct sockaddr_in sa,ca;
    int SocketFD = socket(TODO);

    bool compressPollArr=false;

    if (SocketFD == -1)
    {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof sa);

    sa.sin_family = TODO;
    sa.sin_port = htons(TODO);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    //fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)

    if (bind(SocketFD,(struct sockaddr *)&sa, sizeof sa) == -1) 
    {
        perror("bind failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (listen(SocketFD, 10) == -1) 
    {
        perror("listen failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    poll_set[0].fd = TODO;
    poll_set[0].events = TODO;
    int n = 0;
    int currentSize=0;
    int ndfs=1;
    char  buffer[256];

    int len= sizeof(ca);
    for (;;) 
    {
        printf("Wait for poll..\n");
        //poll

        if(n<0)
        {
            perror("poll fail\n");
            break;
        }

        

        currentSize=ndfs;

        for(int i=0;i<currentSize;i++)
        {

            if(poll_set[i].revents==0)
            {
                //TODO
            }

            if(poll_set[i].revents!=POLLIN)
            {
                //TODO
            }

            if(poll_set[i].fd==SocketFD)
            {
                for(;;)
                {
                    socklen_t ca_size=sizeof(struct sockaddr);
                    int ConnectFD = accept(TODO);
                    
                    if(ConnectFD<0)
                    {
                        perror("Connection error");
                        break;
                    }
                    else
                    {
                        printf("connected\n");
                    }

                    poll_set[ndfs].fd=TODO;
                    poll_set[ndfs].events=TODO;
                    ndfs++;

                    if(ConnectFD!=-1)
                    {
                        break;
                    }
                }
            }
            else
            {

                for(;;)
                {
                    printf("Can read \n");

                    n=recv(TODO);
                    if(n<0)
                    {
                        break;
                    }

                    if(n==0)
                    {
                        printf("Connection closed \n");
                        break;
                    }

                    //do echo
                }

                close(poll_set[i].fd);
                printf("closing \n");
                poll_set[i].fd=-1;
                compressPollArr=true;
            }
        }

        if(compressPollArr)
        {
            compressPollArr=false;
            for(int i=0;i<ndfs;i++)
            {
                if(poll_set[i].fd==-1)
                {
                for(int j=i;j<ndfs;j++)
                {
                    poll_set[j].fd=poll_set[j+1].fd;
                }
                i--;
                ndfs--;
                }
            }
        }

    }

        
    for(int i=0;i<ndfs;i++)
    {
        if(poll_set[i].fd>=0)
        close(TODO);
    }

    close(SocketFD);
    return EXIT_SUCCESS;  
}