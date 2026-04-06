#!/bin/bash

mydir="$(cd "$(dirname "$0")" && pwd)"

if [ "$mydir" == "" ]; then
    echo "mancata definizione directory di build."
else
    echo "Clean in $mydir/debug"
    cd $mydir/debug
    rm -f euridice
    rm -rf CMakeFiles/euridice.dir/src/*
    cd ..
fi
