#!/bin/bash
# file: stop-lxc.sh
for ((i=0; i<$1; i++));
        do
		# Stop Linux Containers
		lxc-stop -n lxc$i -k
	done
lxc-ls -f
