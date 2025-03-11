#!/bin/bash

# Exit on error
set -e

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Configure with CMake
cmake ..

# Build the project
make

# Run the application
echo -e "\n--- Running the application ---\n"
./main

# Return to original directory
cd ..