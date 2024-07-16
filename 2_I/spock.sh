#!/bin/bash

device_name="ioctl_mem"

echo "============loading device============="
make unload
make clean
make
make load

# retrieve major number
major=$(cat /proc/devices | grep "$device_name")
major_number=($major)


echo "--------Removing and then Replacing stale nodes----------"
sudo rm -f /dev/${device_name}
sudo mknod /dev/${device_name} c ${major_number[0]} 0

echo -e "\n============Execute=============\n"
./test
make see

echo -e "\n==========Done==============\n"
make unload
make clean