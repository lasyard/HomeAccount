#!/usr/bin/env bash

EXE_FILE="$1"
FRM_DIR="$(dirname "${EXE_FILE}")/../Frameworks"

dep_list() {
    otool -L "$1" |
        tail +2 |
        grep -v '/System/Library/' |
        grep -v '/usr/lib/' |
        cut -d' ' -f 1
}

check_dylib() {
    local TARGET="$1"
    local DEPS=$(dep_list "${TARGET}")
    if [ "${TARGET}" != "${EXE_FILE}" ]; then
        install_name_tool -id "${name}" "${TARGET}"
    fi
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
            if [ -f "${dep}" ]; then
                cp "${dep}" "${target}"
                check_dylib ${target}
            else
                echo -e "\033[0;31mDylib \"${dep}\" not found.\033[0m"
            fi
        fi
    done
}

mkdir -p "${FRM_DIR}"

check_dylib "${EXE_FILE}"
