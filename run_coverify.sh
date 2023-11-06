#!/bin/bash

# Cleanup
rm -rf ./build
rm -rf ./cov-int
rm -rf cmep.xz

# Clean
cmake --build . --target clean

# Run build
cov-build --dir cov-int bash build.sh && cmake --build . --target runtest 

# Compress
tar caf cmep.xz cov-int

post_version=$(date)

curl --form token=edaOX1ta8Mep_UuH35GqrQ \
  --form email=maal-vire@seznam.cz \
  --form file=@./cmep.xz \
  --form version="${post_version}" \
  --form description="A regular debug build" \
  https://scan.coverity.com/builds?project=Snezhnaya-chan%2FCMEP