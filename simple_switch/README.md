`simple_switch` Example Programs
================================

[simple_switch.py](simple_switch.py) contains a simple Mininet topology definition. There are two
switches: SimpleSwitch and SimpleSwitchGrpc which correspond to the bmv2 targets simple_switch
and simple_switch_grc.


l2_switch_grpc (simple_switch_grpc)
-----------------------------------
Learning Ethernet switch with a P4Runtime controller implemented in C++.

Run the dataplane:
```
$ cd l2_switch_grpc
$ make run
```

The controller is not integrated in the Mininet topology and has to be started in a second terminal:
```
$ build/controller/ctrl build/p4info.txt build/l2_switch.json localhost:9559 0 1
```
The controller will exit when the Mininet instance is terminated.

You can also attach additional controllers with a different election ID, although every change in
primary controller simply resets the pipeline and does not keep state:
```
$ build/controller/ctrl build/p4info.txt build/l2_switch.json localhost:9559 0 2
```

Inspecting the dataplane with the bmv2 runtime CLI:
```
$ behavioral-model/tools/runtime_CLI.py
```
Try commands like `show_tables` and `table_dump forward_table`. Type `EOF` to exit.


l2_switch (simple_switch)
-------------------------
Incomplete learning Ethernet switch. Uses PI library directly.

Run the dataplane:
```
$ cd l2_switch_grpc
$ make run
```

Run the controller in a second terminal:
```
$ build/controller/ctrl
```
