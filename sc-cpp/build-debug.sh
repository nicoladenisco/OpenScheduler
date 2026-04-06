#!/bin/bash

CMAKE=/usr/bin/cmake
MAKE=/usr/bin/make
IDE_CC=/usr/bin/gcc
IDE_CXX=/usr/bin/g++

rm -rf debug
mkdir -p debug
cd debug

${CMAKE} -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=${IDE_CC} -DCMAKE_CXX_COMPILER=${IDE_CXX} \
  -DCMAKE_C_FLAGS_DEBUG="-g3 -gdwarf-2" -DCMAKE_CXX_FLAGS_DEBUG="-g3 -gdwarf-2" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  ../

${MAKE} -f Makefile
