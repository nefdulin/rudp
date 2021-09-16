cmake_minimum_required (VERSION 3.2)

set (RUDP_INCLUDE_DIR ../RUDP/include)

include_directories (${RUDP_INCLUDE_DIR})

add_executable (Server "main.cpp")

message ("msesage is: " ${RUDP_LIBRARY})

set (RUDP_LIBRARY ${CMAKE_BINARY_DIR}/RUDP/libRUDP.dylib)

target_link_libraries (Server ${RUDP_LIBRARY})
