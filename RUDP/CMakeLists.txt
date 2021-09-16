# CMakeList.txt : CMake project for RUDP, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.2)

set (INCLUDE_FILES
        include/RUDP.h
        include/RUDPHost.h
        include/RUDPPacket.h
        include/RUDPPeer.h
        include/RUDPProtocol.h
        include/RUDPTime.h
        include/RUDPUnix.h
        include/RUDPWin32.h)

set (SOURCE_FILES
        src/RUDPHost.cpp
        src/RUDPUnix.cpp
        src/RUDPWin32.cpp)

# Add source to this project's executable.
add_library (RUDP SHARED ${INCLUDE_FILES} ${SOURCE_FILES})

# TODO: Add tests and install targets if needed.
