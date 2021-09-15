# CMakeList.txt : CMake project for RUDP, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.2)

set (SRC_FOLDER src/)

# Add source to this project's executable.
add_library (RUDPLib STATIC ${SRC_FOLDER}RUDPHost.cpp ${SRC_FOLDER}UDPSocket.cpp)

# TODO: Add tests and install targets if needed.
