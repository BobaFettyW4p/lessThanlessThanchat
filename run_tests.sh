#!/bin/bash

# Chat System Test Runner
# This script builds and runs the test suite

set -e  # Exit on any error

echo "🧪 Chat System Test Runner"
echo "=========================="

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Error: Please run this script from the project root directory"
    exit 1
fi

# Check if GTest is available
echo "🔍 Checking for Google Test..."
if ! pkg-config --exists gtest; then
    echo "⚠️  Warning: Google Test not found via pkg-config"
    echo "   Make sure GTest is installed:"
    echo "   - macOS: brew install googletest"
    echo "   - Ubuntu: sudo apt-get install libgtest-dev"
    echo "   - Or build from source: https://github.com/google/googletest"
fi

# Create build directory
echo "📁 Creating build directory..."
mkdir -p build

# Configure with CMake
echo "⚙️  Configuring with CMake..."
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build the project
echo "🔨 Building project..."
cmake --build build -j

# Check if tests were built
if [ ! -f "build/tests/chat_tests" ]; then
    echo "❌ Error: Test executable not found. Check build output for errors."
    exit 1
fi

# Run tests
echo "🚀 Running tests..."
echo ""

cd build
if command -v ctest &> /dev/null; then
    echo "Using CTest..."
    ctest --output-on-failure
else
    echo "Using direct test executable..."
    ./tests/chat_tests
fi

echo ""
echo "✅ Tests completed!"
echo ""
echo "To run tests manually:"
echo "  ./build/tests/chat_tests"
echo ""
echo "To run specific tests:"
echo "  ./build/tests/chat_tests --gtest_filter=\"NetEndianTest.*\""
echo ""
echo "To run with verbose output:"
echo "  ./build/tests/chat_tests --gtest_verbose"