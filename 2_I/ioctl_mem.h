#ifndef IOCTL_H
#define IOCTL_H

#include <linux/ioctl.h>

// Structure to hold data exchanged between user-space and kernel-space
struct ioctl_data {
    unsigned long virtual_addr;
    unsigned long physical_addr;
    unsigned char value; // Byte value
};

#define DEVICE_NAME "ioctl_mem" // Device name used in the file operations

#define IOCTL_GET_PHYSICAL_ADDR _IOR('k', 0, struct ioctl_data) // IOCTL command to get the physical address for a given virtual address
#define IOCTL_WRITE_MEM _IOWR('k', 1, struct ioctl_data) // IOCTL command to write a byte value to a memory address given its physical address

#endif