#!/bin/bash

java \
    --enable-native-access=ALL-UNNAMED \
    -jar target/sc-java-1.0.jar \
    $*
