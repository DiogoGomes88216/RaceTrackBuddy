#ifndef CGNSS_H
#define CGNSS_H

#include <stdlib.h>
#include <sstream>
#include <assert.h>
#include <vector>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string>
#include <pthread.h>

#define MINUTE_LENGTH 9
#define KNOTS_TO_KMH 1.852

using namespace std;

typedef struct
{
    float Lat, Lon;
}coordinates;

typedef struct
{
    char Status;
    double Time;
    coordinates Position;
    double Speed;
    int Date;

}RMC;

class cGnss
{
private:
    RMC info;

public:
    cGnss();
    bool Start();
    RMC getRMC(){return info;}
    void setRMC(const string RMCSentence);
    void UpdateGnss();
};

#endif // CGNSS_H
