cmake_minimum_required(VERSION 3.5)

project(ControllerFms LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Network)
find_package(Qt5 REQUIRED COMPONENTS Sql)

file(GLOB SRC_FILES INCLUDE_DIRECTORIES false
    *.cpp
    *.h
    ./common/*.h
    ./common/*.cpp)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_link_libraries(  ${PROJECT_NAME}
                        Qt5::Network
                        Qt5::Sql
                        )
