#!/bin/bash
set -exuo pipefail
export

#. ./libnitrokey-source-metadata/metadata
#tar xf artifacts/${LIBNITROKEY_BUILD_OUTNAME}.tar.gz


#pushd ${LIBNITROKEY_BUILD_OUTNAME}
#pip3 install --user -r unittest/requirements.txt

## This is quite sketchy but will work for now - we're using a tarball prepared by git archive, so we can't pull submodules.
## Instead, we download the Catch2 header manually. The version is hardcoded, which is bad.
## TODO: figure out a better way to handle that
## One possibility is to install Catch2 system-wide on builder images.
#mkdir -p unittest/Catch/single_include/catch2
#curl -L -o unittest/Catch/single_include/catch2/catch.hpp https://github.com/catchorg/Catch2/releases/download/v2.3.0/catch.hpp

mkdir build
mkdir install

pushd build
cmake .. -DERROR_ON_WARNING=OFF -DCOMPILE_TESTS=ON 
make -j2
ctest -VV
make install DESTDIR=../install
popd

pushd unittest
python3 -m pytest -sv test_offline.py
popd
