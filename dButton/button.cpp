#include "button.h"

button::button()
{

}

button::~button()
{

}

void button::DaemonCreator()
{
    openlog("dButton", LOG_PID, LOG_DAEMON);

    pid = fork();

    if(pid < 0){

        syslog(LOG_ERR,"%s\n", "fork");
        exit(EXIT_FAILURE);

    }
    if(pid > 0){

        exit(EXIT_SUCCESS);
    }
    if(setsid() < 0){

        syslog(LOG_ERR, "%s\n", "setsid");
        exit(EXIT_FAILURE);

    }
    pid = fork();
    if(pid < 0){

        syslog(LOG_ERR,"%s\n", "fork");
        exit(EXIT_SUCCESS);
    }
    if(pid > 0){

        exit(EXIT_SUCCESS);
    }
    if(setsid() < 0){

        syslog(LOG_ERR, "%s\n", "setsid");
        exit(EXIT_FAILURE);
    }

    umask(0);

    if(chdir("/") < 0){

        syslog(LOG_ERR, "%s\n", "chdir");
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);  // close standard input file descriptor
    close(STDOUT_FILENO); // close standard output file descriptor
    close(STDERR_FILENO); // close standard error file descriptor
}

void button::shutdownMainProcess()
{
    int pid;
    //attach to existing semaphore
    sem_t * sem_prod = sem_open(SEM_PRODUCER_FNAME, 0);
    if(!sem_prod)
            syslog(LOG_SYSLOG, "\nPen Drive producer sempahore\n\n");

    //grab the shared memory block
    char* block = attach_block(FILENAME_MP, 64);
    if(!block)
        syslog(LOG_SYSLOG, "\nShared memory block\n\n");

    //wait for pid
    sem_wait(sem_prod);

    //(...) get pid
    pid = atoi(block);
    //kill(pid, SIGTERM);
    syslog(LOG_SYSLOG, "\nKILLING PROCESS %d\n\n",pid);

    kill(pid,SIGKILL);
    //kill(pid,SIGTERM);

    //let go of shared memory block and semaphore
    sem_close(sem_prod);
    detach_block(block);
}
pid_t button::spawnChild(const char* program, char** arg_list)
{
    pid_t ch_pid = fork();

    if (ch_pid > 0) {
        printf("spawn child with pid - %d\n", ch_pid);
        wait(NULL);
        return ch_pid;
    } else {
        pid_t ch2_pid = fork();

        if(ch2_pid > 0)
            exit(0);
        execvp(program, arg_list);
        perror("execve");
        exit(EXIT_FAILURE);
    }
}
