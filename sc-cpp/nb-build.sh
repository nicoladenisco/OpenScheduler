#!/bin/bash

mydir="$(cd "$(dirname "$0")" && pwd)"

if [ "$mydir" == "" ]; then
    echo "mancata definizione directory di build."
else
    echo "Build targets in $mydir/debug"
    cd $mydir/debug
    make -j 4
    cd ..
fi
