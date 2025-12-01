/*******************************************************************************
 *
 *  phtrdsMsgLyr.c - Service routines for concurrent access to a circular
 *                   buffer modelling a message queue (MeteoSys).
 *
 ******************************************************************************/

#include "phtrdsMsgLyr.h"

/* Global array of message queues (one per SDL process) */
msgq_t queue[NUM_QUEUES];

/* Put message msg into queue (circular buffer) pointed to by queue_ptr */
static void PutMsg(msgq_t *queue_ptr, msg_t msg)
{
    pthread_mutex_lock(&queue_ptr->buffer_lock);

    queue_ptr->buffer[queue_ptr->bufin] = msg;
    queue_ptr->bufin = (queue_ptr->bufin + 1) % BUFSIZE;

    pthread_mutex_unlock(&queue_ptr->buffer_lock);
}

/* Get message from queue (circular buffer) pointed to by queue_ptr */
static msg_t GetMsg(msgq_t *queue_ptr)
{
    msg_t msg;

    pthread_mutex_lock(&queue_ptr->buffer_lock);

    msg = queue_ptr->buffer[queue_ptr->bufout];
    queue_ptr->bufout = (queue_ptr->bufout + 1) % BUFSIZE;

    pthread_mutex_unlock(&queue_ptr->buffer_lock);

    return msg;
}

/* Initialise array of message queues */
void initialiseQueues(void)
{
    int i;

    for (i = 0; i < NUM_QUEUES; ++i)
    {
        queue[i].bufin  = 0;
        queue[i].bufout = 0;

        pthread_mutex_init(&queue[i].buffer_lock, NULL);

        /* Semaphores */
        sem_init(&queue[i].items, LOCAL, 0);       /* no messages yet     */
        sem_init(&queue[i].slots, LOCAL, BUFSIZE); /* BUFSIZE free slots  */
    }
}

/* Destroy array of message queues */
void destroyQueues(void)
{
    int i;

    for (i = 0; i < NUM_QUEUES; ++i)
    {
        pthread_mutex_destroy(&queue[i].buffer_lock);
        sem_destroy(&queue[i].items);
        sem_destroy(&queue[i].slots);
    }
}

/* Emulate SDL Output operation */
void sendMessage(msgq_t *queue_ptr, msg_t msg)
{
    /* Wait for an empty slot */
    sem_wait(&queue_ptr->slots);

    /* Put message in buffer */
    PutMsg(queue_ptr, msg);

    /* Signal that one more message is available */
    sem_post(&queue_ptr->items);
}

/* Emulate SDL Input operation */
msg_t receiveMessage(msgq_t *queue_ptr)
{
    msg_t msg;

    /* Wait until there is at least one message */
    sem_wait(&queue_ptr->items);

    msg = GetMsg(queue_ptr);

    /* Signal that there is one more free slot */
    sem_post(&queue_ptr->slots);

    return msg;
}
