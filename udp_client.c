#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mqueue.h>
#include <signal.h>

static void close_task();

double buff[5];
int fd, bytes_read;
int main(int argc, char *argv[]) 
{
    

    int n;
    struct sockaddr_in socket_addr;

    //Zrobienie FIFO
    if((mkfifo("my_fifo", 0664) == -1) && (errno != EEXIST))
    {
        fprintf(stderr, "Cannot create FIFO\n");
        return 0;
    }
    if((fd = open("my_fifo", O_RDONLY))==-1)
    {
        fprintf(stderr, "Cannot open FIFO from child\n");
        return 0;
    }

    //Zrobienie klienta UDP
    int my_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if(my_socket == -1)
    {
        fprintf(stderr, "Cannot create client socket\n");
        return 0;
    }
    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(1100);
    socket_addr.sin_addr.s_addr = INADDR_ANY;

    
    //obsluga sygnalu zamykajacego
    struct sigaction act;
    act.sa_handler = close_task;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if(sigaction(SIGRTMIN, &act, NULL) < 0)
    {
        fprintf(stderr, "Cannot register SIGRTMIN handler\n");
        return 0;
    }

    while(1)
    {
        memset(buff, 0, sizeof(buff));
        if ((bytes_read = read (fd, buff, sizeof (buff))) == -1) {
            fprintf(stderr, "Something is wrong with FIFO.\n" ); 
            return 0; 
        }
        if (bytes_read == 0)
            continue;
        if (bytes_read > 0) {
            sendto(my_socket, buff, sizeof(buff), MSG_CONFIRM, (const struct sockaddr *)&socket_addr, sizeof(socket_addr));
        }
    }
    return EXIT_SUCCESS;
}

void close_task()
{
    close(fd);
    remove("my_fifo");
    printf("Closing UDP client...\n");
    fflush(stdout);
    exit(0);
}
