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
#include "logger.h"

void *tPeriodicThread(void *);
void *plant(void *);
void *control(void *);
void *sluiceThread(void *);
void *auxiliaryTankThread(void *);

pthread_barrier_t barrier_plant, barrier_control;

int fd;
double buff[7];
int counter = 3;

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

    pthread_barrier_init(&barrier_plant, NULL, 2);
    pthread_barrier_init(&barrier_control, NULL, 3);

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
	timerSpecStruct.it_interval.tv_nsec = 500000000;

    //Run timer
    timer_settime(timerVar, 0, &timerSpecStruct, NULL);

    return 0;
}

void *tPeriodicThread(void *cookie)
{
    pthread_t faster, slower, sluice, tank;
    pthread_attr_t afaster, aslower, asluice, atank;
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(policy);
    pthread_setschedparam(pthread_self(), policy, &param);

    static double time_counter;
    int counter_modulo = counter%4;

    //wyznaczenie wejscia dla obiektu zbiornika (przeplyw przez uklad)
    calculate_input();
    //first task
    pthread_attr_init(&afaster);
    pthread_attr_setschedpolicy(&afaster, SCHED_FIFO);
    
    pthread_create(&faster, &afaster, plant, (void *)&counter_modulo);
    pthread_detach(faster);

    //sluza
    pthread_attr_init(&asluice);
    pthread_attr_setschedpolicy(&asluice, SCHED_FIFO);
    
    pthread_create(&sluice, &asluice, sluiceThread, NULL);
    pthread_detach(sluice);

    //zbiornik pomocniczy
    pthread_attr_init(&atank);
    pthread_attr_setschedpolicy(&atank, SCHED_FIFO);
    
    pthread_create(&tank, &atank, auxiliaryTankThread, NULL);
    pthread_detach(tank);

    if(!counter_modulo) //co 4
    {
        //second task;
        pthread_attr_init(&aslower);
        pthread_attr_setschedpolicy(&aslower, SCHED_FIFO);
        pthread_create(&slower, &aslower, control, NULL);
        pthread_detach(slower);
        counter = 0;
        pthread_barrier_wait(&barrier_control); //synchronizacja, aby przeslac spojne dane
    }
    else
    {
        pthread_barrier_wait(&barrier_plant);
    }

    //Wysylanie danych 
    pthread_mutex_lock(&locks_angles);
    mq_send(loggerLock1MQueue, (const char *)&lock1_angle, sizeof(double), 0);
    mq_send(loggerLock2MQueue, (const char *)&lock2_angle, sizeof(double), 0);
    buff[2] = lock1_angle;
    buff[3] = lock2_angle;
    pthread_mutex_unlock(&locks_angles);
    pthread_mutex_lock(&input_plant_mutex);
    mq_send(loggerInputMQueue, (const char *)&plant_input, sizeof(double), 0);
    buff[0] = plant_input;
    pthread_mutex_unlock(&input_plant_mutex);
    pthread_mutex_lock(&output_plant_mutex);
    mq_send(loggerOutputMQueue, (const char *)&plant_output, sizeof(double), 0);
    buff[1] = plant_output;
    pthread_mutex_unlock(&output_plant_mutex);
    buff[4] = time_counter;
    pthread_mutex_lock(&sluice_door_mutex);
    buff[5] = (double)sluice_door_opened;
    pthread_mutex_unlock(&sluice_door_mutex);
    pthread_mutex_lock(&auxiliary_tank_used_mutex);
    buff[6] = (double)was_tank_used;
    pthread_mutex_unlock(&auxiliary_tank_used_mutex);

    if ((fd = open("my_fifo", O_WRONLY)) == -1) {
            fprintf(stderr, "Cannot open FIFO.\n" ); 
            return 0; 
    }
    write(fd,buff,sizeof(buff));
    close(fd);
    
    counter++;
    time_counter += 0.5;
    for (int i = 0; i<7; i++)
    {
        buff[i] = 0.0;
    }
    return 0;
}

void *plant(void *cookie)
{
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(policy)-1;
    pthread_setschedparam(pthread_self(), policy, &param);

    plant_step();

    if(!(*((int*)cookie)))
    {
        pthread_barrier_wait(&barrier_control);
    }
    else
    {
        pthread_barrier_wait(&barrier_plant);
    }
}

void *control(void *cookie)
{
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(policy)-2;
    pthread_setschedparam(pthread_self(), policy, &param);

    calculate_control();
}

void *sluiceThread(void *cookie)
{
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(policy)-3;
    pthread_setschedparam(pthread_self(), policy, &param);

    sluice_lock();
    sluice();
}

void *auxiliaryTankThread(void *cookie)
{
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(policy)-4;
    pthread_setschedparam(pthread_self(), policy, &param);

    auxiliaryTank();
}