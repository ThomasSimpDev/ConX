#!/bin/bash

mkdir -p build
cd build
cmake ..
make -j$(nproc)

echo "Build complete! Run with: ./build/conx_engine"
echo "Clean with: ./clean.sh"
