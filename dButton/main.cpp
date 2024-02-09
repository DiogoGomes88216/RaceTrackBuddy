#include <iostream>
#include "button.h"
#include <signal.h>
#include <syslog.h>


using namespace std;

volatile sig_atomic_t flag;

button btn;


void signal_catcher(int signo, siginfo_t *info, void *context)
{
    if(info->si_signo ==  SIG_BTN)
        flag = 1;
}

int main()
{
    btn.DaemonCreator();
    pid_t child;
    int file_descriptor;
     char *args[] = {"-platform linuxfb"};
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = (SA_SIGINFO | SA_RESTART);
    act.sa_sigaction = signal_catcher;

    sigaction(SIG_BTN, &act, NULL);

    file_descriptor = open("/dev/button", O_RDWR);

    if(file_descriptor < 0){
        syslog(LOG_ERR, "\nFailed to open /dev/button: %s\n\n", strerror(errno));
       // printf("\nFailed to open /dev/button: %s\n\n", strerror(errno));	//error handling
        return -EINVAL;
    }
    else{
        syslog(LOG_SYSLOG,"\n/dev/button Opened!\nFile descriptor: %d\nWaiting for signal: %d \n", file_descriptor, SIG_BTN);
        //printf("\n/dev/button Opened!\nFile descriptor: %d\nWaiting for signal: %d \n", file_descriptor, SIG_BTN);//success
    }
    system("vcgencmd display_power 0");
    while(1)
    {
        if(flag == 1)
        {
            flag = 0;
            syslog(LOG_SYSLOG, "Start Program\n");

            system("vcgencmd display_power 1");
            // init RaceTrackBuddy
            btn.spawnChild("/etc/RaceTrackBuddy/RaceTrackBuddy", args);
            //block until new button Press
            while(flag == 0){}
            flag = 0;
            syslog(LOG_SYSLOG, "Stop Program\n");
            // turn OFF screen
            system("vcgencmd display_power 0");
            // close RaceTrackBuddy
            btn.shutdownMainProcess();
        }
    }
    close(file_descriptor);
    return 0;
}
