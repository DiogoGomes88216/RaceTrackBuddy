#include "cmainprocess.h"

coordinates demoCoordinates[6] = {{41.539101, -8.442087},{41.539040, -8.441764},{41.538988, -8.441460},{41.538938, -8.441180},{41.539130, -8.441131},{41.539332, -8.442032}};
int demotimeBuff[14] = {50,75,30,20,10,10,10,64,63,56,78,90,34,15};
sem_t * cMainProcess::sem_prod = nullptr;
sem_t * cMainProcess::sem_cons = nullptr;

cMainProcess::cMainProcess(QObject *parent)
    : QObject(parent)
{
    lapTimer = new cLapTimer;
    cam = new CCam;
    video = new CVideo;
    endTiming = false;
    endVideo = false;

    initSem();
    initCondVar();
    initMutex();

    currTrack("Select a Track!");
    gpsFix(false);
    currentTime("- -:- -");
    demoMode(false);

    initThreads();
}

cMainProcess::~cMainProcess()
{
    pthread_kill(tGetPvaID,SIGKILL);
    pthread_kill(tTimingCalculationID,SIGKILL);
    pthread_kill(tAcqMergeFrameID,SIGKILL);

    lapTimer->~cLapTimer();
    video->~CVideo();
    cam->~CCam();
}
void cMainProcess::initSem()
{
   //create new producer semaphore
   sem_prod = sem_open(SEM_PRODUCER_FNAME, 0);
   if(!sem_prod)
       perror("Producer Semaphore not created");

   //create new consumer semaphore
   sem_cons = sem_open(SEM_CONSUMER_FNAME, 0);
   if(!sem_cons)
       perror("Consumer Semaphore not created");
}
void cMainProcess::initCondVar()
{
    condTimingCalc = PTHREAD_COND_INITIALIZER;
    condStartCalc = PTHREAD_COND_INITIALIZER;
    condStartVideo = PTHREAD_COND_INITIALIZER;
}
void cMainProcess::initMutex()
{
    mutTimingCalc = PTHREAD_MUTEX_INITIALIZER;
    mutStartCalc = PTHREAD_MUTEX_INITIALIZER;
    mutStartVideo = PTHREAD_MUTEX_INITIALIZER;

    mutPositionTime = PTHREAD_MUTEX_INITIALIZER;
    mutSpeedG_Force = PTHREAD_MUTEX_INITIALIZER;
    mutTimingScreen = PTHREAD_MUTEX_INITIALIZER;
}
void cMainProcess::setThread(int priority, pthread_attr_t *pthread_attr, sched_param *pthread_param)
{
    pthread_attr_setschedpolicy(pthread_attr, SCHED_RR);
    pthread_param->sched_priority = priority;
    pthread_attr_setschedparam(pthread_attr, pthread_param); //The pthread_attr_setschedparam() function sets the scheduling parameter
    pthread_attr_setinheritsched(pthread_attr, PTHREAD_EXPLICIT_SCHED);
}
bool cMainProcess::initThreads()
{
    int threadPolicy;
    pthread_attr_t threadAttr;
    struct sched_param threadPara;

    givePID();

    pthread_attr_init(&threadAttr);
    pthread_attr_getschedparam(&threadAttr, &threadPara);
    pthread_attr_getschedpolicy(&threadAttr, &threadPolicy);

    setThread(4, &threadAttr, &threadPara);
    if((pthread_create(&tGetPvaID, &threadAttr, tGetPva, static_cast<void*>(this))) != 0)
    {
        perror("Failed to Create get PVA thread");
        return false;
    }
    setThread(4, &threadAttr, &threadPara);
    if((pthread_create(&tTimingCalculationID, &threadAttr, tTimingCalculation, static_cast<void*>(this))) != 0)
    {
        perror("Failed to Create get Timing calculation thread");
        return false;
    }
    setThread(5, &threadAttr, &threadPara);
    if((pthread_create(&tAcqMergeFrameID, &threadAttr, tAcqMergeFrame, static_cast<void*>(this))) != 0)
    {
        perror("Failed to Create VIDEO overlay thread");
        return false;
    }
    return true;
}
void *cMainProcess::tGetPva(void *arg)
{
    cMainProcess *main;
    main = (cMainProcess*)arg;
    PVA pva;

    PVA* block = (PVA*) attach_block(FILENAME, SH_BLOCK_SIZE);
    if(!block)
        cout<<"block is bad"<<endl;

    int demoTime = 0;
    int demopos = 0;
    int buffindex = 0;
    while(1)
    {
        sem_wait(sem_prod);
        pva = *block;
        sem_post(sem_cons);

        main->dPvaEstimatePID = pva.daemonPid;

        pthread_mutex_lock(&main->mutPositionTime);
        if(main->demoMode())
        {
            demoTime++;
            if(demoTime == demotimeBuff[buffindex])
            {

                main->currPos = demoCoordinates[demopos];
                demoTime = 0;
                demopos++;
                buffindex++;
                if(demopos == 6)
                    demopos = 0;
                if(buffindex == 14)
                    buffindex = 0;
            }
        }
        else
            main->currPos = pva.gnss.Position;

        main->currTimeMS = QTime(0,0).msecsTo(QTime::fromString(QString::number(pva.gnss.Time,'f',3),"hmmss.zzz"));
        main->Date = QString::number(pva.gnss.Date);
        pthread_mutex_unlock(&main->mutPositionTime);

        pthread_mutex_lock(&main->mutSpeedG_Force);
        main->Speed = pva.gnss.Speed;
        main->G_force = pva.g_force;
        pthread_mutex_unlock(&main->mutSpeedG_Force);

        if(pva.gnss.Status == 'A')
            main->gpsFix(true);
        else
            main->gpsFix(false);

        if(main->demoMode())
            main->gpsFix(true);

        main->currentTime(QTime(0,0).addMSecs(main->currTimeMS).toString("hh:mm:ss"));

        pthread_mutex_lock(&main->mutTimingCalc);
        pthread_cond_signal(&main->condTimingCalc);
        pthread_mutex_unlock(&main->mutTimingCalc);
    }
    //let go of shared memory block
    detach_block((char*)block);
    sem_close(sem_prod);
    sem_close(sem_cons);
}
void *cMainProcess::tTimingCalculation(void *arg)
{
    cMainProcess *main;
    main = (cMainProcess*)arg;

    string sessionDate;
    coordinates currPos, lastPos;
    Lap currLap;
    int sessionStartTime = 0, sectorStartTime = 0, currTime = 0, lastTime = 0, calcTime = 0, delta = 1;
    int SectorNumber = 0;

    while(1)
    {
        pthread_mutex_lock(&main->mutStartCalc);
        pthread_cond_wait(&main->condStartCalc, &main->mutStartCalc);
        pthread_mutex_unlock(&main->mutStartCalc);

        pthread_mutex_lock(&main->mutPositionTime);
        sessionStartTime = main->currTimeMS;
        sessionDate = main->Date.toStdString();
        pthread_mutex_unlock(&main->mutPositionTime);

        /*RESET VARIABLES */
        sectorStartTime = 0;
        SectorNumber = 0;
        main->lapTimer->LapNumber = 0;
        main->lapTimer->LapBuff.clear();
        main->lapTimer->BestLap.sectors[0] = INT_MAX;
        main->lapTimer->BestLap.sectors[1] = INT_MAX;
        main->lapTimer->BestLap.sectors[2] = INT_MAX;
        main->lapTimer->BestLap.LapTime = INT_MAX;
        main->UpdateScreen(INT_MAX, 0, INT_MAX, INT_MAX);

        while (!main->endTiming)
        {
            pthread_mutex_lock(&main->mutTimingCalc);
            pthread_cond_wait(&main->condTimingCalc, &main->mutTimingCalc);
            pthread_mutex_unlock(&main->mutTimingCalc);

            pthread_mutex_lock(&main->mutPositionTime);
            currPos = main->currPos;
            currTime = main->currTimeMS;
            pthread_mutex_unlock(&main->mutPositionTime);

            if(main->lapTimer->crossed({lastPos, currPos}, currTime - lastTime, SectorNumber, &calcTime))
            {
                if(main->lapTimer->LapNumber == 0)
                {
                    main->lapTimer->LapNumber++;
                    main->UpdateScreen(INT_MAX, main->lapTimer->LapNumber, INT_MAX, INT_MAX);
                }
                else
                {
                    if(SectorNumber == 0)
                    {
                        currLap.sectors[2] = (lastTime + calcTime) - sectorStartTime;

                        delta += currLap.sectors[2] -  main->lapTimer->BestLap.sectors[2];

                        currLap.LapTime = 0;
                        currLap.LapTime += currLap.sectors[0];
                        currLap.LapTime += currLap.sectors[1];
                        currLap.LapTime += currLap.sectors[2];


                        if(currLap.LapTime <= main->lapTimer->BestLap.LapTime)
                            main->lapTimer->BestLap = currLap;

                        main->lapTimer->LapBuff.push_back(currLap);
                        cout<<"NEW LAP"<<endl;
                        cout<<"LAP TIME: "<<currLap.LapTime<<endl;
                        cout<<"SECTOR 1: "<<currLap.sectors[0]<<endl;
                        cout<<"SECTOR 2: "<<currLap.sectors[1]<<endl;
                        cout<<"SECTOR 3: "<<currLap.sectors[2]<<endl;
                        cout<<"----------"<<endl;
                        main->lapTimer->LapNumber++;

                        if(main->lapTimer->LapNumber == 2)
                            main->UpdateScreen(INT_MAX, main->lapTimer->LapNumber, main->lapTimer->LapBuff[main->lapTimer->LapNumber-2].LapTime,main->lapTimer->BestLap.LapTime);
                        else
                            main->UpdateScreen(delta, main->lapTimer->LapNumber, main->lapTimer->LapBuff[main->lapTimer->LapNumber-2].LapTime,main->lapTimer->BestLap.LapTime);

                        delta = 0;
                    }
                    else
                    {
                        currLap.sectors[SectorNumber - 1] = (lastTime + calcTime) - sectorStartTime;
                        cout<<"sector TIME--"<<currLap.sectors[SectorNumber - 1]<<endl;
                        if(main->lapTimer->LapNumber > 1)
                        {
                            delta += currLap.sectors[SectorNumber - 1] - main->lapTimer->BestLap.sectors[SectorNumber - 1];
                            main->UpdateScreen(delta, main->lapTimer->LapNumber, main->lapTimer->LapBuff[main->lapTimer->LapNumber-2].LapTime,main->lapTimer->BestLap.LapTime);
                        }
                    }
                }

                SectorNumber ++;
                if(SectorNumber == 3)
                    SectorNumber = 0;

                sectorStartTime = lastTime + calcTime;
            }

            pthread_mutex_lock(&main->mutTimingScreen);
            main->totalTime(QTime(0,0).addMSecs(currTime - sessionStartTime).toString("hh:mm:ss"));
            pthread_mutex_unlock(&main->mutTimingScreen);

            lastTime = currTime;
            lastPos = currPos;
        }
        cout<<"ESCREVER NO FICHEIRO"<<endl;
        main->lapTimer->saveSession(sessionDate, sessionStartTime);
        main->endTiming = false;
    }
}
void *cMainProcess::tAcqMergeFrame(void *arg)
{
    cMainProcess *main;
    main = (cMainProcess*)arg;

    while(1)
    {
        pthread_mutex_lock(&main->mutStartVideo);
        pthread_cond_wait(&main->condStartVideo, &main->mutStartVideo);
        pthread_mutex_unlock(&main->mutStartVideo);

        main->cam->openCam();

        pthread_mutex_lock(&main->mutPositionTime);

        main->video->open(main->Date.toStdString(), QTime(0,0).addMSecs(main->currTimeMS).toString("hhmm").toStdString());

        pthread_mutex_unlock(&main->mutPositionTime);

        cout<<"RECORDING IS STARTING"<<endl;

        while(!main->endVideo)
        {
            main->cam->catchFrame();
            main->cam->setOverlay();
            pthread_mutex_lock(&main->mutTimingScreen);
            main->cam->setTiming(main->lapNumber().toStdString(), main->lastLap().toStdString(),
                                 main->bestLap().toStdString(), main->lapDelta().toStdString(),
                                 main->totalTime().toStdString());
            pthread_mutex_unlock(&main->mutTimingScreen);

            pthread_mutex_lock(&main->mutSpeedG_Force);
            main->cam->setVA(to_string(main->Speed), main->G_force.Ay, main->G_force.Ay);
            pthread_mutex_unlock(&main->mutSpeedG_Force);

            main->video->writeFrame(main->cam);
        }
        cout<<"RECORDING STOPED"<<endl;
        main->cam->closeCam();
        main->video->closeVideo();
        main->endVideo = false;
    }
}
void cMainProcess::startSession()
{
    pthread_mutex_lock(&mutStartCalc);
    pthread_cond_signal(&condStartCalc);
    pthread_mutex_unlock(&mutStartCalc);

    pthread_mutex_lock(&mutStartVideo);
    pthread_cond_signal(&condStartVideo);
    pthread_mutex_unlock(&mutStartVideo);

    //SIGNAL DAEMON
    kill(dPvaEstimatePID, SIG_START);
    cout<<"STARTED"<<endl;
}
void cMainProcess::stopSession()
{
    endTiming = true;
    endVideo = true;
    //SIGNAL DAEMON
    kill(dPvaEstimatePID, SIG_STOP);

    cout<<"STOPED"<<endl;
}

void cMainProcess::setDemoMode(bool newDemoMode)
{
    demoMode(newDemoMode);
}
void cMainProcess::UpdateScreen(int delta,int LapNumber, int lastLapTime, int bestLapTime)
{
    pthread_mutex_lock(&mutTimingScreen);
    lapNumber(QString::number(LapNumber));

    if(lastLapTime == INT_MAX)
        lastLap("- : - - . - - -");
    else
        lastLap(QTime(0,0).addMSecs(lastLapTime).toString("mm:ss.zzz"));
    if(bestLapTime == INT_MAX)
        bestLap("- : - - . - - -");
    else
        bestLap(QTime(0,0).addMSecs(bestLapTime).toString("mm:ss.zzz"));
    if(delta == INT_MAX)
        lapDelta("+ - - . - - -");
    else
    {
        cout<<"DELTA IS: "<<delta<<endl;
        if(delta > 0)
            lapDelta(QTime(0,0).addMSecs(delta).toString("+ss.zzz"));
        else
            lapDelta(QTime(0,0).addMSecs(-delta).toString("-ss.zzz"));
    }
    pthread_mutex_unlock(&mutTimingScreen);
}
void cMainProcess::givePID()
{
    int myPid = getpid();

    //remove any leftover semaphore (precaution)
    sem_unlink(SEM_MP_PRODUCER_FNAME);

    //create new semaphore
    sem_t * sem_producer = sem_open(SEM_MP_PRODUCER_FNAME, O_CREAT, 0660, 0);
    if(sem_producer == SEM_FAILED)
        cout<<"PID SHARE SEMAPHORE ERROR"<<endl;

    //grab the shared memory block
    char* block = attach_block(FILENAME_MP, 64);
    if(!block)
        cout<<"ATTACH BLOCK ERROR"<<endl;

    //put pid on shared memory
    sprintf(block, "%d", myPid);

    //change semaphore value to let know pid is available
    sem_post(sem_producer);

    //let go of shared memory block
    sem_close(sem_producer);
    detach_block(block);
}
const QString &cMainProcess::currentTime() const
{
    return m_currentTime;
}
void cMainProcess::currentTime(const QString &newCurrentTime)
{
    if (m_currentTime == newCurrentTime)
        return;
    m_currentTime = newCurrentTime;
    emit currentTimeChanged();
}
bool cMainProcess::gpsFix() const
{
    return m_gpsFix;
}
void cMainProcess::gpsFix(bool newGpsFix)
{
    if (m_gpsFix == newGpsFix)
        return;
    m_gpsFix = newGpsFix;
    emit gpsFixChanged();
}
const QString &cMainProcess::currTrack() const
{
    return m_currTrack;
}

void cMainProcess::currTrack(const QString &newCurrTrack)
{
    if (m_currTrack == newCurrTrack)
        return;
    m_currTrack = newCurrTrack;
    emit currTrackChanged();
}
const QString &cMainProcess::lapDelta() const
{
    return m_lapDelta;
}
void cMainProcess::lapDelta(const QString &newLapDelta)
{
    if (m_lapDelta == newLapDelta)
        return;
    m_lapDelta = newLapDelta;
    emit lapDeltaChanged();
}
const QString &cMainProcess::lastLap() const
{
    return m_lastLap;
}
void cMainProcess::lastLap(const QString &newLastLap)
{
    if (m_lastLap == newLastLap)
        return;
    m_lastLap = newLastLap;
    emit lastLapChanged();
}
const QString &cMainProcess::bestLap() const
{
    return m_bestLap;
}
void cMainProcess::bestLap(const QString &newBestLap)
{
    if (m_bestLap == newBestLap)
        return;
    m_bestLap = newBestLap;
    emit bestLapChanged();
}
const QString &cMainProcess::totalTime() const
{
    return m_totalTime;
}
void cMainProcess::totalTime(const QString &newTotalTime)
{
    if (m_totalTime == newTotalTime)
        return;
    m_totalTime = newTotalTime;
    emit totalTimeChanged();
}
const QString &cMainProcess::lapNumber() const
{
    return m_lapNumber;
}
void cMainProcess::lapNumber(const QString &newLapNumber)
{
    if (m_lapNumber == newLapNumber)
        return;
    m_lapNumber = newLapNumber;
    emit lapNumberChanged();
}
bool cMainProcess::demoMode() const
{
    return m_demoMode;
}
void cMainProcess::demoMode(bool newDemoMode)
{
    if (m_demoMode == newDemoMode)
        return;
    m_demoMode = newDemoMode;
    emit demoModeChanged();
}

void cMainProcess::addToReviewList(QObject* rootObject)
{
    vector<string> pathBuff;
    vector<string> headerInfo;
    string path = "/mnt/usb/Sessions/";

    pathBuff = listFiles(path);
    for(uint i =0; i< pathBuff.size();i++)
    {
        headerInfo = read_header(pathBuff[i]);

        QVariantMap newElement;  // QVariantMap will implicitly translates into JS-object
          newElement.insert("path",      QString::fromStdString(pathBuff[i]));
          newElement.insert("date",      QString::fromStdString(headerInfo[0]));
          newElement.insert("session",   QString::fromStdString(headerInfo[1]));
          newElement.insert("laps",      QString::fromStdString(headerInfo[2]));
          newElement.insert("fastestLap",QString::fromStdString(headerInfo[3]));


          QMetaObject::invokeMethod(
             rootObject,                         // for this object we will call method
              "appendReview",                                 // actually, name of the method to call
              Q_ARG(QVariant, QVariant::fromValue(newElement))  // method parameter
          );
    }
}
void cMainProcess::addtoLapsList(QObject *rootObject, QString path)
{
    string color;
    string fstLap;
    vector<string>laps;
    vector<string>elementAttr = {"lap","lapTime","s1","s2","s3","gap"};
    fstLap = read_header(path.toStdString())[3];
    laps = read_laps(path.toStdString());
    for(uint i = 0; i < laps.size() / 6; i++)
    {
        QVariantMap newElement;
        for(uint j=0; j < 6 ; j++)
        {
            newElement.insert(QString::fromStdString(elementAttr[j]), QString::fromStdString(laps[(i*6)+j]));
            if((elementAttr[j] == "lap") && (laps[(i*6)+j] == "Avg"))
                color = "blue";
            else if ((elementAttr[j] == "lap") && (laps[(i*6)+j] == "Opt"))
                color = "purple";
            else if((elementAttr[j] == "lapTime") && (laps[(i*6)+j] == fstLap))
                color = "green";
            else if(elementAttr[j] == "lapTime" && ((laps[((i*6)+j)-1] != "Avg") && (laps[((i*6)+j)-1] != "Opt")))
                color = "gray";
        }
        newElement.insert("setColor", QString::fromStdString(color));
        newElement.insert("path", path);
        QMetaObject::invokeMethod(
           rootObject,                         // for this object we will call method
            "appendLaps",                                 // actually, name of the method to call
            Q_ARG(QVariant, QVariant::fromValue(newElement))  // method parameter
        );
    }
}
void cMainProcess::addtoTrackList(QObject *rootObject)
{
    QString path = "/mnt/usb/tracks.csv";
    vector <string> tracks;
    vector <string> elementAttr = {"trackname","s1x_lat","s1x_lon","s1y_lat","s1y_lon","s2x_lat","s2x_lon","s2y_lat","s2y_lon","s3x_lat","s3x_lon","s3y_lat","s3y_lon"};
    tracks = read_tracks(path.toStdString());
    for(uint i = 0; i < tracks.size() / 13; i++)
    {
        QVariantMap newElement;
        for(uint j = 0; j < 13; j++)
        {
            newElement.insert(QString::fromStdString(elementAttr[j]), QString::fromStdString(tracks[(i*13)+j]));
        }
        QMetaObject::invokeMethod(
           rootObject,                         // for this object we will call method
            "appendTracks",                                 // actually, name of the method to call
            Q_ARG(QVariant, QVariant::fromValue(newElement))  // method parameter
        );
    }
}
void cMainProcess::setTrack(QString trackname,QString s1x_lat,QString s1x_lon,QString s1y_lat,
                            QString s1y_lon,QString s2x_lat,QString s2x_lon,QString s2y_lat, QString s2y_lon,
                            QString s3x_lat,QString s3x_lon,QString s3y_lat,QString s3y_lon)
{
    Track newTrack;
    newTrack.sectors[0].p = {s1x_lat.toFloat(), s1x_lon.toFloat()};
    newTrack.sectors[0].q = {s1y_lat.toFloat(), s1y_lon.toFloat()};
    newTrack.sectors[1].p = {s2x_lat.toFloat(), s2x_lon.toFloat()};
    newTrack.sectors[1].q = {s2y_lat.toFloat(), s2y_lon.toFloat()};
    newTrack.sectors[2].p = {s3x_lat.toFloat(), s3x_lon.toFloat()};
    newTrack.sectors[2].q = {s3y_lat.toFloat(), s3y_lon.toFloat()};

    lapTimer->SetTrack(newTrack);
    currTrack(trackname);
}
