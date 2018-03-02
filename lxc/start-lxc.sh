#!/bin/bash
# file: start-lxc.sh
for ((i=0; i<$1; i++));
do
    lxc-start -n lxc$i
done
lxc-ls -f

