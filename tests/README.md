# Chat System Test Suite

This directory contains comprehensive unit and integration tests for the chat system.

## Test Structure

### Unit Tests
- **`test_net_endian.cpp`**: Tests for network endian conversion functions
- **`test_protocol.cpp`**: Tests for protocol framing and message serialization
- **`test_messages.cpp`**: Tests for message types and serialization/deserialization
- **`test_hub.cpp`**: Tests for room management and message broadcasting
- **`test_session.cpp`**: Tests for session management and network I/O

### Integration Tests
- **`test_integration.cpp`**: End-to-end tests for client-server communication

## Building and Running Tests

### Prerequisites
- Google Test (GTest) must be installed
- On macOS: `brew install googletest`
- On Ubuntu: `sudo apt-get install libgtest-dev`
- On other systems: Follow GTest installation instructions

### Build Tests
```bash
# From the project root
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

### Run Tests
```bash
# Run all tests
./build/tests/chat_tests

# Run with verbose output
./build/tests/chat_tests --gtest_verbose

# Run specific test suite
./build/tests/chat_tests --gtest_filter="NetEndianTest.*"

# Run specific test
./build/tests/chat_tests --gtest_filter="NetEndianTest.ToBe16RoundTrip"
```

### Run Tests with CMake
```bash
# From build directory
ctest

# With verbose output
ctest --verbose
```

## Test Coverage

### Network Endian Tests
- Round-trip conversion tests
- Known value verification
- Zero and maximum value handling
- Cross-platform compatibility

### Protocol Tests
- Frame header structure and parsing
- Message framing and unframing
- Large message handling
- Special character support
- Round-trip serialization

### Message Tests
- ChatLine serialization/deserialization
- Empty and large message handling
- Special characters and Unicode
- Error handling for malformed data
- Serializable concept compliance

### Hub Tests
- Room joining and management
- Message broadcasting
- Multiple room support
- Expired session cleanup
- Concurrent access safety
- Session lifecycle management

### Session Tests
- Session creation and management
- Message sending
- Network I/O handling
- Error handling
- Large message support

### Integration Tests
- Single and multiple client connections
- Message broadcasting between clients
- Room isolation
- Large message handling
- Connection lifecycle management

## Test Philosophy

The test suite follows these principles:

1. **Comprehensive Coverage**: Tests cover all major components and edge cases
2. **Isolation**: Unit tests are isolated and don't depend on external resources
3. **Realistic Scenarios**: Integration tests simulate real usage patterns
4. **Error Handling**: Tests verify proper error handling and edge cases
5. **Performance**: Tests include large message handling and concurrent access
6. **Cross-Platform**: Tests work on different operating systems

## Adding New Tests

When adding new features:

1. **Unit Tests**: Add tests for new components in appropriate test files
2. **Integration Tests**: Add end-to-end tests for new functionality
3. **Edge Cases**: Test error conditions and boundary values
4. **Documentation**: Update this README with new test descriptions

## Continuous Integration

The test suite is designed to run in CI/CD pipelines:

```bash
# Install dependencies
brew install googletest  # or equivalent for your system

# Build and test
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cd build && ctest --output-on-failure
```

## Troubleshooting

### Common Issues

1. **GTest not found**: Install Google Test library
2. **Build errors**: Ensure C++20 support and Asio headers are available
3. **Test failures**: Check that the main chat system builds successfully first
4. **Integration test timeouts**: Some tests use timeouts; increase if needed on slow systems

### Debug Mode

For debugging test failures:

```bash
# Build with debug symbols
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j

# Run with debugger
gdb ./build/tests/chat_tests
```