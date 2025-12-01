/*******************************************************************************
 *
 *  MeteoSys.c - Prototype of the MeteoSys system using pthreads
 *               and the SDL-like message layer (no time-sync signals).
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
#include "phtrdsMsgLyr.h"   /* queues, msg_t, enums */

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

    /* Wait for threads (they never finish in this prototype) */
    pthread_join(customr_tid, NULL);
    pthread_join(server_tid,  NULL);
    pthread_join(station_tid, NULL);

    destroyQueues(); 
    return 0;
}

/*** SDL system processes ************************************************/

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
        /* 1) Configure station location */
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

        /* 2) Trigger a measurement at the station */
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

        /* 3) Ask server for the last stored data */
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

        /* 4) Change sampling period in the server (and station) */
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

        /* Loop continues: back to "Enter location..." */
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

        switch (state)
        {
            case IdleSe:
                switch ((TO_SERVER)InMsg.signal)
                {
                    case sData:
                        /* New measurement from station */
                        lastLocation   = InMsg.location;
                        lastMeas       = InMsg.meas;
                        lastTimeStamp  = InMsg.timeStamp;
                        currentSampleT = InMsg.sampleTime;

                        printf("\n\tServer: received data meas=%d at location=%d "
                               "(timeStamp=%d, sampleTime=%d)\n",
                               lastMeas, lastLocation,
                               lastTimeStamp, currentSampleT);
                        fflush(stdout);
                        break;

                    case sGetData:
                        if (lastLocation >= 0)
                        {
                            printf("\n\t\t\t\tRequested Data: location=%d, meas=%d, "
                                   "timeStamp=%d, sampleTime=%d\n",
                                   lastLocation, lastMeas,
                                   lastTimeStamp, currentSampleT);
                        }
                        else
                        {
                            printf("\n\tServer: no data available yet for location %d\n",
                                   InMsg.location);
                        }
                        fflush(stdout);
                        break;

                    case sSamplingIn:
                        if (InMsg.sampleTime > 0)
                            currentSampleT = InMsg.sampleTime;

                        printf("\n\tServer: new sampling period sampleTime=%d\n",
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
                        /* Ignore any other signal */
                        break;
                }
                break;

            default:
                break;
        }
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

        switch (state)
        {
            case IdleSt:
                switch ((TO_STATION)InMsg.signal)
                {
                    case sLocation:
                        /* Configure station location */
                        location   = InMsg.location;
                        timeStamp  = 0;
                        sampleTime = 1;

                        printf("\n\t\tStation: configured location=%d\n", location);
                        fflush(stdout);

                        state_next = GetLocation;
                        break;

                    default:
                        /* Ignore other signals until location is set */
                        break;
                }
                break;

            case GetLocation:
                switch ((TO_STATION)InMsg.signal)
                {
                    case sMeasurementData:
                    {
                        /* Take a new measurement */
                        timeStamp += sampleTime;

                        /* Very simple measurement model */
                        int meas = location * 10 + timeStamp;

                        OutMsg.signal     = (int)sData;
                        OutMsg.location   = location;
                        OutMsg.meas       = meas;
                        OutMsg.timeStamp  = timeStamp;
                        OutMsg.sampleTime = sampleTime;
                        sendMessage(&(queue[SERVER_Q]), OutMsg);

                        printf("\n\t\tStation: meas=%d at location=%d "
                               "(timeStamp=%d, sampleTime=%d)\n",
                               meas, location, timeStamp, sampleTime);
                        fflush(stdout);
                        break;
                    }

                    case sSamplingOut:
                        if (InMsg.sampleTime > 0)
                            sampleTime = InMsg.sampleTime;

                        printf("\n\t\tStation: updated sampling period "
                               "sampleTime=%d\n", sampleTime);
                        fflush(stdout);
                        break;

                    default:
                        break;
                }
                /* stay in operational state */
                state_next = GetLocation;
                break;

            default:
                break;
        }
    }

    /* never reached */
    return NULL;
}
