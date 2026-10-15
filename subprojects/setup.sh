#!/bin/bash

LIBRIME_COMMIT="$1"

cd subprojects/librime

if [ "$LIBRIME_COMMIT" != "" ]; then
    git fetch origin
    git checkout "$LIBRIME_COMMIT"
    git submodule update --recursive
fi

make deps
./install-plugins.sh hchunhui/librime-lua
./install-plugins.sh rime/librime-predict
./install-plugins.sh lotem/librime-octagram

# patch librime-lua
cp ../patches/CMakeLists.txt plugins/lua/CMakeLists.txt
