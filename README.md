# satellites-blockchain

## LXC setup
Just run `configure.sh` script from the `lxc` folder.
Typical using:
1) `sudo sh configure.sh i`, where `i` is number of desired lxc will setup tap bridges, and launch `i` Linux containers.
2) `sudo sh clear.sh i`, where `i` with the same `i` from the previous step to restore all the changes we have made.


## Launching examples:
./waf --run "satellites-net-device-example --tracePath=Path/To/Trace/ --maxBytes=10000 --linksPath=Path/To/Links"