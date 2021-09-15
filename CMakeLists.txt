# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.2)

project ("RUDP")

# Include sub-projects.
add_subdirectory ("RUDP")
# Client-Server project
add_subdirectory ("Server")
add_subdirectory ("Client")