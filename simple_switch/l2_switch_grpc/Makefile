P4_SRC = l2_switch.p4
CTRL_SRC = controller/main.cpp controller/connection.cpp controller/controller.cpp

LDFLAGS += `pkg-config --libs protobuf grpc grpc++` -lpiprotobuf -lpiprotogrpc
TOPOFALGS = --grpc localhost:9559 --cpu-port 128

include ../Makefile
