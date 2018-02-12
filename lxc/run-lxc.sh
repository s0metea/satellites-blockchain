#!/bin/bash
# file: run-lxc.sh

printf 'Note, that you need to run this script with sudo to succesfully launch LXC containers!\n';
for i in `seq 1 $1`;
    do
        lxc-start -n lxc-$i
    done
# Show bridges with tap devices
lxc-ls -f