cmake_minimum_required (VERSION 3.2)

set (RUDP_INCLUDE_DIR ../RUDP/include)

include_directories (${RUDP_INCLUDE_DIR})

add_executable (Client main.cpp)

set (RUDP_LIBRARY ${CMAKE_BINARY_DIR}/RUDP/libRUDP.dylib)

target_link_libraries (Client ${RUDP_LIBRARY})

message ("msesage is: " ${RUDP_LIBRARY})