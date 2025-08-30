# Terminal Chat (C++20, Asio) — Working Implementation

A fully functional multithreaded terminal chat system with server (`chatd`) and client (`chatc`) components. Features:

- **Networking**: Asio-based async I/O with TCP sockets
- **Threading**: Thread pool with thread-safe message queues
- **Memory Management**: Smart pointers (`shared_ptr`/`weak_ptr`) for safe session lifetimes
- **Message Protocol**: Binary framing with type-safe serialization
- **Room Support**: Multi-room chat with automatic message broadcasting
- **Cross-Platform**: Works on Linux, macOS, and Windows

> ✅ **Status**: Fully functional! Server handles multiple concurrent clients, message broadcasting works, and the client can send/receive messages in real-time.

## Build (standalone Asio)

1) Download [standalone Asio](https://think-async.com/) and put headers under `external/asio/include` so that `external/asio/include/asio.hpp` exists.
2) Configure & build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Build (Boost.Asio alternative)

- Remove `ASIO_STANDALONE` from `CMakeLists.txt`.
- Ensure Boost headers are discoverable (e.g., `brew install boost`), and change include to `<boost/asio.hpp>`.

## Run

- **Server**: `./build/chatd 5555`
- **Client**: `./build/chatc 127.0.0.1 5555 lobby matt`

The server will listen on the specified port and handle multiple concurrent clients. Each client can join different rooms and send messages that are broadcast to all other clients in the same room.

### Example Usage
```bash
# Terminal 1: Start server
./build/chatd 5555

# Terminal 2: Join as user "alice" in "general" room
./build/chatc 127.0.0.1 5555 general alice

# Terminal 3: Join as user "bob" in "general" room  
./build/chatc 127.0.0.1 5555 general bob

# Terminal 4: Join as user "charlie" in "dev" room
./build/chatc 127.0.0.1 5555 dev charlie
```

Messages sent by alice will be seen by bob (same room), but not by charlie (different room).

## Future Extensions

Here are some exciting features we could add to enhance the chat system:

### Core Features
- **User Authentication**: Login/logout with username/password or token-based auth
- **Private Messages**: Direct messaging between users
- **Message History**: Persistent storage and retrieval of chat history
- **User Presence**: Show online/offline status and last seen timestamps
- **Room Management**: Create/delete rooms, set room permissions, room descriptions

### Advanced Features
- **File Sharing**: Send files/images with the chat
- **Message Reactions**: Emoji reactions to messages
- **Message Search**: Full-text search through chat history
- **Voice Chat**: Real-time audio communication
- **Video Chat**: WebRTC-based video calls
- **Screen Sharing**: Share screens during conversations

### Technical Enhancements
- **Database Integration**: SQLite/PostgreSQL for persistent storage
- **Web Interface**: HTML5/WebSocket client for browser access
- **Mobile App**: React Native or Flutter mobile client
- **Message Encryption**: End-to-end encryption for privacy
- **Rate Limiting**: Prevent spam and abuse
- **Load Balancing**: Multiple server instances with Redis pub/sub
- **Docker Support**: Containerized deployment
- **Metrics & Monitoring**: Prometheus metrics and health checks

### Developer Experience
- **Unit Tests**: Comprehensive test suite with Google Test
- **Integration Tests**: End-to-end testing with multiple clients
- **CI/CD Pipeline**: GitHub Actions for automated testing and deployment
- **API Documentation**: OpenAPI/Swagger documentation
- **Configuration Management**: YAML/JSON config files
- **Logging**: Structured logging with different levels

Pick any of these features to work on next! Each would make great additions to the chat system.

## CI/CD Pipeline

This project includes a comprehensive CI/CD pipeline using GitHub Actions:

### 🔄 Automated Testing
- **Multi-platform testing**: Linux, macOS, Windows
- **Multi-compiler support**: GCC, Clang
- **Comprehensive test suite**: Unit tests, integration tests, performance tests
- **Code coverage reporting**: Track test coverage with detailed reports

### 🚀 Workflows

| Workflow | Trigger | Purpose |
|----------|---------|---------|
| [CI/CD Pipeline](.github/workflows/ci.yml) | Push/PR | Main testing and quality checks |
| [Performance Testing](.github/workflows/performance.yml) | Push/PR | Performance benchmarks and stress tests |
| [Code Coverage](.github/workflows/coverage.yml) | Push/PR | Test coverage analysis |
| [Release Build](.github/workflows/release.yml) | Tags | Automated release builds |
| [Dependency Updates](.github/workflows/dependencies.yml) | Weekly | Security and dependency monitoring |

### 📊 Status Badges

[![CI/CD Pipeline](https://github.com/matthewivancic/lessThanlessThanchat/actions/workflows/ci.yml/badge.svg)](https://github.com/matthewivancic/lessThanlessThanchat/actions/workflows/ci.yml)
[![Performance Tests](https://github.com/matthewivancic/lessThanlessThanchat/actions/workflows/performance.yml/badge.svg)](https://github.com/matthewivancic/lessThanlessThanchat/actions/workflows/performance.yml)
[![Code Coverage](https://github.com/matthewivancic/lessThanlessThanchat/actions/workflows/coverage.yml/badge.svg)](https://github.com/matthewivancic/lessThanlessThanchat/actions/workflows/coverage.yml)

### 🛡️ Quality Checks
- **Code formatting**: Automated clang-format checks
- **Static analysis**: Cppcheck and clang-tidy
- **Security scanning**: Trivy vulnerability scanner
- **Documentation validation**: README and test documentation checks

### 📦 Automated Releases
- **Cross-platform builds**: Linux, macOS, Windows binaries
- **Automatic tagging**: Version-based releases
- **Artifact uploads**: Downloadable release packages
- **Release notes**: Automated changelog generation

### 🔧 Local Development
To run the same checks locally:

```bash
# Run the test suite
./run_tests.sh

# Check code formatting
find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run --Werror

# Run static analysis
cppcheck --enable=all --suppress=missingIncludeSystem .

# Build with coverage
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage"
cmake --build build
cd build && ctest
```
