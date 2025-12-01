/* phtrdsMsgLyr.h - Pthreads message layer for MeteoSys */

#ifndef PHTRDSMSGLYR_H
#define PHTRDSMSGLYR_H

#include <pthread.h>
#include <semaphore.h>
#include "pMLusrConf.h"      /* BUFSIZE, NUM_QUEUES, msg_t, etc. */

/* Local semaphores */
#define LOCAL 0

/* Message queue (circular buffer) */
typedef struct
{
    msg_t           buffer[BUFSIZE];
    int             bufin;
    int             bufout;
    pthread_mutex_t buffer_lock;
    sem_t           items;
    sem_t           slots;
} msgq_t;

/* Global array of queues (indexes: SERVER_Q, STATION_Q, ...) */
extern msgq_t queue[NUM_QUEUES];

/* Prototypes */
void  initialiseQueues(void);
void  destroyQueues(void);
void  sendMessage(msgq_t *queue_ptr, msg_t msg);
msg_t receiveMessage(msgq_t *queue_ptr);

#endif /* PHTRDSMSGLYR_H */
