 #include <stdio.h>
 #include <stdlib.h>

#include "periodic.h"

int main(int argc, char *argv[])
{
    char c;

    //inicjalizacje
    init_periodic();

    while((c = getc(stdin)))
    {
        if(c == 'q')
        {
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
