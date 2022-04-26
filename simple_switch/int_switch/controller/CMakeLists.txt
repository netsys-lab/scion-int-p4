cmake_minimum_required(VERSION 3.16)

project (l2_switch_grpc)
find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
# find_package(asio-grpc REQUIRED)

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/../../control_plane/controllers/int/report/report.pb.cc
                   COMMAND make all
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../control_plane/controllers/int/report/
)

set_property(SOURCE ../../control_plane/controllers/int/report/report.pb.cc PROPERTY SYMBOLIC 1)

add_executable(ctrl
    main.cpp
    ../../control_plane/connection.cpp
    ../../control_plane/control_plane.cpp
    ../../control_plane/p4_util.cpp
    ../../control_plane/controllers/default.cpp
    ../../control_plane/controllers/mac_learn.cpp
    ../../control_plane/controllers/int/int.cpp
    ../../control_plane/controllers/int/kafkaProducer.cpp
    ../../control_plane/controllers/int/tcpClient.cpp
    ../../control_plane/controllers/int/report/report.pb.cc)

set_property(TARGET ctrl PROPERTY CXX_STANDARD 20)
target_include_directories(ctrl PRIVATE ../../control_plane)
target_link_libraries(ctrl PRIVATE Boost::boost)
target_link_libraries(ctrl PRIVATE protobuf::libprotobuf)
target_link_libraries(ctrl PRIVATE gRPC::grpc++)
# target_link_libraries(ctrl PRIVATE asio-grpc::asio-grpc)
target_link_libraries(ctrl PRIVATE -lpiprotobuf)
target_link_libraries(ctrl PRIVATE -lpiprotogrpc)
target_link_libraries(ctrl PRIVATE -lcppkafka)
target_link_libraries(ctrl PRIVATE -lboost_system)
target_link_libraries(ctrl PRIVATE -lboost_coroutine)
target_link_libraries(ctrl PRIVATE -lboost_context)
