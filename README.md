# satellites-blockchain

## LXC setup
Just run `configure.sh` script from the `lxc` folder.
Typical using:
1) run `sudo sh configure.sh i`, where `i` is the number of desired lxc. The command will setup tap bridges and launch `i` Linux containers.
2) run `sudo sh clear.sh i`, with the same `i` from the previous step to restore all the changes we have made.


## Launching examples:
./waf --run "satellites-blockchain-example --tracePath=Path/To/Trace/ --maxBytes=10000 --linksPath=Path/To/Links"


## Trace file:
Trace file describes satellites positions in moments of time.

## Creating links file:
Supported units include:
s (seconds),
ms (milliseconds),
us (microseconds),
ns (nanoseconds),
ps (picoseconds),
fs (femtoseconds).
