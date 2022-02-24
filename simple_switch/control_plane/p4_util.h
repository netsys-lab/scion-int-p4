#pragma once

#include "common.h"
#include <p4/config/v1/p4info.pb.h>
#include <memory>

std::unique_ptr<p4::config::v1::P4Info> loadP4Info(const char* filename);
DeviceConfig loadDeviceConfig(const char* filename);
