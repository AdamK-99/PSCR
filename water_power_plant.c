 #include <stdio.h>
 #include <stdlib.h>
#include <mqueue.h>

#include "periodic.h"
#include "calculations.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    char c;

    //inicjalizacje
    init_logger();
    init_periodic();
    

    while((c = getc(stdin)))
    {
        if(c == 'q')
        {
            finalize_loggers();
            break;
        }
        else if (c == '\n')
        { 
            continue;
        }
        else
        {
            continue; //docelowo wysyanie kolejka mq
        }
    }


    return EXIT_SUCCESS;
}
