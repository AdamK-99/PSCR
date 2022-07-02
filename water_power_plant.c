#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "periodic.h"
#include "calculations.h"
// #include "logger.h"
#include "keyboard.h"

int main(int argc, char *argv[])
{    
    pid_t pid;
    char * child_arg[3] = {"./udp_client",NULL};
    pid = fork();

    if(pid == 0) //child process
    {
        execvp(child_arg[0], child_arg);
    }
    //printf("Parent program, pid = %d\n", getpid());


    char c;

    //inicjalizacje
    set_initial_level();
    init_keyboard();
    // init_logger();
    init_periodic();
    
    while((c = getc(stdin)))
    {
        if(c == 'q')
        {
            // finalize_loggers();
            
            break;
        }
        else if (c == '\n')
        { 
            continue;
        }
        else
        {
            mq_send(keyboardMQueue, (const char *)&c, sizeof(char), 0);
            //continue; //docelowo wysyanie kolejka mq
        }
    }
    //printf("koniec");

    return EXIT_SUCCESS;
}