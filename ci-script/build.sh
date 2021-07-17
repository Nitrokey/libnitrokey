#!/bin/bash
set -exuo pipefail
export

. ./libnitrokey-source-metadata/metadata
tar xf output/${LIBNITROKEY_BUILD_OUTNAME}.tar.gz


pushd ${LIBNITROKEY_BUILD_OUTNAME}
pip3 install --user -r unittest/requirements.txt

mkdir build
mkdir install

pushd build
cmake .. -DERROR_ON_WARNING=ON -DCOMPILE_TESTS=ON 
make -j2
ctest -VV
make install DESTDIR=../install
popd

pushd unittest
python3 -m pytest -sv test_offline.py
popd
