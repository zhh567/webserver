cmake_minimum_required(VERSION 3.15)
project(webserver_net_test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -rdynamic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -rdynamic")

set(WEBSERVER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../)
message(${WEBSERVER_DIR})


# 指定要递归查找的目录
# 使用 file(GLOB ...) 命令递归查找所有 .cc 文件
file(GLOB_RECURSE SOURCE_FILES_CC 
  "${WEBSERVER_DIR}/*.cc"
)
# 使用 list(FILTER ...) 命令排除指定目录的文件
list(FILTER SOURCE_FILES_CC EXCLUDE REGEX "${WEBSERVER_DIR}/tests")
# 输出找到的 .cc 文件（可选，用于验证）
foreach(file ${SOURCE_FILES_CC})
    message("Found source file: ${file}")
endforeach()

include_directories(
  ${CMAKE_SOURCE_DIR}/../../
)

add_executable(EchoServer
  ./EchoServer.cc
  ${SOURCE_FILES_CC}
)

