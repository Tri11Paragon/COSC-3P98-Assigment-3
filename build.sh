#!/bin/bash
cmake -B build/ -S ./ -DCMAKE_BUILD_TYPE=Debug && ninja -C build/
