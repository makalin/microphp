#!/bin/bash

# micro-PHP Build Test Script
# This script tests the basic build process for the micro-PHP project

set -e  # Exit on any error

echo "=== micro-PHP Build Test ==="
echo "Testing basic build process..."
echo

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Create build directory
echo "Creating build directory..."
rm -rf build
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build the project
echo "Building project..."
cmake --build . --config Debug

echo
echo "=== Build completed successfully! ==="
echo
echo "Generated files:"
ls -la

echo
echo "Testing microphpc compiler..."
if [ -f "tools/microphpc/microphpc" ]; then
    echo "✓ microphpc compiler built successfully"
    ./tools/microphpc/microphpc --help
else
    echo "✗ microphpc compiler not found"
    exit 1
fi

echo
echo "=== All tests passed! ==="
echo "The micro-PHP project builds successfully."
echo
echo "Next steps:"
echo "1. Test the compiler with: ./build/tools/microphpc/microphpc examples/blink.php -o blink.mbc"
echo "2. Inspect bytecode with: python3 tools/mbc-inspect/mbc_inspect.py blink.mbc"
echo "3. Generate C code with: python3 tools/objgen/objgen.py blink.mbc -o embedded.c"
