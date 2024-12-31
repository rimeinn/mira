#!/bin/bash

cd subprojects/librime
make deps
./install-plugins.sh hchunhui/librime-lua
./install-plugins.sh rime/librime-predict
./install-plugins.sh lotem/librime-octagram

