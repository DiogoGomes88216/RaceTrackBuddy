#ifndef CMAINPROCESS_H
#define CMAINPROCESS_H

#include <unistd.h>
#include <QString>
#include <QObject>
#include <QQmlContext>

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "csvFile.h"
#include "shared_memory.h"
#include "claptimer.h"
#include "ccam.h"
#include "/home/diogo/Qt-Projects/dPVA_estimate/dpvaestimate.h"

//semaphores for shared memory with dPVA_estimate daemon
#define SEM_PRODUCER_FNAME "/sem_prod-pva"
#define SEM_CONSUMER_FNAME "/sem_cons-pva"
#define FILENAME "/etc/RaceTrackBuddy/Daemons/dPVA_estimate"
#define SH_BLOCK_SIZE 256
//semaphores for shared memory with Button daemon
#define SEM_MP_PRODUCER_FNAME "/sem_prod_MP"
#define FILENAME_MP "/etc/RaceTrackBuddy/RaceTrackBuddy"

#define SIG_START (SIGRTMIN+1)
#define SIG_STOP (SIGRTMIN+2)
using namespace std;

class cMainProcess : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentTime READ currentTime WRITE currentTime NOTIFY currentTimeChanged)
    Q_PROPERTY(bool    gpsFix      READ gpsFix      WRITE gpsFix      NOTIFY gpsFixChanged)
    Q_PROPERTY(QString currTrack   READ currTrack   WRITE currTrack   NOTIFY currTrackChanged)
    Q_PROPERTY(QString lapDelta    READ lapDelta    WRITE lapDelta    NOTIFY lapDeltaChanged)
    Q_PROPERTY(QString lastLap     READ lastLap     WRITE lastLap     NOTIFY lastLapChanged)
    Q_PROPERTY(QString bestLap     READ bestLap     WRITE bestLap     NOTIFY bestLapChanged)
    Q_PROPERTY(QString totalTime   READ totalTime   WRITE totalTime   NOTIFY totalTimeChanged)
    Q_PROPERTY(QString lapNumber   READ lapNumber   WRITE lapNumber   NOTIFY lapNumberChanged)
    Q_PROPERTY(bool    demoMode    READ demoMode    WRITE demoMode    NOTIFY demoModeChanged)

public slots:
    void addToReviewList(QObject *rootObject);
    void addtoLapsList(QObject *rootObject, QString path);
    void addtoTrackList(QObject *rootObject);
    void setTrack(QString name,QString,QString,QString,QString,QString,QString,
                  QString,QString,QString,QString,QString,QString);
    void startSession();
    void stopSession();
    void setDemoMode(bool newDemoMode);
public:
    cMainProcess(QObject * parent = Q_NULLPTR);
    ~cMainProcess();

    const QString &currentTime() const;
    void currentTime(const QString &newCurrentTime);

    bool gpsFix() const;
    void gpsFix(bool newGpsFix);

    const QString &lapDelta() const;
    void lapDelta(const QString &newLapDelta);

    const QString &lastLap() const;
    void lastLap(const QString &newLastLap);

    const QString &bestLap() const;
    void bestLap(const QString &newBestIMINGLap);

    const QString &totalTime() const;
    void totalTime(const QString &newTotalTime);

    const QString &lapNumber() const;
    void lapNumber(const QString &newLapNumber);

    const QString &currTrack() const;
    void currTrack(const QString &newCurrTrack);

    bool demoMode() const;
    void demoMode(bool newDemoMode);

signals:
    void currentTimeChanged();
    void gpsFixChanged();
    void lapDeltaChanged();
    void lastLapChanged();
    void bestLapChanged();
    void totalTimeChanged();
    void lapNumberChanged();

    void currTrackChanged();

    void demoModeChanged();

private:
    QString m_currentTime;
    QString m_lapDelta;
    QString m_lastLap;
    QString m_bestLap;
    QString m_totalTime;
    QString m_lapNumber;
    QString m_currTrack;
    bool m_gpsFix;

    /* pThreads Declaration*/
    pthread_t tGetPvaID;
    pthread_t tTimingCalculationID;
    pthread_t tAcqMergeFrameID;

    void initMutex();
    void initCondVar();
    void initSem();
    void UpdateScreen(int delta,int LapNumber, int lastLapTime, int bestLapTime);

    void setThread(int prio, pthread_attr_t *pthread_attr, struct sched_param *pthread_param);
    bool initThreads();

    /*      Objects        */
    cLapTimer *lapTimer;
    CCam *cam;
    CVideo *video;
    /* pThreads functions */
    static void *tGetPva(void *arg);
    static void *tTimingCalculation(void *arg);
    static void *tAcqMergeFrame(void *arg);

    /* Shared Memory Semaphores*/
    static sem_t * sem_prod, *sem_cons;
    /*      Mutexes      */
    pthread_mutex_t mutTimingCalc;
    pthread_mutex_t mutStartCalc;
    pthread_mutex_t mutStartVideo;

    pthread_mutex_t mutTimingScreen;
    pthread_mutex_t mutPositionTime;
    pthread_mutex_t mutSpeedG_Force;
    /* Condition Variables */
    pthread_cond_t condTimingCalc;
    pthread_cond_t condStartCalc;
    pthread_cond_t condStartVideo;
    /*      Flags        */
    bool endTiming;
    bool endVideo;

    /* VALUES FROM PVA DAEMON */
    int dPvaEstimatePID;
    coordinates currPos;
    int currTimeMS;
    int Speed;
    QString Date;
    accel G_force;

    void givePID();
    bool m_demoMode;
};

#endif // CMAINPROCESS_H
