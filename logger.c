#include <stdio.h>
#include <pthread.h>
#include <mqueue.h>
#include <stdlib.h>

#include "logger.h"

void *loggerOutputThreadFunc(void*);
void *loggerInputThreadFunc(void*);
void *loggerLock1ThreadFunc(void*);
void *loggerLock2ThreadFunc(void*);

mqd_t loggerOutputMQueue;
mqd_t loggerInputMQueue;
mqd_t loggerLock1MQueue;
mqd_t loggerLock2MQueue;

struct mq_attr loggerOutputMQueueAttr;
struct mq_attr loggerInputMQueueAttr;
struct mq_attr loggerLock1MQueueAttr;
struct mq_attr loggerLock2MQueueAttr;

void init_logger()
{
    pthread_t loggerOutputThread, loggerInputThread, loggerLock1Thread, loggerLock2Thread;
    pthread_attr_t loggerThreadAttr;
    int status;
    pthread_attr_init(&loggerThreadAttr);
    pthread_attr_setschedpolicy(&loggerThreadAttr, SCHED_FIFO);

    loggerOutputMQueueAttr.mq_maxmsg = 10;
    loggerInputMQueueAttr.mq_maxmsg = 10;
    loggerLock1MQueueAttr.mq_maxmsg = 10;
    loggerLock2MQueueAttr.mq_maxmsg = 10;

    loggerOutputMQueueAttr.mq_msgsize = sizeof(double);
    loggerInputMQueueAttr.mq_msgsize = sizeof(double);
    loggerLock1MQueueAttr.mq_msgsize = sizeof(double);
    loggerLock2MQueueAttr.mq_msgsize = sizeof(double);

    if((loggerOutputMQueue = mq_open("/outputMQ", O_CREAT | O_RDWR, 0644, &loggerOutputMQueueAttr)) == -1)
    {
        fprintf(stderr, "Cannot create OutputMQueue\n");
    }
    if((loggerInputMQueue = mq_open("/inputMQ", O_CREAT | O_RDWR, 0644, &loggerInputMQueueAttr)) == -1)
    {
        fprintf(stderr, "Cannot create InputMQueue\n");
    }
    if((loggerLock1MQueue = mq_open("/lock1MQ", O_CREAT | O_RDWR, 0644, &loggerLock1MQueueAttr)) == -1)
    {
        fprintf(stderr, "Cannot create Lock1MQueue\n");
    }
    if((loggerLock2MQueue = mq_open("/lock2MQ", O_CREAT | O_RDWR, 0644, &loggerLock2MQueueAttr)) == -1)
    {
        fprintf(stderr, "Cannot create Lock2MQueue\n");
    }

    if(status == pthread_create(&loggerOutputThread, NULL, loggerOutputThreadFunc, &loggerThreadAttr))
    {
        fprintf(stderr,"Cannot create LoggerOutput thread\n");
    }
    if(status == pthread_create(&loggerInputThread, NULL, loggerInputThreadFunc, &loggerThreadAttr))
    {
        fprintf(stderr,"Cannot create LoggerInput thread\n");
    }
    if(status == pthread_create(&loggerLock1Thread, NULL, loggerLock1ThreadFunc, &loggerThreadAttr))
    {
        fprintf(stderr,"Cannot create LoggerLock1 thread\n");
    }
    if(status == pthread_create(&loggerLock2Thread, NULL, loggerLock2ThreadFunc, &loggerThreadAttr))
    {
        fprintf(stderr,"Cannot create LoggerLock2 thread\n");
    }
}

void *loggerOutputThreadFunc(void *cookie)
{
    FILE *output_file;
    double output;
    
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_min(policy);
    pthread_setschedparam(pthread_self(), policy, &param);

    output_file = fopen("output_results","w");
    fprintf(output_file, "Poziom cieczy: \n");
    fclose(output_file);

    for(;;)
    {
        mq_receive(loggerOutputMQueue, (char *)&output, sizeof(double), NULL);

        output_file = fopen("output_results", "a");
        fprintf(output_file, "%f\n", output);
        fclose(output_file);
    }

}
void *loggerInputThreadFunc(void *cookie)
{
    FILE *input_file;
    double input;

    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_min(policy);
    pthread_setschedparam(pthread_self(), policy, &param);

    input_file = fopen("input_results", "w");
    fprintf(input_file, "Natezenie przeplywu: \n");
    fclose(input_file);

    for(;;)
    {
        mq_receive(loggerInputMQueue, (char *)&input, sizeof(double), NULL);

        input_file = fopen("input_results", "a");
        fprintf(input_file, "%f\n", input);
        fclose(input_file);
    }

}

void *loggerLock1ThreadFunc(void *cookie)
{
    FILE *lock1_file;
    double lock1;
    
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_min(policy);
    pthread_setschedparam(pthread_self(), policy, &param);

    lock1_file = fopen("lock1_results", "w");
    fprintf(lock1_file, "Kat otwarcia kierownicy 1: \n");
    fclose(lock1_file);

    for(;;)
    {
        mq_receive(loggerLock1MQueue, (char *)&lock1, sizeof(double), NULL);

        lock1_file = fopen("lock1_results", "a");
        fprintf(lock1_file, "%d\n", (int)lock1);
        fclose(lock1_file);
    }

}

void *loggerLock2ThreadFunc(void *cookie)
{
    FILE *lock2_file;
    double lock2;
    
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_min(policy);
    pthread_setschedparam(pthread_self(), policy, &param);

    lock2_file = fopen("lock2_results", "w");
    fprintf(lock2_file, "Kat otwarcia kierownicy 2: \n");
    fclose(lock2_file);

    for(;;)
    {
        mq_receive(loggerLock2MQueue, (char *)&lock2, sizeof(double), NULL);

        lock2_file = fopen("lock2_results", "a");
        fprintf(lock2_file, "%d\n", (int)lock2);
        fclose(lock2_file);
    }

}

void finalize_loggers()
{
    mq_close(loggerOutputMQueue);
    mq_close(loggerInputMQueue);
    mq_close(loggerLock1MQueue);
    mq_close(loggerLock2MQueue);
}