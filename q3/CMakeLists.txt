cmake_minimum_required(VERSION 3.20)
project(UdpProject VERSION 0.0.1 LANGUAGES CXX)

# Make compile commands, for clang-tidy support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable testing support before adding test subdirectory
enable_testing()

# Doxygen documentation support
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(
        docs
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
    message(STATUS "Doxygen found: docs target added (run 'make docs' to generate)")
else()
    message(WARNING "Doxygen not found: documentation target will not be available")
endif()

# Include subdirectories
add_subdirectory(src)
add_subdirectory(test)
