#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) 
{
    double buff[5];
    //char buff[100];
    int n, addr_length;
    struct sockaddr_in socket_addr; //socket nasluchujacy
    struct sockaddr client_addr;

    int my_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if(my_socket == -1)
    {
        fprintf(stderr, "Cannot create socket\n");
        return 0;
    }
    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(1100);
    socket_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(my_socket, (struct sockaddr *)&socket_addr, sizeof(socket_addr)) == -1)
    {
        fprintf(stderr, "Cannot bind socket\n");
        close(my_socket);
        return 0;
    }
    addr_length = sizeof(client_addr);
    printf("I'm waiting\n");
    for(;;)
    {
        //n = recvfrom(my_socket, (char *)buff, 100, MSG_WAITALL, &client_addr, &addr_length); 
        n = recvfrom(my_socket, (double *)buff, sizeof(buff), MSG_WAITALL, &client_addr, &addr_length);
        printf("Server received %f, %f, %d, %d, %f\n", buff[0], buff[1], (int)buff[2], (int)buff[3], buff[4]);
        //printf("Received %s", buff);
        fflush(stdout);
    }
}
