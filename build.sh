#!/bin/sh
# Bash build
# Argument 1 <optional>: Configuration to build

CURRENTCFG=$1

if [[ $# == 0 ]]
then
	CURRENTCFG=Debug	
fi

if [[ "$CURRENTCFG" != "Debug" ]] && [[ "$CURRENTCFG" != "Release" ]]
then
	echo Unknown config "$CURRENTCFG"
    exit
fi

echo Building "$CURRENTCFG"

cmake -DCMAKE_BUILD_TYPE=$CURRENTCFG .
cmake --build . -j 6 --parallel 6 --target rungame
