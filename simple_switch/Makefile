P4C = p4c
P4TEST = p4test
CXX = clang++
CMAKE = cmake

CXXFLAGS += -Wall -Wextra -Wno-unused-parameter -Werror -std=c++20 -I ../control_plane

BUILD_DIR = ./build
LOG_DIR = ./log

P4INFO := p4info.txt
DEVICE_CONFIG := $(P4_SRC:.p4=.json)

SIMPLE_SWITCH_TOPO = ../simple_switch.py
SW_ARCH ?= v1model
SW_LOG_LEVEL ?= info


.PHONY: all
all: dataplane controller


# Data Plane

.PHONY: dataplane
dataplane: $(BUILD_DIR)/$(DEVICE_CONFIG)

$(BUILD_DIR)/$(DEVICE_CONFIG): ../data_plane/$(P4_SRC)
	$(P4C) --target bmv2 --arch $(SW_ARCH) \
	-o $(BUILD_DIR) --p4runtime-files $(BUILD_DIR)/$(P4INFO) $<


# Controller

.PHONY: controller
controller: $(BUILD_DIR)/controller/controller

$(BUILD_DIR)/controller/controller: $(BUILD_DIR)/controller/Makefile
	$(MAKE) -C $(@D)

$(BUILD_DIR)/controller/Makefile: controller/CMakeLists.txt $(BUILD_DIR)/controller
	$(CMAKE) -D CMAKE_CXX_COMPILER=$(CXX) -S $(<D) -B $(@D)

$(BUILD_DIR)/controller:
	mkdir -p $(BUILD_DIR)/controller

.PHONY: docs
docs:
	doxygen


# Mininet

.PHONY: run
run: dataplane controller $(LOG_DIR)
	rm -f $(LOG_DIR)/*
	sudo python3 $(SIMPLE_SWITCH_TOPO) --config $(BUILD_DIR)/$(DEVICE_CONFIG) \
	--log-file=$(LOG_DIR)/simple_switch --log-level=$(SW_LOG_LEVEL) $(TOPOFALGS)

$(LOG_DIR):
	mkdir -p $(LOG_DIR)


# Utilities

.PHONY: check-p4
check-p4:
	$(P4TEST) ../data_plane/$(P4_SRC)

.PHONY: check-cpp
check-cpp: $(SRC)
	clang-tidy $^ -- $(CXXFLAGS)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(LOG_DIR)
