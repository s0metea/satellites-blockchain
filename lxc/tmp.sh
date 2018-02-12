#!/bin/bash
# file: configure.sh
# Create the bridges we referenced in the conf files:
brctl addbr br-1
# Create the tap devices that ns-3 will use to get packets from the bridges into its process
tunctl -t tap-1
# Set their IP, MAC addresses and bring them up
ifconfig tap-1 0.0.0.0 promisc up
# Add the tap devices we just created to their bridges
#ifconfig tap-1 hw ether 00:00:00:00:00:01 #TODO
#ifconfig tap-1 10.0.0.1 netmask 255.255.255.0 up
brctl addif br-1 tap-$i
ifconfig br-1 up
# Show bridges with tap devices
brctl show
lxc-ls -f
