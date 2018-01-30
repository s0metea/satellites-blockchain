#!/bin/bash
# file: clear.sh

for i in `seq 1 $1`;
        do
		# Stop the two Linux Containers
		lxc-stop -n lxc-$i -k

		# Take both of the bridges down
		ifconfig br-$i down

		# Remove the taps from the bridges
		brctl delif br-$i tap-$i

		# Destroy the bridges
		brctl delbr br-$i

		# Bring down the taps
		ifconfig tap-$i down

		# Delete the taps
		tunctl -d tap-$i

		# Destroy the containers
		lxc-destroy -n lxc-$i
	done
brctl show
lxc-ls -f

