/*******************************************************************************
 *
 *  MeteoSysTrc.c - Traced prototype of the MeteoSys system using pthreads
 *                  and the SDL-like message layer (no time-sync signals).
 *
 *  Processes:
 *      - pCustomer  : environment (RTDS_Env)
 *      - pServer    : server (pServer)
 *      - pStation   : station (pStation)
 *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "phtrdsMsgLyr.h"

/*** Function prototypes *************************************************/

static void *pCustomer (void *arg);
static void *pServer   (void *arg);
static void *pStation  (void *arg);

/*** SDL system creation *************************************************/

int main(void)
{
    pthread_t customr_tid;   /* environment thread */
    pthread_t server_tid;    /* server thread      */
    pthread_t station_tid;   /* station thread     */

    /* Create queues */
    initialiseQueues();

    /* Create SDL processes (threads) */
    pthread_create(&customr_tid, NULL, pCustomer, NULL);
    pthread_create(&server_tid,   NULL, pServer,   NULL);
    pthread_create(&station_tid,  NULL, pStation,  NULL);

    /* Wait for threads (they never finish) */
    pthread_join(customr_tid, NULL);
    pthread_join(server_tid,  NULL);
    pthread_join(station_tid, NULL);

    destroyQueues();
    return 0;
}

/*** SDL processes with tracing *****************************************/

/* Environment (RTDS_Env) */
static void *pCustomer(void *arg)
{
    char  line[100];
    int   location;
    char  answer;
    int   samplingIn;
    msg_t OutMsg;

    for (;;)
    {
        /* Phase 1: configure station location */
        printf("\nEnter location for the station (an integer value): ");
        fflush(stdout);
        fflush(stdin);
        if (fgets(line, sizeof(line), stdin) == NULL)
            continue;
        sscanf(line, "%d", &location);

        OutMsg.signal     = (int)sLocation;
        OutMsg.location   = location;
        OutMsg.meas       = 0;
        OutMsg.timeStamp  = 0;
        OutMsg.sampleTime = 0;
        sendMessage(&(queue[STATION_Q]), OutMsg);

        /* Phase 2: trigger measurement */
        do {
            printf("\nMeasure data now? (Y)es: ");
            fflush(stdout);
            fflush(stdin);
            if (fgets(line, sizeof(line), stdin) == NULL)
                continue;
            sscanf(line, "%c", &answer);
        } while (answer != 'Y');

        OutMsg.signal     = (int)sMeasurementData;
        OutMsg.location   = location;
        OutMsg.meas       = 0;
        OutMsg.timeStamp  = 0;
        OutMsg.sampleTime = 0;
        sendMessage(&(queue[STATION_Q]), OutMsg);

        /* Phase 3: request last data from server */
        do {
            printf("\nRequest data from server? (Y)es: ");
            fflush(stdout);
            fflush(stdin);
            if (fgets(line, sizeof(line), stdin) == NULL)
                continue;
            sscanf(line, "%c", &answer);
        } while (answer != 'Y');

        OutMsg.signal     = (int)sGetData;
        OutMsg.location   = location;
        OutMsg.meas       = 0;
        OutMsg.timeStamp  = 0;
        OutMsg.sampleTime = 0;
        sendMessage(&(queue[SERVER_Q]), OutMsg);

        /* Phase 4: change sampling period */
        printf("\nRequest sampling period (positive integer): ");
        fflush(stdout);
        fflush(stdin);
        if (fgets(line, sizeof(line), stdin) == NULL)
            continue;
        sscanf(line, "%d", &samplingIn);
        if (samplingIn <= 0)
            samplingIn = 1;

        OutMsg.signal     = (int)sSamplingIn;
        OutMsg.location   = location;
        OutMsg.meas       = 0;
        OutMsg.timeStamp  = 0;
        OutMsg.sampleTime = samplingIn;
        sendMessage(&(queue[SERVER_Q]), OutMsg);

        /* loop */
    }

    return NULL;
}

/* Server (pServer) */
static void *pServer(void *arg)
{
    SERVER_STATES state, state_next;
    msg_t         InMsg;
    msg_t         OutMsg;

    int lastLocation   = -1;
    int lastMeas       = 0;
    int lastTimeStamp  = 0;
    int currentSampleT = 1;

    state      = IdleSe;
    state_next = IdleSe;

    for (;;)
    {
        state = state_next;
        InMsg = receiveMessage(&(queue[SERVER_Q]));

        /* Trace: which message was received */
        printf("]\n\tServer received signal %d in state %d "
               "(loc=%d, meas=%d, ts=%d, Ts=%d)",
               InMsg.signal, state,
               InMsg.location, InMsg.meas,
               InMsg.timeStamp, InMsg.sampleTime);
        fflush(stdout);

        switch (state)
        {
            case IdleSe:
                switch ((TO_SERVER)InMsg.signal)
                {
                    case sData:
                        lastLocation   = InMsg.location;
                        lastMeas       = InMsg.meas;
                        lastTimeStamp  = InMsg.timeStamp;
                        currentSampleT = InMsg.sampleTime;

                        printf("\n\tServer: stored data loc=%d, meas=%d, "
                               "timeStamp=%d, sampleTime=%d",
                               lastLocation, lastMeas,
                               lastTimeStamp, currentSampleT);
                        fflush(stdout);
                        break;

                    case sGetData:
                        if (lastLocation >= 0)
                        {
                            printf("\n\tServer->Customer (sSendData): "
                                   "loc=%d, meas=%d, timeStamp=%d, sampleTime=%d",
                                   lastLocation, lastMeas,
                                   lastTimeStamp, currentSampleT);
                        }
                        else
                        {
                            printf("\n\tServer: no data available yet for location %d",
                                   InMsg.location);
                        }
                        fflush(stdout);
                        break;

                    case sSamplingIn:
                        if (InMsg.sampleTime > 0)
                            currentSampleT = InMsg.sampleTime;

                        printf("\n\tServer: new sampling period sampleTime=%d",
                               currentSampleT);
                        fflush(stdout);

                        OutMsg.signal     = (int)sSamplingOut;
                        OutMsg.location   = InMsg.location;
                        OutMsg.meas       = 0;
                        OutMsg.timeStamp  = 0;
                        OutMsg.sampleTime = currentSampleT;
                        sendMessage(&(queue[STATION_Q]), OutMsg);
                        break;

                    default:
                        break;
                }
                break;

            default:
                break;
        }

        /* Show next state (only IdleSe in this model) */
        printf("\n\tServer next state is %d", state_next);
        fflush(stdout);
    }

    /* never reached */
    return NULL;
}

/* Station (pStation) */
static void *pStation(void *arg)
{
    STATION_STATES state, state_next;
    msg_t          InMsg;
    msg_t          OutMsg;

    int location   = 0;
    int timeStamp  = 0;
    int sampleTime = 1;

    state      = IdleSt;
    state_next = IdleSt;

    for (;;)
    {
        state = state_next;
        InMsg = receiveMessage(&(queue[STATION_Q]));

        /* Trace similar to Hardware in CoffeeMachineTrc */
        printf("\n\t\tStation received signal %d in state %d "
               "(loc=%d, meas=%d, ts=%d, Ts=%d)\n",
               InMsg.signal, state,
               InMsg.location, InMsg.meas,
               InMsg.timeStamp, InMsg.sampleTime);
        fflush(stdout);

        switch (state)
        {
            case IdleSt:
                switch ((TO_STATION)InMsg.signal)
                {
                    case sLocation:
                        location   = InMsg.location;
                        timeStamp  = 0;
                        sampleTime = 1;
                        printf("\n\t\tStation: configured location=%d", location);
                        fflush(stdout);

                        state_next = GetLocation;
                        break;

                    default:
                        break;
                }
                break;

            case GetLocation:
                switch ((TO_STATION)InMsg.signal)
                {
                    case sMeasurementData:
                    {
                        timeStamp += sampleTime;

                        int meas = location * 10 + timeStamp;

                        OutMsg.signal     = (int)sData;
                        OutMsg.location   = location;
                        OutMsg.meas       = meas;
                        OutMsg.timeStamp  = timeStamp;
                        OutMsg.sampleTime = sampleTime;
                        sendMessage(&(queue[SERVER_Q]), OutMsg);

                        printf("\n\t\tStation: meas=%d at location=%d "
                               "(timeStamp=%d, sampleTime=%d)",
                               meas, location, timeStamp, sampleTime);
                        fflush(stdout);
                        break;
                    }

                    case sSamplingOut:
                        if (InMsg.sampleTime > 0)
                            sampleTime = InMsg.sampleTime;

                        printf("\n\t\tStation: updated sampling period "
                               "sampleTime=%d",
                               sampleTime);
                        fflush(stdout);
                        break;

                    default:
                        break;
                }
                state_next = GetLocation;
                break;

            default:
                break;
        }
    }

    /* never reached */
    return NULL;
}
