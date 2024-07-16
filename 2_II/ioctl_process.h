#ifndef IOCTL_H
#define IOCTL_H

#include <linux/ioctl.h>


#define DEVICE_NAME "ioctl_process"  // Device name used in the file operations

#define IOCTL_SET_PARENT_PID _IOW('k', 0, pid_t)  // IOCTL command to modify parent of a process

#endif