#!/usr/bin/env bash

rm -rf ./libtdmm.a
rm -rf build
cmake -S . -B build
make -C build
mv ./build/libtdmm/libtdmm.a ./