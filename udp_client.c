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

// #include "logger.h"

static void close_task();

double buff[5];
int fd, bytes_read;
int main(int argc, char *argv[]) 
{
    

    int n;//, addr_length;
    struct sockaddr_in socket_addr;
    //printf("Child process, pid = %d, parent's pid = %d\n", getpid(), getppid());

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

        // If there is no data just continue
        if (bytes_read == 0)
            continue;

        // If there is message print it
        if (bytes_read > 0) {
            printf("Message: input %f, output %f, lock1 %d, lock2 %d, time %f\n", buff[0], buff[1], (int)buff[2], (int)buff[3], buff[4]);
            fflush(stdout);
            // zapis danych do plikow
            // mq_send(loggerInputMQueue, (const char *)&buff[0], sizeof(double), 0);
            // mq_send(loggerOutputMQueue, (const char *)&buff[1], sizeof(double), 0);
            // mq_send(loggerLock1MQueue, (const char *)&buff[2], sizeof(double), 0);
            // mq_send(loggerLock2MQueue, (const char *)&buff[3], sizeof(double), 0);
            // wysylka danych do serwera UDP
            sendto(my_socket, buff, sizeof(buff), MSG_CONFIRM, (const struct sockaddr *)&socket_addr, sizeof(socket_addr));
        }
    }
    return EXIT_SUCCESS;
}

void close_task()
{
    // finalize_loggers();
    close(fd);
    remove("my_fifo");
    printf("Quitting...\n");
    fflush(stdout);
    exit(0);
}
