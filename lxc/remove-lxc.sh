#!/bin/bash
# file: remove-lxc.sh

for ((i=0; i<$1; i++));
        do
		# Stop the two Linux Containers
		lxc-stop -n lxc$i -k
		# Destroy the containers
		lxc-destroy -n lxc$i
	done
lxc-ls -f
