cmake_minimum_required(VERSION 3.5)

project(Controller_FMS LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core)

set(MODUL_FMS ControllerFms)

add_subdirectory    (${MODUL_FMS})
include_directories (${MODUL_FMS})

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
                        Qt5::Core
                        ${MODUL_FMS}
                        )
