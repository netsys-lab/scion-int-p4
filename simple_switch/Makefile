P4C = p4c
P4TEST = p4test
CXX = clang++
SIMPLE_SWITCH_TOPO = ../simple_switch.py

CXXFLAGS += -Wall -Werror -std=c++17 -MMD -MP

BUILD_DIR = ./build
LOG_DIR = ./log

P4INFO = p4info.txt
DEVICE_CONFIG = $(P4_SRC:.p4=.json)
OBJS = $(CTRL_SRC:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

SW_ARCH ?= v1model
SW_LOG_LEVEL ?= info

all: dataplane controller

# Data Plane

check:
	$(P4TEST) $(source)

dataplane: $(BUILD_DIR)/$(DEVICE_CONFIG)

$(BUILD_DIR)/%.json: %.p4
	$(P4C) --target bmv2 --arch $(SW_ARCH) \
	-o $(BUILD_DIR) --p4runtime-files $(BUILD_DIR)/$(P4INFO) $<

# Controller

controller: $(BUILD_DIR)/controller/ctrl

$(BUILD_DIR)/controller/ctrl: $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

# Mininet

run: dataplane controller log_dir
	rm -f $(LOG_DIR)/*
	sudo python3 $(SIMPLE_SWITCH_TOPO) --config $(BUILD_DIR)/$(DEVICE_CONFIG) \
	--log-file=$(LOG_DIR)/simple_switch --log-level=$(SW_LOG_LEVEL) $(TOPOFALGS)

# Utilities

log_dir:
	mkdir -p $(LOG_DIR)

clean:
	rm -rf $(BUILD_DIR) $(LOG_DIR)
