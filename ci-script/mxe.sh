#!/bin/bash
set -exuo pipefail

#. ./nitrokey-app-source-metadata/metadata
LIBNITROKEY_BUILD_ARTIFACT_VERSION=${git describe}
mkdir -p artifacts
OUTDIR="$(realpath artifacts)"
OUTNAME="libnitrokey-${LIBNITROKEY_BUILD_ARTIFACT_VERSION}.exe"
MXE_TARGET=i686-w64-mingw32.shared

git submodule init
git submodule update --init --recursive

## compile
mkdir -p build/
ls
pushd build
ls
${MXE_TARGET}-qmake-qt5 ..
make -j2 -f Makefile.Release

pushd release
upx libnitrokey.exe --force
cp libnitrokey.exe ${OUTDIR}/${OUTNAME}
popd
popd

upx -t ${OUTDIR}/${OUTNAME}

pushd ${OUTDIR}
sha256sum *.exe > SHA256SUMS
popd
