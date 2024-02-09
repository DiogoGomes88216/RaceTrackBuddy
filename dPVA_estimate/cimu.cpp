#include "cimu.h"
#include <unistd.h>
#include <fcntl.h>

cImu::cImu()
{
    g_force = {0,0};
    imu_fd = open("/dev/i2c-1", O_RDWR);
    if (imu_fd < 0)
    {
        cout << "coulnt open i2c" << endl;
    }

   ioctl(imu_fd, I2C_SLAVE, 0x68);

   char wake[2] = {0x6B, 0x1};
   write(imu_fd, wake, 2);

}
cImu::~cImu()
{
    close(imu_fd);
}

short readRawData(int fd, char *addr)
{
    char read_buff[2];

    if(write(fd, addr, 1) != 1)
        return -1;

    if(read(fd, read_buff, 2) < 1)
        return -1;

    return (read_buff[0] << 8 | read_buff[1]);
}
accel cImu::get_Imu()
{
    accel temp;
    char RegAx [2] = {RegAx_L, RegAx_H};
    char RegAz [2] = {RegAz_L, RegAz_H};

    //Ax = readRawData((char*)RegAx);
    temp.Ax = readRawData(imu_fd, RegAx);
    temp.Ay = readRawData(imu_fd, RegAz); //z-axis in IMU but y-axis for application

    temp.Ax = temp.Ax * 0.000061f ; //accel y both in IMU and application
    temp.Ay = temp.Ay * 0.000061f ; //z-axis in IMU but y-axis for our application
    return temp;
}


