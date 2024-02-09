#ifndef DPVAESTIMATE_H
#define DPVAESTIMATE_H

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>

#include "cimu.h"
#include "cgnss.h"
#include "cfilesaver.h"
#include "shared_memory.h"

//semaphores for shared memory
#define SEM_PRODUCER_FNAME "/sem_prod-pva"
#define SEM_CONSUMER_FNAME "/sem_cons-pva"
#define FILENAME "/etc/RaceTrackBuddy/Daemons/dPVA_estimate"
#define SH_BLOCK_SIZE 256

#define SIG_START (SIGRTMIN+1)
#define SIG_STOP (SIGRTMIN+2)

typedef struct
{
    int daemonPid;
    RMC gnss;
    accel g_force;
}PVA;

class dPvaEstimate
{

public:
   bool run();
   dPvaEstimate();
   ~dPvaEstimate();
   void make_daemon();

private:
    /* pThreads Declaration */
    pthread_t tGetGnssDataID;
    pthread_t tGetImuDataID;
    pthread_t tSaveDataFrameID;
    pthread_t tSharePvaID;


    void initMutex();
    void initCondVar();
    void initSem();

    void setThread(int prio, pthread_attr_t *pthread_attr, struct sched_param *pthread_param);
    bool initThreads();

    /* pThreads functions */
    static void *tSaveDataFrame(void *arg);
    static void *tGetGnssData(void *arg);
    static void *tGetImuData(void *arg);
    static void *tSharePva(void *arg);

    /* Objects */
    cFileSaver* file;
    cGnss *gnss;
    cImu *imu;

    pid_t myPid, sid;
    static sem_t * sem_prod, *sem_cons;
    uint8_t pid[3], status[4];
};

#endif // DPVAESTIMATE_H
