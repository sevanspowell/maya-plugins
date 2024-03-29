#!/bin/bash

cmake -E make_directory build
CXX='cc_args.py g++' cmake -E chdir build cmake -E time cmake ../ -DMAYA_VERSION=2016.5
CXX='cc_args.py g++' cmake -E time cmake --build build --target all --config Release --clean-first
find ./build | ag clang_complete | xargs cat | sort | uniq | sed '/-W.*$/ d' > .clang_complete
