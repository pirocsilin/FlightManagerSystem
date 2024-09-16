cmake_minimum_required(VERSION 3.5)

project(Adapter LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Network)
find_package(Qt5 REQUIRED COMPONENTS Concurrent)

file(GLOB SRC_FILES INCLUDE_DIRECTORIES false
    ./*.cpp
    ./*.h
#    ./math/*.h
#    ./math/*.cpp
    ./active_plan/*.h
    ./active_plan/*.cpp
    ./connect/*.h
    ./connect/*.cpp
    ./common/*.h
    ./common/*.cpp)

add_library(Adapter STATIC ${SRC_FILES})

target_link_libraries(Adapter
                        Qt5::Network
                        Qt5::Concurrent
                        )

