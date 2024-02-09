#include <iostream>
#include "pendrive.h"
#include <signal.h>

volatile sig_atomic_t status;

using namespace std;

penDrive penD;

int main()
{
    penD.DaemonCreator();
    penD.startUpMount();

    penD.setupEvent();

    while(1){
        penD.checkDevState();
        sleep(1);
    }
    syslog(LOG_INFO, "Daemon terminated!\n");
    closelog();
    return 0;
}
