#!/bin/bash
# file: clear.sh

for i in `seq 1 $1`;
        do
		# Stop the two Linux Containers
		lxc-stop -n lxc-$i -k
		# Destroy the containers
		lxc-destroy -n lxc-$i
	done
lxc-ls -f


