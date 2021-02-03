#!/usr/bin/env bash

APP="HomeAccount"

APP_DIR="${APP}"
EXE_FILE="${APP_DIR}/${APP}.exe"

for dll in $(ldd "${EXE_FILE}" | grep mingw | cut -d ' ' -f 3); do
    cp "${dll}" ${APP_DIR}
done
