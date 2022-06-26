#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <mqueue.h>
#include <unistd.h>

#include "periodic.h"
#include "calculations.h"

void *tPeriodicThread(void *);
void *plant(void *);
void *control(void *);

sig_atomic_t overrun = 0, overrun1 = 0, overrun2 = 0;
int counter = 4;
sig_atomic_t flaga = 0, flaga2=0; //usunac potem

int init_periodic()
{
    int status, ret;

    pthread_attr_t periodicThreadttr;
    struct itimerspec timerSpecStruct;
    timer_t timerVar;
    struct sigevent timerEvent;

    //Initialize threads
    pthread_attr_init(&periodicThreadttr);
    ret = pthread_attr_setschedpolicy(&periodicThreadttr, SCHED_FIFO);

    //Initialize event
    timerEvent.sigev_notify = SIGEV_THREAD;
    timerEvent.sigev_notify_function = tPeriodicThread;
    timerEvent.sigev_notify_attributes = &periodicThreadttr;

    //Create timer 
    if((status = timer_create(CLOCK_REALTIME, &timerEvent, &timerVar)))
    {
        fprintf(stderr, "Creating timer failed: %d\n", status);
        return 0;
    }

    //Set timer parameters
	timerSpecStruct.it_value.tv_sec = 1;
	timerSpecStruct.it_value.tv_nsec = 0;
	timerSpecStruct.it_interval.tv_sec = 0;
    //timerSpecStruct.it_interval.tv_nsec = 0;
	timerSpecStruct.it_interval.tv_nsec = 500000000;

    //Run timer
    timer_settime(timerVar, 0, &timerSpecStruct, NULL);

    return 0;
}

void *tPeriodicThread(void *cookie)
{
    //potem usunac
    if(overrun)
    {
        printf("Overrun\n");
        fflush(stdout);
    }
    overrun = 1;
    //------------
    pthread_t faster, slower;
    pthread_attr_t afaster, aslower;
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(policy);
    pthread_setschedparam(pthread_self(), policy, &param);

    //static int counter = 0;

    //first task
    pthread_attr_init(&afaster);
    pthread_attr_setschedpolicy(&afaster, SCHED_FIFO);
    
    pthread_create(&faster, &afaster, plant, NULL);
    pthread_detach(faster);

    

    if(!(counter%4)) //co 4
    {
        //second task;
        pthread_attr_init(&aslower);
        pthread_attr_setschedpolicy(&aslower, SCHED_FIFO);
        pthread_create(&slower, &aslower, control, NULL);
        pthread_detach(slower);
        counter = 0;
    }
    counter++;
    //potem usunac
    overrun = 0;
    //----------
    return 0;
}

void *plant(void *cookie)
{
    //potem usunac
    if(overrun1)
    {
        printf("Overrun1\n");
        fflush(stdout);
    }
    overrun1 = 1;
    flaga++;
    //----------
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(policy)-1;
    pthread_setschedparam(pthread_self(), policy, &param);

    static double counter2;
    plant_step();
    pthread_mutex_lock(&output_plant_mutex);
    printf("Poziom %f, krok %f\n", plant_output, counter2);
    pthread_mutex_unlock(&output_plant_mutex);
    fflush(stdout);
    //potem usunac
    counter2+=0.5;
    flaga2 ++;
    overrun1 = 0;
    //---------
}

void *control(void *cookie)
{
    //potem usunac
    if(overrun2)
    {
        printf("Overrun2\n");
        fflush(stdout);
    }
    overrun2 = 2;
    //------------
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(policy)-2;
    pthread_setschedparam(pthread_self(), policy, &param);

    calculate_control();

    printf("I'm control, flag %d, flag2 %d\n", flaga, flaga2);
    pthread_mutex_lock(&input_plant_mutex);
    printf("Sterowanie %f\n", plant_input);
    pthread_mutex_unlock(&input_plant_mutex);
    fflush(stdout);
    //potem usunac
    flaga = 0;
    flaga2 = 0;
    overrun2 = 0;
    //----------
}