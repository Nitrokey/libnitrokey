#!/bin/bash
set -exuo pipefail
export
mkdir -p artifacts
OUTDIR="$(realpath artifacts)"

BASENAME="libnitrokey"
#pushd libnitrokey

VERSION="$(git describe --abbrev=0)"
BUILD="${VERSION}.${CI_COMMIT_SHORT_SHA}"
DATE="$(date -Iseconds)"
case "${CI_PIPELINE_SOURCE}" in
  push)
    OUTNAME="${BASENAME}-${BUILD}"
    ;;
  schedule)
    OUTNAME="${BASENAME}-${DATE}"
    ;;
  web)
    OUTNAME="${BASENAME}-${VERSION}"
    ;;
esac

git archive --format tar --prefix ${OUTNAME}/ ${CI_COMMIT_SHA} | gzip > ${OUTDIR}/${OUTNAME}.tar.gz
echo size:
gzip -l ${OUTDIR}/${OUTNAME}.tar.gz

echo "LIBNITROKEY_BUILD_VERSION=\"${VERSION}\"" >> ./metadata
echo "LIBNITROKEY_BUILD_ID=\"${BUILD}\"" >> ./metadata
echo "LIBNITROKEY_BUILD_DATE=\"${DATE}\"" >> ./metadata
echo "LIBNITROKEY_BUILD_TYPE=\"${CI_PIPELINE_SOURCE}\"" >> ./metadata
echo "LIBNITROKEY_BUILD_OUTNAME=\"${OUTNAME}\"" >> ./metadata
cat ./metadata
pwd
ls
mkdir -p libnitrokey-source-metadata
mv metadata libnitrokey-source-metadata/
cat libnitrokey-source-metadata/metadata
pushd ${OUTDIR}
sha256sum *.tar.gz > SHA256SUM
popd
