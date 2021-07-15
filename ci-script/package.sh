 set -xe

              mkdir output
              OUTDIR="$(realpath output)"

              BASENAME="libnitrokey"

              pushd libnitrokey
              VERSION="$(git describe --abbrev=0 ${GO_REVISION_LIBNITROKEY})"
              BUILD="${VERSION}.${GO_PIPELINE_COUNTER:-0}.${GO_STAGE_COUNTER:-0}"
              DATE="$(date -Iseconds)"

              BUILD_TYPE="continuous"
              git describe --exact-match ${GO_REVISION_LIBNITROKEY} &>/dev/null && BUILD_TYPE="release"
              [ "${GO_TRIGGER_USER}" == "timer" ] && BUILD_TYPE="nightly"
              
              case "${BUILD_TYPE}" in
                continuous)
                  OUTNAME="${BASENAME}-${BUILD}"
                  ;;
                nightly)
                  OUTNAME="${BASENAME}-${DATE}"
                  ;;
                release)
                  OUTNAME="${BASENAME}-${VERSION}"
                  ;;
              esac
              
              git archive --format tar --prefix ${OUTNAME}/ ${GO_REVISION_LIBNITROKEY} | gzip > ${OUTDIR}/${OUTNAME}.tar.gz
              mkdir -p libnitrokey-source-metadata
              popd

              echo "LIBNITROKEY_BUILD_VERSION=\"${VERSION}\"" >> libnitrokey-source-metadata/metadata
              echo "LIBNITROKEY_BUILD_ID=\"${BUILD}\"" >> libnitrokey-source-metadata/metadata
              echo "LIBNITROKEY_BUILD_DATE=\"${DATE}\"" >> libnitrokey-source-metadata/metadata
              echo "LIBNITROKEY_BUILD_TYPE=\"${BUILD_TYPE}\"" >> libnitrokey-source-metadata/metadata
              echo "LIBNITROKEY_BUILD_OUTNAME=\"${OUTNAME}\"" >> libnitrokey-source-metadata/metadata
              
              pushd ${OUTDIR}
              sha256sum *.tar.gz > SHA256SUMS
              popd
