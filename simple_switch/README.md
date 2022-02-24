Control and data plane programs for the `simple_switch`
=======================================================

Directory Contents
------------------

### BMv2 Programs with P4Runtime Controllers
- [l2_switch_grpc](l2_switch_grpc/) Very simple learning Ethernet switch.
- [int_switch](int_switch/) In-band Network Telemetry for SCION

The actual control adn data plane implementations are in [control_plane](control_plane/) and
[data_plane](data_plane/).


### Mininet
- [simple_switch.py](simple_switch.py) contains a simple Mininet topology definition for basic
testing. There are two switches: SimpleSwitch and SimpleSwitchGrpc which correspond to the bmv2
targets simple_switch and simple_switch_grc.


### Obsolete Projects
- [l2_switch](l2_switch/) Incomplete learning Ethernet switch. Uses PI library directly.
