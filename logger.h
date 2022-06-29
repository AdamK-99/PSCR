#ifndef LOGGER_H_
#define LOGGER_H_

extern mqd_t loggerOutputMQueue;
extern mqd_t loggerInputMQueue;
extern mqd_t loggerLock1MQueue;
extern mqd_t loggerLock2MQueue;

void init_logger();
void finalize_loggers();

#endif