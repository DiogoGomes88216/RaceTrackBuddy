#ifndef PENDRIVE_H
#define PENDRIVE_H

#include <sys/epoll.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <iostream>
#include <dirent.h>
#include <syslog.h>
#include <fcntl.h>   // open
#include <unistd.h>  // read, write, close
#include <cstdio>    // BUFSIZ
#include <semaphore.h>
#include <libudev.h>
#include <sys/statvfs.h>
#include <time.h>
#include <cstring>

#define MOUNT_PATH "/mnt/usb"
#define BLOCK_PATH "/dev/sda"
#define MAX_EVENTS 5

using namespace std;

class penDrive
{
public:
    penDrive();
    ~penDrive();
    void DaemonCreator();
    void unmount();
    void startUpMount();
    void setupEvent();
    struct udev_device* get_child(struct udev* udev, struct udev_device* parent, const char* subsystem);

    void Mount(string devnode);
    void checkDevState();

private:
    struct udev *udev;
    struct udev_monitor *monitor;
    struct udev_device *dev;
    struct epoll_event epoll_udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *dev_list;

    DIR *dir;
    pid_t pid;
    int fd_epoll, fd_udev, status;
    string action, devicenode, devicesubsystem, partition, path, block;


};


#endif // PENDRIVE_H
