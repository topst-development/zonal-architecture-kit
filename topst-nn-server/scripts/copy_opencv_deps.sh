#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PROJECT_DIR=$(cd "${SCRIPT_DIR}/.." && pwd)

SDK_TMP_DEFAULT="${HOME}/topst-sdk/build/ai-g-topst/tmp"
SDK_TMP="${1:-${SDK_TMP_DEFAULT}}"

OPENCV_INCLUDE_SRC="${SDK_TMP}/work/cortexa53-telechips-linux/tc-nn-app/1.0.0-r0/recipe-sysroot/usr/include/opencv4"
OPENCV_LIB_SRC="${SDK_TMP}/work/cortexa53-telechips-linux/tc-nn-app/1.0.0-r0/recipe-sysroot/usr/lib"

TARGET_INCLUDE_DIR="${PROJECT_DIR}/third_party/opencv/include"
TARGET_LIB_DIR="${PROJECT_DIR}/third_party/opencv/lib"

copy_glob() {
    local src_dir="$1"
    local pattern="$2"

    if compgen -G "${src_dir}/${pattern}" > /dev/null; then
        cp -aL "${src_dir}"/${pattern} "${TARGET_LIB_DIR}/"
    else
        echo "missing: ${src_dir}/${pattern}" >&2
        return 1
    fi
}

require_dir() {
    local path="$1"

    if [[ ! -d "${path}" ]]; then
        echo "missing directory: ${path}" >&2
        exit 1
    fi
}

require_dir "${OPENCV_INCLUDE_SRC}"
require_dir "${OPENCV_LIB_SRC}"

mkdir -p "${TARGET_INCLUDE_DIR}" "${TARGET_LIB_DIR}"

rm -rf "${TARGET_INCLUDE_DIR}/opencv4"
cp -a "${OPENCV_INCLUDE_SRC}" "${TARGET_INCLUDE_DIR}/"

copy_glob "${OPENCV_LIB_SRC}" "libopencv_core.so*"
copy_glob "${OPENCV_LIB_SRC}" "libopencv_imgproc.so*"
copy_glob "${OPENCV_LIB_SRC}" "libopencv_imgcodecs.so*"

copy_glob "${SDK_TMP}/sysroots-components/cortexa53/zlib/usr/lib" "libz.so*"
copy_glob "${SDK_TMP}/sysroots-components/cortexa53/tbb/usr/lib" "libtbb.so*"
copy_glob "${SDK_TMP}/sysroots-components/cortexa53/libjpeg-turbo/usr/lib" "libjpeg.so*"
copy_glob "${SDK_TMP}/sysroots-components/cortexa53/libwebp/usr/lib" "libwebp.so*"
copy_glob "${SDK_TMP}/sysroots-components/cortexa53/libpng/usr/lib" "libpng16.so*"
copy_glob "${SDK_TMP}/sysroots-components/cortexa53/tiff/usr/lib" "libtiff.so*"
copy_glob "${SDK_TMP}/sysroots-components/cortexa53/xz/usr/lib" "liblzma.so*"

echo "Copied OpenCV headers and runtime libraries into:"
echo "  ${TARGET_INCLUDE_DIR}"
echo "  ${TARGET_LIB_DIR}"
