#!/usr/bin/env bash

mv ./build/libtdmm/libtdmm.a ./
rm -rf build
cmake -S . -B build
make -C build