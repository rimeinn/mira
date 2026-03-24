#!/bin/bash

cd subprojects/librime

# patch opencc
echo "#include<cstdint>" | cat - deps/opencc/src/SerializedValues.hpp > temp
mv temp deps/opencc/src/SerializedValues.hpp

make deps
./install-plugins.sh hchunhui/librime-lua
./install-plugins.sh rime/librime-predict
./install-plugins.sh lotem/librime-octagram

