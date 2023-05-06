#!/bin/bash -e
cmake -S . -B ./build/
cd build
make
cd ..
echo
echo
