#!/bin/bash
# file: init-lxc.sh
for ((i=0; i<$1; i++));
do
    lxc-create -f ./conf/lxc$i.conf -n lxc$i -t ubuntu
    lxc-start -n lxc$i
    <./conf/route/lxc$i.sh lxc-attach -n lxc$i -- /bin/sh -c "/bin/cat > /home/lxc$i.sh"
done
for ((i=0; i<$1; i++));
do
    lxc-attach -n lxc$i -- bash /home/lxc$i.sh
done
# Show bridges with tap devices
lxc-ls -f

