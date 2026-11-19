#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/ssl.h> 
#include <openssl/bio.h> /* BasicInput/Output streams */
#include <openssl/err.h> /* errors */
#include <openssl/ssl.h> /* core library */

int main(){

struct sockaddr_in server_adress;


SSL_CTX* ctx;

SSL* ssl;

//SSL_load_error_strings(); //#depricated
//SSL_library_init(); //#depricated

ctx= SSL_CTX_new(TLS_server_method()); //TLS_client_method() / TLS_server_method()

SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM); //SSL_FILETYPE_PEM

SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM); //SSL_FILETYPE_PEM

memset(&server_adress, 0, sizeof server_adress);

server_adress.sin_family = AF_INET;
server_adress.sin_port = htons(1100);
server_adress.sin_addr.s_addr = htonl(INADDR_ANY);

int server_socket=socket(AF_INET,SOCK_STREAM,0);
int client_socket;

if (bind(server_socket,(struct sockaddr *)&server_adress, sizeof server_adress) == -1) {
    perror("bind failed");
    close(server_socket);
    exit(EXIT_FAILURE);
}

if (listen(server_socket, 10) == -1) {
    perror("listen failed");
    close(server_socket);
    exit(EXIT_FAILURE);
}
char buff[256];

int rc;
for(;;){
   
    client_socket=accept(server_socket,NULL,NULL);
    ssl=SSL_new(ctx);
    SSL_set_fd(ssl, client_socket);
    SSL_accept(ssl);

    for(;;){
    bzero(buff,256);
    rc=SSL_read(ssl, buff, sizeof(buff));
    printf("recv %d : %s\n",rc, buff);

        if(rc==0){
            printf("DC\n");
            break;
        }
    }


    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client_socket);

    }

SSL_CTX_free(ctx);
close(server_socket);


return EXIT_SUCCESS;

}