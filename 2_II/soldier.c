#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include "ioctl_process.h"
// Include header or define the IOCTL call interface and devide name


//**************************************************

/**
 * @brief Opens the specified driver.
 *
 * @param driver_name Name of the driver to open.
 * @return File descriptor of the opened driver, or -1 on failure.
 */
int open_driver(const char* driver_name) {

    int fd_driver = open(driver_name, O_RDWR);
    if (fd_driver == -1) {
        perror("ERROR: could not open driver");
    }

	return fd_driver;
}

/**
 * @brief Closes the specified driver.
 *
 * @param fd_driver File descriptor of the driver to close.
 */
void close_driver(int fd_driver) {

    int result = close(fd_driver);
    if (result == -1) {
        perror("ERROR: could not close driver");
    }
}


/**
 * @brief Main function to update the parent process of a specified process.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return Exit status of the program.
 */
int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Usage: %s <parent_pid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t parent_pid = atoi(argv[1]);


    // open ioctl driver
    char device_path[256];
    sprintf(device_path,"/dev/%s",DEVICE_NAME);

    int fd = open_driver(device_path);
    if (fd < 0) {
        perror("Failed to open the device file");
        return -1;
    }
    
    sleep(1);
    printf("ioctl call is made to update parent of the soldier\n");
    //call ioctl with parent pid as argument to change the parent
    if (ioctl(fd, IOCTL_SET_PARENT_PID, &parent_pid) < 0) {
        perror("IOCTL call failed");
        close(fd);
        return -1;
    }

	printf("pid of soldier process is: %u and current parent id: %u\n", getpid(), getppid());
    // close ioctl driver
    close_driver(fd);
	return EXIT_SUCCESS;
}