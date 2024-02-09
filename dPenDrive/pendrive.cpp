#include "pendrive.h"

penDrive::penDrive() : fd_epoll{-1}, fd_udev{-1}
{
    path=MOUNT_PATH;
    udev = udev_new();

}
penDrive::~penDrive()
{

}

void penDrive::DaemonCreator(){

    openlog("dPenDrive", LOG_PID, LOG_DAEMON);

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
void penDrive::unmount(){

    dir = opendir(path.c_str());

    if(dir){

        closedir(dir);

        status= umount2(path.c_str(), MNT_DETACH);

        if(status != 0){

            syslog(LOG_ERR, "%m\n");
        }

        status=rmdir(path.c_str());

        if(status != 0){

           syslog(LOG_ERR, "%m\n");
        }
    }
}
void penDrive::startUpMount(){

    enumerate = udev_enumerate_new(udev);


    udev_enumerate_add_match_subsystem(enumerate, "scsi");
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "scsi_device");
    udev_enumerate_scan_devices(enumerate);

    dev_list = udev_enumerate_get_list_entry(enumerate);


    struct udev_list_entry *entry;

    /// enumerate through any that are installed
    udev_list_entry_foreach(entry, dev_list){

    const char *path1 = udev_list_entry_get_name(entry);

    struct udev_device *scsi = udev_device_new_from_syspath(udev, path1);
    struct udev_device *block = get_child(udev, scsi, "block");
    struct udev_device *scsi_disk = get_child(udev, scsi, "scsi_disk");
    struct udev_device *usb = udev_device_get_parent_with_subsystem_devtype(scsi, "usb", "usb_device");

    //cout << "chegou!" << endl;
    if(usb && block && scsi_disk){

        string devnode =  udev_device_get_devnode(block);
        devnode.append("1");
        Mount(devnode);
        }

        if(block){
              udev_device_unref(block);
        }
        if(scsi_disk){
              udev_device_unref(scsi_disk);
        }
        udev_device_unref(scsi);

        }
    udev_enumerate_unref(enumerate);
}

void penDrive::setupEvent()
{
    monitor = udev_monitor_new_from_netlink(udev, "udev");

    udev_monitor_filter_add_match_subsystem_devtype(monitor, "usb", "usb_device");
    udev_monitor_filter_add_match_subsystem_devtype(monitor, "block", NULL);


    udev_monitor_enable_receiving(monitor);


    fd_epoll = epoll_create1(0);

    if (fd_epoll  < 0) {
        syslog(LOG_ERR, "%m\n");
        exit(EXIT_FAILURE);
    }


    fd_udev = udev_monitor_get_fd(monitor);


    epoll_udev.events = EPOLLOUT;
    epoll_udev.data.fd = fd_udev;
    if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_udev, &epoll_udev) < 0) {
        syslog(LOG_ERR, "%m\n");
        exit(EXIT_FAILURE);
    }

}

struct udev_device* penDrive::get_child(struct udev* udev, struct udev_device* parent, const char* subsystem){
    struct udev_device* child = NULL;
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_parent(enumerate, parent);
    udev_enumerate_add_match_subsystem(enumerate, subsystem);
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;

    udev_list_entry_foreach(entry, devices) {
        const char *path = udev_list_entry_get_name(entry);
        child = udev_device_new_from_syspath(udev, path);
        break;
    }

    udev_enumerate_unref(enumerate);
    return child;
}
void penDrive::Mount(string devnode){

    dir = opendir(path.c_str());
        if(!dir){


            status = mkdir(path.c_str(), 777);
            //cout << "1:" << status<< endl;

            if(status < 0){

               syslog(LOG_ERR, "mkdir: %m\n");
            }
        }
        //cout << "chegou1!" << endl;
        //cout << devicenode.c_str() << endl;
         //cout << path.c_str() << endl;
        status = mount(devnode.c_str(), path.c_str(), "vfat", MS_NOATIME, NULL);
        //cout << "2:" << status<< endl;

        if(status < 0){

            syslog(LOG_ERR, "mount: %m\n");
        }

}

void penDrive::checkDevState()
{
   dev = udev_monitor_receive_device(monitor);

   if(dev != NULL){
       action = udev_device_get_action(dev);
       devicenode = udev_device_get_devnode(dev);
       partition = udev_device_get_devtype(dev);
       syslog(LOG_SYSLOG, "mkdir: %m\n");


       if(!action.compare("remove") && !partition.compare("partition")){

           unmount();
       }
       if(!action.compare("add") && !partition.compare("partition")){
           Mount(devicenode);

       }
   }

}
