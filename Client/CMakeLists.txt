cmake_minimum_required(VERSION 3.0.0)
project( Client )

# Добавляем исходные файлы для цели
file(GLOB HEADERS "*.hpp")
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCES)

# Формируем цель
add_executable( ${PROJECT_NAME} ${SOURCES} ${HEADERS} )

find_package( 
	Boost 1.64 REQUIRED 
	COMPONENTS regex filesystem system date_time chrono thread locale iostreams
)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE  ${Boost_LIBRARY_DIR_DEBUG})

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(${PROJECT_NAME} boost_regex boost_system boost_filesystem boost_chrono boost_thread boost_locale boost_date_time boost_timer boost_iostreams)

target_compile_definitions(${PROJECT_NAME} 
    PRIVATE NOMINMAX
)

if(UNIX)
	find_library(pthread REQUIRED)
	target_link_libraries(${PROJECT_NAME} pthread)
endif()