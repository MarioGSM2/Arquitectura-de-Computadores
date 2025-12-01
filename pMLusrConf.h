/*******************************************************************************
 *
 *  pMLusrConf.h - Pthreads Message Layer user configuration file
 *                 for the MeteoSys example.
 *
 *  Notes: User defined according to application.
 *
 ******************************************************************************/

#ifndef PMLUSRCONF_H
#define PMLUSRCONF_H

/*** Manifest constants for user-defined queuing system ******************/

#define BUFSIZE       8    /* number of slots in queues          */
#define NUM_QUEUES    2    /* number of queues                   */
#define SERVER_Q      0    /* queue 0: pServer                   */
#define STATION_Q     1    /* queue 1: pStation                  */

/*** User-defined message structure **************************************/

typedef struct
{
    int signal;       /* signal identifier                          */
    int location;     /* station location (or generic integer arg)  */
    int meas;         /* measurement value (or generic integer arg) */
    int timeStamp;    /* logical/physical time stamp                */
    int sampleTime;   /* sampling period                            */
} msg_t;

/*** User-defined signals ************************************************/

/* Signals sent to the “customer” (RTDS_Env) – not used as SDL queue here */
typedef enum
{
    sSendData              /* pServer -> RTDS_Env (informational only) */
} TO_CUSTOMER;

/* Signals whose destination is pServer (from RTDS_Env or pStation) */
typedef enum
{
    sData,                 /* pStation -> pServer                      */
    sGetData,              /* RTDS_Env -> pServer                      */
    sSamplingIn            /* RTDS_Env -> pServer                      */
} TO_SERVER;

/* Signals whose destination is pStation (from RTDS_Env or pServer) */
typedef enum
{
    sLocation,             /* RTDS_Env -> pStation                     */
    sMeasurementData,      /* RTDS_Env -> pStation (trigger measure)   */
    sSamplingOut           /* pServer -> pStation                      */
} TO_STATION;

/*** User-defined EFSM states ********************************************/

/* EFSM states for pServer */
typedef enum
{
    IdleSe                 /* single state for server in this model   */
} SERVER_STATES;

/* EFSM states for pStation */
typedef enum
{
    IdleSt,                /* waiting for location                     */
    GetLocation            /* location configured, normal operation    */
} STATION_STATES;

#endif /* PMLUSRCONF_H */
