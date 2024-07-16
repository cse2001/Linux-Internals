#!/bin/bash
device_name_1="hello_procfs"
device_name_2="get_pgfaults"


## Building the applications
make clean
make 

## Build your procfs driver and load it here
make load


###############################################
echo -e "\n -----------------------------------------\n"

# Launching the control station
cat /proc/${device_name_1}

## Remove the driver here
echo -e "\n -----------------------------------------\n"

# Launching the control station
cat /proc/${device_name_2}

## Remove the driver here
echo -e "\n -----------------------------------------\n"
make clean