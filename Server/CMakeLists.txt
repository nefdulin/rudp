cmake_minimum_required (VERSION 3.2)

set (RUDP_INCLUDE_DIR ../RUDP/include)

include_directories (${RUDP_INCLUDE_DIR})

add_executable (Server "main.cpp")

set (RUDP_LIBRARY ${CMAKE_BINARY_DIR}/RUDP/RUDPLib.lib)

target_link_libraries (Server ${RUDP_LIBRARY})

message ("msesage is: " ${RUDP_LIBRARY})