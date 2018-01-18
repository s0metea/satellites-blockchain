#!/bin/bash
# file: configure.sh
for i in `seq 1 $1`;
        do
		# Create the bridges we referenced in the conf files:
		brctl addbr br-$i

		# Create the tap devices that ns-3 will use to get packets from the bridges into its process
		tunctl -t tap-$i

		# Set their IP, MAC addresses and bring them up
		ifconfig tap-$i 10.0.0.$i netmask 255.255.255.0 up
        ifconfig tap-$i hw ether 00:00:00:00:00:0$i # TODO


		# Add the tap devices you just created to their respective 			# bridges
		brctl addif br-$i tap-$i
		ifconfig br-$i up

		# Create containers
		lxc-create -f ./lxc-conf/lxc-$i.conf -n lxc-$i -t ubuntu
		lxc-start -n lxc-$i
	done
# Show bridges with tap devices
brctl show
lxc-ls -f
