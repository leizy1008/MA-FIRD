cmake_minimum_required(VERSION 3.18)
project(MA-FIRD)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

if(WIN32)
    message(STATUS "Configuring for Windows")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif(UNIX)
    message(STATUS "Configuring for Linux")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto")
else()
    message(FATAL_ERROR "Unsupported operating system")
endif()

include_directories("include/")
aux_source_directory("src/" SRC)
add_executable(MA-FIRD ${SRC})
message("CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}")
