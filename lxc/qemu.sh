qemu-system-x86_64 --enable-kvm -hda lesclient1.qcow -m 1024 -net nic,macaddr=00:00:00:00:01:01 -net tap,ifname=tap-1,script=no
