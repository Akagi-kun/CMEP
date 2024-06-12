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

cmake .
cmake --build . -j --parallel --target rungame --config $CURRENTCFG
