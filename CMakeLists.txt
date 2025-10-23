cmake_minimum_required(VERSION 3.8)
project(Trading-Engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDES
    ${CMAKE_SOURCE_DIR}/src)

set(COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/src/server.cpp
    ${CMAKE_SOURCE_DIR}/src/httpserver.cpp
    ${CMAKE_SOURCE_DIR}/src/Trading-Engine.cpp
)

# -------------------------- BOOST --------------------------
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.83 REQUIRED COMPONENTS system thread json)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost 1.83 or higher is required. 
        Install via: sudo apt install libboost-system-dev libboost-thread-dev")
endif()

# ------------------------- Executable -------------------------
add_executable(Trading-Engine ${COMMON_SOURCES})

target_include_directories(Trading-Engine PRIVATE ${INCLUDES} ${Boost_INCLUDE_DIRS})

target_link_libraries(Trading-Engine PRIVATE ${Boost_LIBRARIES} pthread)

message(STATUS "Boost found at ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
