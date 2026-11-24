#!/bin/bash

FONT_NAME="$1"
FONT_SIZE="$2"

FILE_NAME=$(basename "${FONT_NAME}" .ttf)"-${FONT_SIZE}.png"

read -r -d '' CHARSET << EOF
abcdefghijklmnopqrstuvwxyz
ABCDEFGHIJKLMNOPQRSTUVWXYZ
0123456789.:,;(*!?}^)#\${\%
&-+@'\"[]<>
EOF

convert -font "${FONT_NAME}" -pointsize "${FONT_SIZE}" label:"${CHARSET}" "${FILE_NAME}"
