project(cd3_rpc)
cmake_minimum_required(VERSION 2.8)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS cd3.proto)

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST} ${PROTO_SRCS})
target_link_libraries(${PROJECT_NAME} -lprotobuf -lzmq)

