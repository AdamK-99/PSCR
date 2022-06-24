#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <mqueue.h>

#include "periodic.h"

void *tPeriodicThread(void *);
void plant();
void control();
//int counter;
int init_periodic()
{
    int status;

    pthread_attr_t periodicThreadttr;
    struct itimerspec timerSpecStruct;
    timer_t timerVar;
    struct sigevent timerEvent;

    //Initialize threads
    pthread_attr_init(&periodicThreadttr);
    pthread_attr_setschedpolicy(&periodicThreadttr, SCHED_FIFO);

    //Initialize event
    timerEvent.sigev_notify = SIGEV_THREAD;
    timerEvent.sigev_notify_function = tPeriodicThread;
    timerEvent.sigev_notify_attributes = &periodicThreadttr;

    //Create timer 
    if(status = timer_create(CLOCK_REALTIME, &timerEvent, &timerVar))
    {
        fprintf(stderr, "Creating timer failed: %d\n", status);
        return 0;
    }

    //Set timer parameters
	timerSpecStruct.it_value.tv_sec = 1;
	timerSpecStruct.it_value.tv_nsec = 0;
	timerSpecStruct.it_interval.tv_sec = 0;
	timerSpecStruct.it_interval.tv_nsec = 500000000;

    //Run timer
    timer_settime(timerVar, 0, &timerSpecStruct, NULL);

    return 0;
}

void *tPeriodicThread(void *cookie)
{
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(policy);
    pthread_setschedparam(pthread_self(), policy, &param);

    static int counter;

    //first task
    plant();

    counter++;

    if(!(counter%4)) //co 4
    {
        control();
        counter = 0;
    }
    return 0;
}

void plant()
{
    printf("I'm a plant\n");
    fflush(stdout);
}

void control()
{
    printf("I'm control\n");
    fflush(stdout);
}