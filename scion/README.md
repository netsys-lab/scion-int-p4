Dockerized SCION Topologies
===========================

This directory contains definitions for two local SCION topologies:
- A diamond-shaped topology consisting of four ASes (script: `topo`)
- A star topology with one core AS and six non-core ASes (script: `demo`)

Both topologies are run in the same way (replace `topo` with the correct script):
```bash
$ ./topo build # Build switch and Docker container
$ ./topo run   # Create SCION configuration and run the topology
$ ./topo stop  # Stop the switches and containers
```
