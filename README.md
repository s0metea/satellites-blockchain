# satellites-blockchain
    Note! The project is unfinished, some parts of it may be broken and not working at all.


## Network topology
Every satellite (or ground station) in this model
can be easily connected to Linux Container (LXC) using Tap interface. In reason that
routing provided by LRR module exist only inside our simulation, containers
can not perform multihop route with respect to its neighbors.
To solve this problem we added additional nodes that acts like gateways.
With static route help we able to transmit all traffic over them.

## Launching examples:
1) Initial setup:
Download satellites-blockchain module and drop it to the src folder of NS3.
2) Download LRR module and then patch NS3.
3) Patch mobility module to add Z axis support.
4) Build NS3:
We needs sudo for tap bridges using, so:
`./waf configure --enable-examples --enable-sudo`\
`./waf build`\
`./waf --run "satellites-blockchain-example --tracePath=Path/To/Trace/ --linksPath=Path/To/Links"`

## Trace file:
We use ns2-mobility-helper to setup nodes mobility. Unfortunatelly, ns2-mobility-helper doesn't supports `Z` axis, so we need to patch it before we start.
FIle has the following structure:\
`$node(i) set X_ 0`\
`$node(i) set Y_ 0`\
`$node(i) set Z_ 0`\
`$ns_ at 1.0 "$node_(i) setdest 1057050.07 -388614.23 -6959651.68 7518.823577349052"`

## Creating links file:
Trace file describes satellites positions in moments of time.
Let assume we have 3 nodes and want to describe their connections in time.
The first row contains time from the start of simulation.
Supported units include:
* s (seconds),
* ms (milliseconds),
* us (microseconds),
* ns (nanoseconds),
* ps (picoseconds),
* fs (femtoseconds)

Next 3 rows of the length 3 will contain their links, for example:\
110\
111\
011\
Where `1` is full duplex connection and `0` otherwise (There are no connection between).

## LXC setup
We use Linux containers that executes Ethereum services.
Just run `init-lxc.sh` script from the `lxc` folder.
lxc-conf folder contains .conf files for LXC containers, they have the same structure for each container.
Typical using:
1) Run `sudo bash init-lxc.sh i`, where `i` is the number of desired lxc. The command will setup tap bridges and launch `i` Linux containers.
Note that for each of them there needs to be .conf file.
2) Run `sudo bash remove-lxc.sh i`, with the same `i` from the previous step to restore all the changes we have made.
