#!/bin/bash
# file: configure.sh
for i in `seq 1 $1`;
do
    lxc-create -f ./lxc-conf/lxc-$i.conf -n lxc-$i -t ubuntu
    lxc-start -n lxc-$i
done
# Show bridges with tap devices
lxc-ls -f
