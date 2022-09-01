#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mqueue.h>

#include "keyboard.h"
#include "calculations.h"

mqd_t keyboardMQueue;
struct mq_attr keyboardMQueueAttr;

void *userChangesThreadFunc(void *);

void set_initial_level()
{
    int x;
    double start_value;
    printf("Write initial value of water level (valid values: 0 - 6) in meters\n");
    x = scanf("%lf", &start_value);
    if(x)
    {
        if(start_value>6.0) //jezeli podano za duza wartosc 
        {
            start_value = 6.0;
        }
        else if (start_value<0.0) // jezeli podano za mala wartosc
        {
            start_value = 0.0;
        }
    }
    else
    {
        //domyslna wartosc startowa
        start_value = 5.7;
    }
    plant_H = start_value;
}

void init_keyboard()
{
    int status;
    pthread_t userChangesThread;
    pthread_attr_t userChangesThreadAttr;
    pthread_attr_init(&userChangesThreadAttr);
    pthread_attr_setschedpolicy(&userChangesThreadAttr, SCHED_FIFO);

    keyboardMQueueAttr.mq_maxmsg = 10;
    keyboardMQueueAttr.mq_msgsize = sizeof(char);

    if((keyboardMQueue = mq_open("/keyboardMQ", O_CREAT | O_RDWR, 0644, &keyboardMQueueAttr)) == -1)
    {
        fprintf(stderr, "Cannot create KeyboardMQueue\n");
    }

    if((status = pthread_create(&userChangesThread, &userChangesThreadAttr, userChangesThreadFunc, NULL)))
    {
        fprintf(stderr, "Cannot create keyboard thread\n");
    }
}
void *userChangesThreadFunc(void *cookie)
{
    char c;
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(policy);
    pthread_setschedparam(pthread_self(), policy, &param);
    for(;;)
    {
        mq_receive(keyboardMQueue, (char *)&c, sizeof(char), NULL);
        if (c == '+') //zwiekszenie przeplywu rzeki o 5l/s
        {
            pthread_mutex_lock(&input_plant_mutex);
            river_flowrate += 5;
            pthread_mutex_unlock(&input_plant_mutex);
        }
        else if (c == '-') //zmniejszenie przeplywu rzeki o 5l/s
        {
            pthread_mutex_lock(&input_plant_mutex);
            if(river_flowrate >=5)
            {
                river_flowrate -= 5;
            }
            pthread_mutex_unlock(&input_plant_mutex);
        }
        else if (c == 'a')//przejscie na automatyczne sterowanie kierownicami
        {
            pthread_mutex_lock(&mode_mutex);
            mode = 0;
            pthread_mutex_unlock(&mode_mutex);
        }
        else if (c == 'c')//zamkniecie kierownic
        {
            pthread_mutex_lock(&mode_mutex);
            mode = 1;
            pthread_mutex_unlock(&mode_mutex);
        }
        else if(c == '1') //sterowanie kierownica nr 1
        {
            pthread_mutex_lock(&mode_mutex);
            mode = 2;
            pthread_mutex_unlock(&mode_mutex);
        }
        else if(c == '2')//sterowanie kierownica nr 2
        {
            pthread_mutex_lock(&mode_mutex);
            mode = 3;
            pthread_mutex_unlock(&mode_mutex);
        }
        else if(c == '3') // sterowanie obiema kierownicami
        {
            pthread_mutex_lock(&mode_mutex);
            mode = 4;
            pthread_mutex_unlock(&mode_mutex);
        }
        else if(c == 'p') //dodanie 5 stopni otwarcia kierownicy
        {
            int mode_local;
            pthread_mutex_lock(&mode_mutex);
            mode_local = mode;
            pthread_mutex_unlock(&mode_mutex);
            if(mode_local == 2)
            {
                pthread_mutex_lock(&locks_set);
                if(!(lock1_set<85))
                {
                    lock1_set -= 5;
                }
                pthread_mutex_unlock(&locks_set);
            }
            else if(mode_local == 3)
            {
                pthread_mutex_lock(&locks_set);
                if(!(lock2_set>85))
                {
                    lock2_set += 5;
                }
                pthread_mutex_unlock(&locks_set);
            }
            else if (mode_local == 4)
            {
                pthread_mutex_lock(&locks_set);
                if(!(lock1_set>85))
                {
                    lock1_set += 5;
                }
                if(!(lock2_set>85))
                {
                    lock2_set += 5;
                }
                pthread_mutex_unlock(&locks_set);
            }
        }
        else if(c == 'm') //odjecie 5 stopni otwarcia kierownicy
        {
            int mode_local;
            pthread_mutex_lock(&mode_mutex);
            mode_local = mode;
            pthread_mutex_unlock(&mode_mutex);
            if(mode_local == 2)
            {
                pthread_mutex_lock(&locks_set);
                if(!(lock1_set<5))
                {
                    lock1_set -= 5;
                }
                pthread_mutex_unlock(&locks_set);
            }
            else if(mode_local == 3)
            {
                pthread_mutex_lock(&locks_set);
                if(!(lock2_set<5))
                {
                    lock2_set -= 5;
                }
                pthread_mutex_unlock(&locks_set);
            }
            else if (mode_local == 4)
            {
                pthread_mutex_lock(&locks_set);
                if(!(lock1_set<5))
                {
                    lock1_set -= 5;
                }
                if(!(lock2_set<5))
                {
                    lock2_set -= 5;
                }
                pthread_mutex_unlock(&locks_set);
            }
        }
        else if (c == 's') //sluza; z gory, from up
        {
            pthread_mutex_lock(&mode_mutex);
            mode = 5;
            pthread_mutex_unlock(&mode_mutex);
        }
        else if (c == 'd') //zamkniecie drzwi sluzy
        {
            pthread_mutex_lock(&sluice_door_mutex);
            if (sluice_door_opened == 1)
            {
                pthread_mutex_lock(&sluice_signal_to_close_door_mutex);
                sluice_signal_to_close_door = 1;
                pthread_mutex_unlock(&sluice_signal_to_close_door_mutex);
            }
            pthread_mutex_unlock(&sluice_door_mutex);
        }
    }
}

void finalize_keyboard()
{
    mq_close(keyboardMQueue);
}