// kompilacja: gcc <nazwa tego pliku> -Wall -o <nazwa pliku wynikowego>
// nadanie uprawnien do uruchomienia pliku wynikowego: chmod +x <nazwa pliku wynikowego>
// uruchomienie: ./<nazwa pliku wynikowego>

//TODO dolaczenie wymaganych pliklow naglowkowych
// poszukaÄ w man <nazwa funkcji> i w man 7 ip
// ewentualnie przy kompilacji z -Wall moĹźe pojawiÄsiÄ sugestia, wskazujÄca ktĂłrego pliku nagĹĂłwkowego brakuje

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main()
{
	struct sockaddr_in server_addr;

	int my_socket = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));

    int port = 1100;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);

    if(bind(my_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind failed: ");
        close(my_socket);
        exit(EXIT_FAILURE);
    }

    listen(my_socket, 10);

    int conn_socket = accept(my_socket, NULL, NULL);

    if (conn_socket == -1)
    {
        perror("Accept fail: ");
        close(conn_socket);
        close(my_socket);
        exit(EXIT_FAILURE);
    } 
    else
    {
        printf("Client connected");
    }
    char send[256];
    char recieve[256];
    for (;;)
    {
        bzero(send, sizeof(send));
        bzero(recieve, sizeof(recieve));
        read(conn_socket, recieve, sizeof(recieve));
        printf("Recieved from client: %s \n", recieve);
        printf("Podaj wiadomość, ktora chcesz wysłać:\n");
        fgets("%s", &send), stdin;
        write(conn_socket, send, strlen(send));
    }
    close(conn_socket);
    close(my_socket);
    
}
