#!/usr/bin/env sh

DIR="i18n"
POT="${DIR}/messages.pot"

xgettext --c++ --no-location -k_ -f xgettext.list -o "${POT}" \
    --copyright-holder=Lasyard \
    --package-name=HomeAccount \
    --package-version=1.1 \
    --msgid-bugs-address=lasyard@yeah.net

EN_US="en_US"
ZH_CN="${DIR}/zh_CN.po"

for locale in "en_US" "zh_CN"; do
    file="${DIR}/${locale}.po"
    if [ -f "${file}" ]; then
        msgmerge -U "${file}" "${POT}"
    else
        msginit -i "${POT}" -o "${file}" --locale ${locale} --no-translator
    fi
done
