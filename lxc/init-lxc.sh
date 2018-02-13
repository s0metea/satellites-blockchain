#!/bin/bash
# file: init-lxc.sh
for ((i=0; i<$1; i++));
do
    lxc-create -f ./lxc$i.conf -n lxc$i -t ubuntu
    lxc-start -n lxc$i
done
# Show bridges with tap devices
lxc-ls -f

