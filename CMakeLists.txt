cmake_minimum_required(VERSION 3.0.0)

project(Cliver)

set(CMAKE_CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug"
AND NOT CMAKE_BUILD_TYPE STREQUAL "Release"
AND NOT CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"
AND NOT CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
	set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})

set(CMAKE_STATIC_LIBRARY_PREFIX "")

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
	message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
	file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(REQUIRES boost/1.71.0 BASIC_SETUP BUILD missing)

message(WARNING "CONAN_BOOST_ROOT ${CONAN_BOOST_ROOT}")
message(WARNING "CONAN_INCLUDE_DIRS_BOOST ${CONAN_INCLUDE_DIRS_BOOST}")
message(WARNING "CONAN_LIB_DIRS_BOOST ${CONAN_LIB_DIRS_BOOST}")

add_subdirectory(Client)
add_subdirectory(Sever)
