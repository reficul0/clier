cmake_minimum_required(VERSION 3.0.0)
project( Client )

# Добавляем исходные файлы для цели
file(GLOB HEADERS "*.hpp")
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCES)

# Формируем цель
add_executable( ${PROJECT_NAME} ${SOURCES} ${HEADERS} )

SET(BOOST_ROOT ${CONAN_BOOST_ROOT})
SET(BOOST_INCLUDEDIR ${CONAN_INCLUDE_DIRS_BOOST})
set(BOOST_LIBRARYDIR ${CONAN_LIB_DIRS_BOOST})
set(BOOST_LIBRARY_DIR ${CONAN_LIB_DIRS_BOOST})

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.70.0 EXACT REQUIRED COMPONENTS regex filesystem system date_time chrono thread locale iostreams)

target_include_directories(${PROJECT_NAME} PRIVATE ${BOOST_INCLUDEDIR})
target_link_directories(${PROJECT_NAME} PRIVATE  ${BOOST_LIBRARYDIR})


target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS} Boost::regex Boost::system Boost::filesystem Boost::chrono Boost::thread Boost::locale Boost::date_time Boost::iostreams)

target_compile_definitions(${PROJECT_NAME} 
    PRIVATE NOMINMAX
)

if(UNIX)
	set(CMAKE_C_FLAGS " -Wall -Wno-write-strings")
	set(CMAKE_CXX_FLAGS " -Wall -Wno-write-strings")
	find_library(pthread STATIC)
	target_link_libraries(${PROJECT_NAME} pthread)
endif()