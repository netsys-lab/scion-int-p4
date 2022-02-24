P4C = p4c
P4TEST = p4test
CXX = clang++
SIMPLE_SWITCH_TOPO = ../simple_switch.py

CXXFLAGS += -Wall -Werror -std=c++17 -MMD -MP
LDFLAGS = -lpi -lpifecpp -lpip4info -lpi_bmv2 -lbmp4apps -lthrift

BUILD_DIR = ./build
LOG_DIR = ./log

P4_SRC = l2_switch.p4
CTRL_SRC = controller/main.cpp

P4INFO = p4info.txt
DEVICE_CONFIG = $(P4_SRC:.p4=.json)
OBJS = $(CTRL_SRC:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

SW_ARCH ?= v1model
SW_LOG_LEVEL ?= info

.PHONY: all
all: dataplane controller

# Data Plane

.PHONY: check
check:
	$(P4TEST) $(source)

.PHONY: dataplane
dataplane: $(BUILD_DIR)/$(DEVICE_CONFIG)

$(BUILD_DIR)/$(DEVICE_CONFIG): ../data_plane/$(P4_SRC)
	$(P4C) --target bmv2 --arch $(SW_ARCH) \
	-o $(BUILD_DIR) --p4runtime-files $(BUILD_DIR)/$(P4INFO) $<

# Controller

.PHONY: controller
controller: $(BUILD_DIR)/controller/ctrl

$(BUILD_DIR)/controller/ctrl: $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

# Mininet

.PHONY: run
run: dataplane controller $(LOG_DIR)
	rm -f $(LOG_DIR)/*
	sudo python3 $(SIMPLE_SWITCH_TOPO) --config $(BUILD_DIR)/$(DEVICE_CONFIG) \
	--log-file=$(LOG_DIR)/simple_switch --log-level=$(SW_LOG_LEVEL) $(TOPOFALGS)

$(LOG_DIR):
	mkdir -p $(LOG_DIR)

# Utilities

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(LOG_DIR)
