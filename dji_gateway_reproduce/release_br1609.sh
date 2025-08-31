#!/bin/bash

if [ -d "build" ]; then
    rm -rf build
fi

mkdir build
cd build || exit

if [ -d "bin" ]; then
    rm -rf bin
fi
mkdir bin

cmake ..
if [ $? -ne 0 ]; then
    echo "CMake failed"
    exit 1
fi

# 运行make
make
if [ $? -ne 0 ]; then
    echo "Make failed"
    exit 1
fi

# make install || {
#     echo "make install fail, exiting..."
#     exit 1
# }

echo "build success"