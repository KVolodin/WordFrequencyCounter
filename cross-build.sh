#!/bin/bash

BUILD=$PWD/.build
[ -d $BUILD ] && rm -rf $BUILD
mkdir -p $BUILD && cd $BUILD

cmake -G "Unix Makefiles"                 \
      -D CMAKE_BUILD_TYPE:STRING=Debug    \
      -D MAPPED_FILE_BOOST=ON             \
      ../
make


cp freq ../freq