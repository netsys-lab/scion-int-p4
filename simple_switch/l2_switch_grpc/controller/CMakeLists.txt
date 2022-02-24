cmake_minimum_required(VERSION 3.16)

project (l2_switch_grpc)
find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
# find_package(asio-grpc REQUIRED)

add_executable(ctrl
    main.cpp
    ../../control_plane/connection.cpp
    ../../control_plane/control_plane.cpp
    ../../control_plane/p4_util.cpp
    ../../control_plane/controllers/default.cpp
    ../../control_plane/controllers/mac_learn.cpp)

set_property(TARGET ctrl PROPERTY CXX_STANDARD 20)
target_include_directories(ctrl PRIVATE ../../control_plane)
target_link_libraries(ctrl PRIVATE Boost::boost)
target_link_libraries(ctrl PRIVATE protobuf::libprotobuf)
target_link_libraries(ctrl PRIVATE gRPC::grpc++)
# target_link_libraries(ctrl PRIVATE asio-grpc::asio-grpc)
target_link_libraries(ctrl PRIVATE -lpiprotobuf)
target_link_libraries(ctrl PRIVATE -lpiprotogrpc)
