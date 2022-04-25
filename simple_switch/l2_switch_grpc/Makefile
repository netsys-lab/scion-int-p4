P4_SRC = l2_switch.p4
TOPOFALGS = --grpc localhost:9559 --cpu-port 128

VPATH := controller:../control_plane:../control_plane/controllers
SRC := $(notdir $(wildcard controller/*.cpp))
SRC += $(notdir $(wildcard ../control_plane/*.cpp) $(wildcard ../control_plane/controllers/*.cpp))

include ../Makefile
