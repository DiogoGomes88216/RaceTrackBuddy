#include "dpvaestimate.h"

void tSignalHandler(int sig, siginfo_t * info, void * context);

pthread_mutex_t mutSigHandler, mutImu, mutGnss;
pthread_cond_t cond_SigHandler;
bool Stop_flag = false;
sem_t * dPvaEstimate::sem_prod = nullptr;
sem_t * dPvaEstimate::sem_cons = nullptr;

dPvaEstimate::dPvaEstimate()
{
    file = new cFileSaver;
    gnss = new cGnss;
    imu = new cImu;

    initSem();
    initCondVar();
    initMutex();
}

void dPvaEstimate::initMutex()
{
    mutSigHandler = PTHREAD_MUTEX_INITIALIZER;
    mutImu = PTHREAD_MUTEX_INITIALIZER;
    mutGnss = PTHREAD_MUTEX_INITIALIZER;
}
void dPvaEstimate::initCondVar()
{
    cond_SigHandler = PTHREAD_COND_INITIALIZER;
}
void dPvaEstimate::initSem()
{
    //remove any leftover semaphore (precaution)
   sem_unlink(SEM_PRODUCER_FNAME);
   sem_unlink(SEM_CONSUMER_FNAME);

   //create new producer semaphore
   sem_prod = sem_open(SEM_PRODUCER_FNAME, O_CREAT, 0660, 0);
   if(sem_prod == SEM_FAILED)
       syslog(LOG_ERR, "Producer semaphore create: %m\n");

   //create new consumer semaphore
   sem_cons = sem_open(SEM_CONSUMER_FNAME, O_CREAT, 0660, 1);
   if(sem_cons == SEM_FAILED)
       syslog(LOG_ERR, "Consumer semaphore create: %m\n");
}
void dPvaEstimate::setThread(int priority, pthread_attr_t *pthread_attr, sched_param *pthread_param)
{
    pthread_attr_setschedpolicy(pthread_attr, SCHED_RR);
    pthread_param->sched_priority = priority;
    pthread_attr_setschedparam(pthread_attr, pthread_param); //The pthread_attr_setschedparam() function sets the scheduling parameter
    pthread_attr_setinheritsched(pthread_attr, PTHREAD_EXPLICIT_SCHED);
}
bool dPvaEstimate::initThreads()
{
    int threadPolicy;
    pthread_attr_t threadAttr;
    struct sched_param threadPara;

    pthread_attr_init(&threadAttr);
    pthread_attr_getschedparam(&threadAttr, &threadPara);
    pthread_attr_getschedpolicy(&threadAttr, &threadPolicy);

    //GNSS thread

    setThread(2, &threadAttr, &threadPara);
    if((pthread_create(&tGetGnssDataID, &threadAttr, tGetGnssData, static_cast<void*>(gnss))) != 0)
    {
        perror("Failed to Create GNSS thread");
        return false;
    }
    //IMU thread
    setThread(2, &threadAttr, &threadPara);
    if((pthread_create(&tGetImuDataID, &threadAttr, tGetImuData, static_cast<void*>(imu))) != 0)
    {
        perror("Failed to Create IMU thread");
        return false;
    }

    //Save data Thread
    setThread(3, &threadAttr, &threadPara);
    if((pthread_create(&tSaveDataFrameID, &threadAttr, tSaveDataFrame, static_cast<void*>(this))) != 0)
    {
        perror("Failed to Create Save thread");
        return false;
    }
    //Share PVA thread
    setThread(3, &threadAttr, &threadPara);
    if((pthread_create(&tSharePvaID, &threadAttr, tSharePva, static_cast<void*>(this))) != 0)
    {
        perror("Failed to Create Share thread");
        return false;
    }
    return true;
}

bool dPvaEstimate::run()
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = (SA_SIGINFO | SA_RESTART);
    act.sa_sigaction = tSignalHandler;
    sigaction(SIG_START, &act, NULL);
    sigaction(SIG_STOP, &act, NULL);

    if(initThreads())
    {
        pthread_join(tGetGnssDataID,nullptr);
        pthread_join(tGetImuDataID,nullptr);
        pthread_join(tSaveDataFrameID, nullptr);
        return true;
    }
    return false;
}

dPvaEstimate::~dPvaEstimate()
{
    pthread_mutex_destroy(&mutSigHandler);
    pthread_mutex_destroy(&mutImu);
    pthread_mutex_destroy(&mutGnss);
}
void dPvaEstimate::make_daemon()
{
    openlog("EventMonitor", LOG_PID, LOG_DAEMON);
    /* Fork off the parent process */
    myPid = fork();
    // on error exit
    if (myPid < 0){
        syslog(LOG_ERR, "%s\n", "fork");
        exit(EXIT_FAILURE);
    }
    /* Success: Let the parent terminate */
    if (myPid > 0){
        exit(EXIT_SUCCESS);
    }

    /* On success: The child process becomes session leader */
    sid = setsid(); // create a new session
    // on error exit
    if (sid < 0){
        syslog(LOG_ERR, "%s\n", "setsid");
        exit(EXIT_FAILURE);
    }

    // make '/' the root directory
    if (chdir("/") < 0) { // on error exit
        syslog(LOG_ERR, "%s\n", "chdir");
        exit(EXIT_FAILURE);
    }

    umask(0);

    close(STDIN_FILENO);  // close standard input file descriptor
    close(STDOUT_FILENO); // close standard output file descriptor
    close(STDERR_FILENO); // close standard error file descriptor
}

/* Threads */
void tSignalHandler(int sig, siginfo_t * info, void * context)
{
    pthread_mutex_lock(&mutSigHandler);
    if(sig == SIG_START)
    {
        pthread_cond_signal(&cond_SigHandler);
    }
    else if(sig == SIG_STOP)
    {
       Stop_flag = true;
    }
    pthread_mutex_unlock(&mutSigHandler);
}
void *dPvaEstimate::tSharePva(void *arg) //thread that puts current GPS and G-force on shared Memory
{
    dPvaEstimate *daemon;
    daemon = (dPvaEstimate*)arg;
    PVA pva;
    pva.daemonPid = getpid(); // grab process Pid so it can be shared

    PVA* block = (PVA*) attach_block(FILENAME, SH_BLOCK_SIZE); //grab the shared memory block
    if(!block)
        syslog(LOG_ERR, "Attach block: %m\n");

    while(true)
    {
        pthread_mutex_lock(&mutGnss);
        pva.gnss = daemon->gnss->getRMC(); //get current GPS information
        pthread_mutex_unlock(&mutGnss);

        pthread_mutex_lock(&mutImu);
        pva.g_force = daemon->imu->get_G_Force(); //get current G-force
        pthread_mutex_unlock(&mutImu);

        /* UPDATE SHARED MEMORY*/
        sem_wait(sem_cons); // wait for a consumer

        //put pva on shared memory
        *block = pva; //update the current shared memory block

        sem_post(sem_prod); //signal that memory is updated

        usleep(1000 * 100); // sleep for 100ms = 10Hz
    }
    detach_block((char *)block);
    destroy_block((char *)block);// destroy shared memory
}
void *dPvaEstimate::tSaveDataFrame(void *arg) //Saves current gps info and G-Force on .csv file
{
    dPvaEstimate *daemon;
    daemon = (dPvaEstimate*)arg;
    RMC gnss;
    accel g_force;
    string buff[6];
    string header[6] = {"TIME", "Latitude", "Longitude", "Speed", "X-Axis G-Force", "Y-Axis G-Force"};

    while (true)
    {
        /*  CONDTION Wait*/
        pthread_mutex_lock(&mutSigHandler);
        pthread_cond_wait(&cond_SigHandler, &mutSigHandler); //waits for START signal
        pthread_mutex_unlock(&mutSigHandler);

        /*  wait until GPS gets FIX */
//        do
//        {
            pthread_mutex_lock(&mutGnss);
            gnss = daemon->gnss->getRMC(); // gets current time and date
            pthread_mutex_unlock(&mutGnss);
            usleep(100 * 1000);
//        }
//        while(gnss.Status != 'A');

        //cout<< "STARTED"<<endl;

        // SET FILE NAME BASED ON CURRENT TIME AND DATE
        daemon->file->setFile_Path(to_string((int)gnss.Time), to_string(gnss.Date));
        daemon->file->WriteCsv(header, 6); //writes header buff in file

        //start putting data into file
        while (!Stop_flag)
        {
            pthread_mutex_lock(&mutGnss);
            gnss = daemon->gnss->getRMC(); // gets current time, position and speed
            pthread_mutex_unlock(&mutGnss);

            pthread_mutex_lock(&mutImu);
            g_force = daemon->imu->get_G_Force(); // gets current G-force values
            pthread_mutex_unlock(&mutImu);

            /*     UPDATE SHARED MEMORY    */

            /*     WRITE TO FILE  */
            buff[0] = to_string(gnss.Time); //writes current time
            buff[1] = to_string(gnss.Position.Lat);// current latitude
            buff[2] = to_string(gnss.Position.Lon);// current longitude
            buff[3] = to_string(gnss.Speed);// current Speed
            buff[4] = to_string(g_force.Ax); // current X-axis G-Force
            buff[5] = to_string(g_force.Ay); // current Y-axis G-Force

            daemon->file->WriteCsv(buff, 6); // writes current data to file

            usleep(1000 * 100); //waits
        }
        //cout<<"STOPED"<<endl;
        Stop_flag = false;
    }
}
void *dPvaEstimate::tGetImuData(void *arg)
{
    cImu *imu;
    imu = (cImu*) arg;

    accel Gs;

    while(true)
    {
        Gs = imu->get_Imu();
        pthread_mutex_lock(&mutImu);
        imu->set_G_Force(Gs);
        pthread_mutex_unlock(&mutImu);
        usleep(1000 * 100);
    }
}
void *dPvaEstimate::tGetGnssData(void *arg)
{
    cGnss *gnss;
    gnss = (cGnss*) arg;

    if(!gnss->Start())
        return NULL;

    while(true)
    {
        pthread_mutex_lock(&mutGnss);
        gnss->UpdateGnss();
        pthread_mutex_unlock(&mutGnss);

        usleep(100 * 1000);
    }
}
