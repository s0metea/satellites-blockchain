#!/bin/bash
# file: setup-route.sh
for ((i=0; i<$1; i++));
do
    <./conf/route/lxc$i.sh lxc-attach -n lxc$i -- /bin/sh -c "/bin/cat > /home/lxc$i.sh"
    lxc-attach -n lxc$i -- bash /home/lxc$i.sh
done
# Show bridges with tap devices
lxc-ls -f

