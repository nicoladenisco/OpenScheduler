#!/bin/bash

sudo apt install -y \
 build-essential cmake \
 libboost-all-dev libxml2-dev
# libboost-thread1.74.0 libboost-date-time1.74.0 libboost-system1.74.0 \
# libboost-regex1.74.0 libboost-program-options1.74.0 libboost-chrono1.74.0

CMAKE=/usr/bin/cmake
MAKE=/usr/bin/make
IDE_CC=/usr/bin/gcc
IDE_CXX=/usr/bin/g++

rm -rf release
mkdir -p release
cd release

${CMAKE} -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=${IDE_CC} -DCMAKE_CXX_COMPILER=${IDE_CXX} \
  -DCMAKE_C_FLAGS_DEBUG="-g3 -gdwarf-2" -DCMAKE_CXX_FLAGS_DEBUG="-g3 -gdwarf-2" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  ../

${MAKE} -f Makefile
