#ifndef CIMU_H
#define CIMU_H

#include <iostream>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

using namespace std;

#define Device_addr 0x68

#define RegAx_L 59
#define RegAx_H 60
#define RegAy_L 61
#define RegAy_H 62
#define RegAz_L 63
#define RegAz_H 64


typedef struct
{
    float Ax, Ay;
}accel;

class cImu
{
protected:
    int imu_fd;
    accel g_force;

public:
   cImu();
    ~cImu();
    accel get_Imu();
    void  set_G_Force(accel G){g_force = G;}
    accel get_G_Force(){return g_force;}
};

#endif // CIMU_H
