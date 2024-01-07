cmake_minimum_required(VERSION 3.10)
project(example)

add_compile_options(-g -Wall -Og -O3 -std=c++11 -Wno-reorder -fpermissive -DIB_DEBUG_3 )

include_directories(    
    ./
)

link_directories(
    
)

find_package(
    Boost REQUIRED COMPONENTS
    thread
    program_options
)

link_libraries(
    Boost::thread
    Boost::program_options
)

add_executable(
    eurekaclient
    EurekaClient.cpp
)


install(
    TARGETS eurekaclient 
    DESTINATION ${PROJECT_SOURCE_DIR}/bin/
)