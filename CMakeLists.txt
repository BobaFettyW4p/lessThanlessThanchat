cmake_minimum_required(VERSION 3.20)
project(chat LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compile options based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Wall -Wextra -Wpedantic -g -O0)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

# Add coverage flags if specified
if(CMAKE_CXX_FLAGS MATCHES "coverage")
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

# Asio header-only (standalone)
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${CMAKE_SOURCE_DIR}/external/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)

add_library(common INTERFACE
    common/protocol.hpp
    common/serialize.hpp
    common/messages.hpp
    common/ts_queue.hpp
    common/thread_pool.hpp
    common/net_endian.hpp
)
target_include_directories(common INTERFACE ${CMAKE_SOURCE_DIR})

# Add coverage flags to common library if building with coverage
if(CMAKE_CXX_FLAGS MATCHES "coverage")
    target_compile_options(common INTERFACE --coverage -fprofile-arcs -ftest-coverage)
endif()

add_executable(chatd server/server.cpp server/session.cpp)
target_link_libraries(chatd PRIVATE common asio)

add_executable(chatc client/client.cpp)
target_link_libraries(chatc PRIVATE common asio)

# Add coverage flags to executables if building with coverage
if(CMAKE_CXX_FLAGS MATCHES "coverage")
    target_compile_options(chatd PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    target_link_options(chatd PRIVATE --coverage)
    target_compile_options(chatc PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    target_link_options(chatc PRIVATE --coverage)
endif()

# Tests
add_subdirectory(tests)
