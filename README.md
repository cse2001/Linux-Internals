# Linux Kernel Modules for Process and Memory Management

This repository contains a set of Linux Kernel Modules (LKMs) that provide insights into process and memory management in the Linux kernel. These modules demonstrate how to list processes, inspect process information, and map virtual to physical memory addresses.

## Installation

1. Install necessary packages:
    ```sh
    sudo apt install linux-headers-$(uname -r) build-essential
    ```

2. Follow instructions in each directory's `Makefile` to build and load the kernel modules and IOCTL drivers.

## Modules

### 1. lkm1.c - List Running or Runnable Processes
This kernel module lists all processes that are in a running or runnable state and prints their process IDs (PIDs).

#### Usage
- Load the module:
  ```sh
  sudo insmod lkm1.ko
  ```
- Check the kernel log for the output:
  ```sh
  dmesg | tail
  ```
- Unload the module:
  ```sh
  sudo rmmod lkm1
  ```

### 2. lkm2.c - List Child Processes
This kernel module takes a process ID as input and prints the PIDs and states of its child processes.

#### Usage
- Load the module with a PID:
  ```sh
  sudo insmod lkm2.ko pid=<PID>
  ```
- Check the kernel log for the output:
  ```sh
  dmesg | tail
  ```
- Unload the module:
  ```sh
  sudo rmmod lkm2
  ```

### 3. lkm3.c - Virtual to Physical Address Mapping
This kernel module takes a process ID and a virtual address as input. It determines if the virtual address is mapped and, if so, prints the corresponding physical address along with the PID and virtual address.

#### Usage
- Load the module with a PID and virtual address:
  ```sh
  sudo insmod lkm3.ko pid=<PID> vaddr=<VIRTUAL_ADDRESS>
  ```
- Check the kernel log for the output:
  ```sh
  dmesg | tail
  ```
- Unload the module:
  ```sh
  sudo rmmod lkm3
  ```

### 4. lkm4.c - Memory Allocation Stats
This kernel module, given a process ID, determines the size of the allocated virtual address space and the mapped physical address space. A test program is also provided to allocate memory in stages and observe the memory stats.

#### Usage
- Load the module with a PID:
  ```sh
  sudo insmod lkm4.ko pid=<PID>
  ```
- Run the test program:
  ```sh
  ./test_program
  ```
- Check the kernel log for the output:
  ```sh
  dmesg | tail
  ```
- Unload the module:
  ```sh
  sudo rmmod lkm4
  ```

## Building the Modules
To build all the modules, run:
```sh
make
```

## Loading and Unloading Modules
To load a module:
```sh
sudo insmod <module_name>.ko
```

To unload a module:
```sh
sudo rmmod <module_name>
```

## Kernel Log
To view the kernel log:
```sh
dmesg | tail
```
# Spaceship Object Collision Program and Memory Management with IOCTL

## Overview

This project includes multiple tasks involving kernel module programming and IOCTL (Input/Output Control) interface to interact with user-space programs. The project consists of three main components:

1. Implementing an IOCTL device driver to translate virtual to physical addresses and modify physical memory.
2. Implementing an IOCTL device driver to manage process signaling between a central station and soldier processes.
3. Creating `/proc` filesystem entries to display custom information.

## Task 1: Virtual-to-Physical Address Mapping and Memory Modification

### Objective
Create an IOCTL device driver to:
- Provide the physical address for a given virtual address.
- Write a byte value to a specified physical address.

### Components
- **Kernel Module**: `mem_ioctl.c`
- **User Space Program**: `mem_user.c`
- **Script**: `spock.sh`

### Implementation Details
1. **Kernel Module (`mem_ioctl.c`)**:
    - Provide IOCTL commands to get physical address and write to physical memory.

2. **User Space Program (`mem_user.c`)**:
    - Allocate a byte of memory, assign a value, and print the virtual address and value.
    - Use IOCTL calls to get the physical address and modify the value.
    - Verify the changes by printing the modified value.

3. **Script (`spock.sh`)**:
    - Compile the kernel module and user space application.
    - Initiate the IOCTL device.
    - Run the user space application.
    - Cleanly remove the IOCTL device.

### Usage
```sh
./spock.sh
```

## Task 2: Process Signaling with IOCTL

### Objective
Create an IOCTL device driver to:
- Change the parent process of a given PID so that it receives a `SIGCHLD` signal on exit.

### Components
- **Kernel Module**: `signal_ioctl.c`
- **Central Station Program**: Provided in the linked repository.
- **Soldier Program**: Provided in the linked repository.
- **Script**: `station_soldier.sh`

### Implementation Details
1. **Kernel Module (`signal_ioctl.c`)**:
    - Implement an IOCTL command to change the parent process of a given PID.

2. **Script (`station_soldier.sh`)**:
    - Compile the kernel module and provided programs.
    - Launch the central station and soldier programs.
    - Cleanly remove the IOCTL device.

### Usage
```sh
./station_soldier.sh
```

# Task 3: `/proc` Filesystem Entries

### Objective
Create kernel modules to introduce new entries in the `/proc` filesystem.

### Components
- **Hello World Proc Entry**: `hello_procfs.c`
- **Page Faults Proc Entry**: `get_pgfaults.c`

### Implementation Details
1. **Hello World Proc Entry (`hello_procfs.c`)**:
    - Create a `/proc/hello_procfs` entry that displays "Hello World!".

2. **Page Faults Proc Entry (`get_pgfaults.c`)**:
    - Create a `/proc/get_pgfaults` entry that displays the total number of page faults since system boot.

### Usage
```sh
# Insert the module
sudo insmod hello_procfs.ko
# Display the message
cat /proc/hello_procfs
# Remove the module
sudo rmmod hello_procfs

# Insert the module
sudo insmod get_pgfaults.ko
# Display page faults
cat /proc/get_pgfaults
# Remove the module
sudo rmmod get_pgfaults
```

## Compilation and Installation

To compile and install the kernel modules and user space programs, use the provided scripts.

```sh
# For Task 1
./spock.sh

# For Task 2
./station_soldier.sh
```

Ensure you have the necessary kernel headers and build tools installed on your system.

# Conclusion

This project demonstrates the use of IOCTL for interacting between user space and kernel space, managing process signaling, and creating `/proc` filesystem entries for custom information. These tasks highlight key aspects of Linux kernel module programming and memory management.

# Directory Structure

```
Linux-INternals-Understanding-and-eXploration/
├── 1/
│   ├── lkm1.c
│   ├── lkm2.c
│   ├── lkm3.c
│   ├── lkm4.c
│   ├── Makefile
│   └── Kbuild* (if used)
├── 2_I/
│   ├── spock.sh
│   └── ... (solution specific files/dirs)
├── 2_II/
│   ├── control_station.c
│   ├── soldier.c
│   ├── run_dr_doom.sh
│   ├── Makefile
│   └── ... (solution specific files/dirs)
└── 3/
    ├── hello_procfs.c
    ├── get_pgfaults.c
    ├── Makefile
    └── Kbuild* (if needed)
```

# References

- [Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)
- [Bootlin Linux Source Code](https://elixir.bootlin.com/linux/v6.1/source)
- [IOCTL Implementation Resources](https://github.com/pokitoz/ioctl_driver)

For any questions or support, please contact [aratrik1999@gmail.com](mailto:aratrik1999@gmail.com)

# Author
Aratrik Chandra