#!/bin/bash
device_name="ioctl_process"

## Building the applications
make clean      # Clean previous builds
make            # Build the applications

## Build your ioctl driver and load it here
make load
major=$(cat /proc/devices | grep "$device_name")
major_number=($major)

#"========Removing and Replacing stale nodes=========="
sudo rm -f /dev/${device_name}      # Remove existing device node
sudo mknod /dev/${device_name} c ${major_number[0]} 0

sudo chmod 777 /dev/${device_name}      # Set permissions for the device node

###############################################

echo -e "\n ==============Execute===========================\n"

# Launching the control station
./control_station &         # Run the control station in the background
c_pid=$!                     # Capture the PID of the control station
#echo "Control station PID: $c_pid"

sleep 1


# Launching the soldier
./soldier $c_pid &       # Run the soldier with the PID of the control station as argument
s_pid=$!                 # Capture the PID of the soldier
s_ppid=$(ps -o ppid= -p $s_pid)         # Get the previous parent PID of the soldier

echo "Soldier PID: $! and Previous parent: $s_ppid"

sleep 2
echo -e "\n ================Done=========================\n"

kill =9 $c_pid          # Kill the control station

## Remove the driver here

sudo rm -f /dev/${device_name}      # Remove the device node
make clean                          # Clean up the build artifacts