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

    ctx=SSL_CTX_new(TLS_client_method()); //TLS_client_method() / TLS_server_method()
    ssl=SSL_new(ctx);

    int fd=socket(AF_INET,SOCK_STREAM,0);
    memset(&server_adress, 0, sizeof(server_adress));
    server_adress.sin_family=AF_INET;
    server_adress.sin_port=htons(1100);
    server_adress.sin_addr.s_addr=inet_addr("127.0.0.1");

    connect(fd,(struct sockaddr*)&server_adress,sizeof server_adress);

    SSL_set_fd(ssl, fd);

    SSL_connect(ssl); //conecting with ssl
    char buff[256];

    for(;;){

        bzero(buff,256);
        fgets(buff,256,stdin);

        if(strcmp(buff, "exit\n") == 0)
        {
            break;
        }
        else
        {
            SSL_write(ssl, buff, strlen(buff));
        }

    }


    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(fd);
    SSL_CTX_free(ctx);

    printf("end \n");

    return EXIT_SUCCESS;
}