cmake_minimum_required(VERSION 3.27)
project(Trading-Engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDES
    ${CMAKE_SOURCE_DIR}/src)

set(COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/src/server.cpp
    ${CMAKE_SOURCE_DIR}/src/httpserver.cpp
    ${CMAKE_SOURCE_DIR}/src/Trading-Engine.cpp
    ${CMAKE_SOURCE_DIR}/src/TradeQueue.cpp
    ${CMAKE_SOURCE_DIR}/src/TradeExecution.cpp
    ${CMAKE_SOURCE_DIR}/src/TradeDatabase.cpp
)

set(COMMON_DEFS
    TESTING
)

# -------------------------- BOOST --------------------------
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost REQUIRED COMPONENTS system thread json)
#find_package(libpqxx REQUIRED)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost 1.83 or higher is required. Run cmake task to install dependencies via conan.")
endif()

# if(NOT libpqxx_FOUND)
#     message(FATAL_ERROR "libpqxx is required. Run cmake task to install dependencies via conan.")
# endif()

# ------------------------ THREADING (Cross-platform) ------------------------
# Use CMake's built-in threading support instead of hardcoding pthread
find_package(Threads REQUIRED)

# ------------------------- Executable -------------------------
add_executable(Trading-Engine ${COMMON_SOURCES})

target_compile_definitions(Trading-Engine PRIVATE ${COMMON_DEFS})
target_include_directories(Trading-Engine PRIVATE ${INCLUDES})

# Link libraries
target_link_libraries(Trading-Engine PRIVATE 
    Boost::boost    
    Boost::system
    Boost::thread
    Boost::json
    Threads::Threads  # Cross-platform threading
    #libpqxx::pqxx
)

message(STATUS "Boost found at ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")