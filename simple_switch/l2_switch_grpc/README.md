l2_switch_grpc
==============

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
$ ~/behavioral-model/tools/runtime_CLI.py
```
Try commands like `show_tables` and `table_dump forward_table`. Type `EOF` to exit.
