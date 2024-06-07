#!/bin/sh
# Bash build
# Argument 1 <optional>: Configuration to build

CURRENTCFG=$1

if [[ $# == 0 ]]
then
	CURRENTCFG=Debug	
fi

echo Building $CURRENTCFG

cmake .
cmake --build . -j --parallel --target rungame --config $CURRENTCFG
