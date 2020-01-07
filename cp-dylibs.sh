#!/usr/bin/env bash

APP="HomeAccount"

APP_DIR="${APP}.app"
FRM_DIR="${APP_DIR}/Contents/Frameworks"
EXE_FILE="${APP_DIR}/Contents/MacOS/${APP}"

dep_list() {
    objdump -macho -dylibs-used -non-verbose "$1" | grep wxWidgets | cut -d' ' -f 1
}

check_dylib() {
    local TARGET="$1"
    local DEPS=$(dep_list "${TARGET}")
    for dep in ${DEPS}; do
        local name="$(basename "${dep}")"
        local target="${FRM_DIR}/${name}"
        echo "Patch file \"${TARGET}\" for \"${name}\"..."
        if [ "${TARGET}" = "${EXE_FILE}" ]; then
            install_name_tool -change "${dep}" "@executable_path/../Frameworks/${name}" "${TARGET}"
        else
            install_name_tool -change "${dep}" "@loader_path/${name}" "${TARGET}"
        fi
        if [ ! -f "${target}" ]; then
            cp "${dep}" "${target}"
            check_dylib ${target}
        fi
    done
}

mkdir -p "${FRM_DIR}"

check_dylib "${EXE_FILE}"
