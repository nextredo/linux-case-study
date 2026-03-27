cmake_minimum_required(VERSION 3.20)
project(UdpProject VERSION 0.0.1 LANGUAGES CXX)

# Enable testing support before adding test subdirectory
enable_testing()

# Include subdirectories
add_subdirectory(src)
add_subdirectory(test)
