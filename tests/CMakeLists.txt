# Test configuration
find_package(GTest REQUIRED)

# Test executable
add_executable(chat_tests
    test_simple.cpp
    test_net_endian.cpp
    test_protocol.cpp
    test_messages.cpp
    test_hub.cpp
    test_integration.cpp
    ../server/session.cpp
)

target_link_libraries(chat_tests PRIVATE 
    common 
    asio 
    GTest::gtest 
    GTest::gtest_main
)

# Add coverage flags if building with coverage
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_FLAGS MATCHES "coverage")
    target_compile_options(chat_tests PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    target_link_options(chat_tests PRIVATE --coverage)
endif()

# Set RPATH to find Google Test libraries
set_target_properties(chat_tests PROPERTIES
    INSTALL_RPATH "/usr/local/Caskroom/miniconda/base/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
)

# Enable testing
enable_testing()
add_test(NAME chat_tests COMMAND chat_tests)
set_tests_properties(chat_tests PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)