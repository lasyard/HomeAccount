#!/usr/bin/env bash

EXE_FILE="$1"
LIB_PATHS="$2"
APP_DIR="$(dirname "${EXE_FILE}")"

for dll in $(PATH=${PATH}:${LIB_PATHS} ldd "${EXE_FILE}" | grep -v -i 'c/windows' | cut -d ' ' -f 3); do
    cp "${dll}" "${APP_DIR}"
    echo "cp \"${dll}\" to \"${APP_DIR}\"."
done
