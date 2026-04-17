#!/bin/bash

DIRGEN="target/native"
echo "Copy JNI adapters in $DIRGEN"

cp -v $DIRGEN/*.h ../sc-cpp/src/
