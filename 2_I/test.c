#include <stdio.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>
#include <stdlib.h>

#include "ioctl_mem.h"  // Include the header file defining IOCTL commands

int main() {
    struct ioctl_data data={0};  // Initialize ioctl data structure
    char device_path[256]; // Buffer to store device file path
    int fd; // File descriptor for device file
    
    sprintf(device_path,"/dev/%s",DEVICE_NAME); // Construct device file path

    fd = open(device_path, O_RDONLY);  // Open the device file
    if (fd < 0) {
        perror("Failed to open the device file");
        return -1;
    }

    unsigned char* ptr_mem = (unsigned char*) malloc(sizeof(unsigned char)); // Allocate memory and set initial value
    *ptr_mem = 6;

    // Display virtual address and its allocated value
    printf("Virtual address is: %p ,and its allocated value is: %d\n", ptr_mem, *ptr_mem);
    
     // Set the virtual address in ioctl data structure
    data.virtual_addr=(unsigned long)ptr_mem;

// Call IOCTL_GET_PHYSICAL_ADDR to get the physical address corresponding to the virtual address
    if (ioctl(fd, IOCTL_GET_PHYSICAL_ADDR, &data) < 0) {
        perror("IOCTL call failed");
        close(fd);
        return -1;
    }

    // Display the physical address obtained from IOCTL
    printf("Physical address from IOCTL: %p\n", data.physical_addr);
    
    // Update data value and call IOCTL_WRITE_MEM to write to the physical address
    data.value = 5;
     if (ioctl(fd, IOCTL_WRITE_MEM, &data) < 0) {
        perror("IOCTL_WRITE_MEM failed");
        close(fd);
        return -1;
    }

    // Display the modified data after ioctl update
    printf("After ioctl update, at virtual address: %p Modified data is: %u\n",ptr_mem, *ptr_mem);

    //free memory
    free(ptr_mem);
    close(fd);
    return 0;
}
