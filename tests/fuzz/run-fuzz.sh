#!/bin/bash
# Copyright (c) 2026 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
#
# Builds and runs the libFuzzer targets for the two parsers that consume
# raw compositor-controlled bytes. Host build (clang + host Qt6), not part
# of the cross build - these parsers are architecture-independent, so
# fuzzing them on the host covers the target too.
#
# Usage: ./run-fuzz.sh [seconds-per-target]   (default 60)

set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
OUT="${FUZZ_BUILD_DIR:-$HERE/build}"
DURATION="${1:-60}"

command -v clang++ >/dev/null || { echo "clang++ not found" >&2; exit 1; }
pkg-config --exists Qt6Core || { echo "Qt6Core dev package not found" >&2; exit 1; }

QT_CFLAGS="$(pkg-config --cflags Qt6Core)"
QT_LIBS="$(pkg-config --libs Qt6Core)"

# keysymhelper.h pulls in QKeyEvent (Qt6Gui) and xkbcommon keysym defines.
if pkg-config --exists Qt6Gui; then
    QT_GUI_CFLAGS="$(pkg-config --cflags Qt6Gui)"
    QT_GUI_LIBS="$(pkg-config --libs Qt6Gui)"
else
    QT_GUI_CFLAGS=""
    QT_GUI_LIBS=""
fi
if pkg-config --exists xkbcommon; then
    XKB_CFLAGS="$(pkg-config --cflags xkbcommon)"
else
    XKB_CFLAGS=""
fi

# -fno-sanitize-recover: make UBSan findings fail the run rather than log.
SAN="-fsanitize=fuzzer,address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer -g -O1"

mkdir -p "$OUT"

echo "=== building fuzz_exposed_rects ==="
clang++ $SAN -std=c++17 $QT_CFLAGS \
    -I"$REPO/src/webos-platform-interface" \
    "$HERE/fuzz_exposed_rects.cpp" \
    "$REPO/src/webos-platform-interface/webosexposedrectparser.cpp" \
    $QT_LIBS -o "$OUT/fuzz_exposed_rects"

echo "=== building fuzz_modifiers_map ==="
# keysymhelper.h is header-only for our purposes; it needs xkbcommon
# keysym defines and the webOS key enum from the target sysroot's
# qweboskeyextension.h (a plain macro header, no target dependencies).
KEYEXT_DIR="${WEBOS_KEYEXT_INCLUDE_DIR:-}"
if [ -z "$KEYEXT_DIR" ]; then
    KEYEXT_DIR="$(find /media/herrie/LuneOS/wrynose/webos-ports/tmp/work -name qweboskeyextension.h -path '*recipe-sysroot*' -printf '%h\n' 2>/dev/null | head -1 || true)"
fi
if [ -z "$KEYEXT_DIR" ] || [ ! -f "$KEYEXT_DIR/qweboskeyextension.h" ]; then
    echo "SKIP fuzz_modifiers_map: qweboskeyextension.h not found." >&2
    echo "  Set WEBOS_KEYEXT_INCLUDE_DIR to the directory containing it." >&2
elif [ -z "$QT_GUI_CFLAGS" ]; then
    echo "SKIP fuzz_modifiers_map: Qt6Gui dev package not found." >&2
else
    # Copy the single header into an isolated include dir: adding the target
    # sysroot's /usr/include wholesale would shadow the host libc headers
    # and break this host build. qweboskeyextension.h is a self-contained
    # macro header (only needs QtCore/qglobal.h), so this is safe.
    mkdir -p "$OUT/keyext-include"
    cp "$KEYEXT_DIR/qweboskeyextension.h" "$OUT/keyext-include/"
    clang++ $SAN -std=c++17 $QT_CFLAGS $QT_GUI_CFLAGS $XKB_CFLAGS \
        -I"$REPO/src/plugins/platforminputcontexts/wayland" \
        -I"$OUT/keyext-include" \
        "$HERE/fuzz_modifiers_map.cpp" \
        $QT_LIBS $QT_GUI_LIBS -o "$OUT/fuzz_modifiers_map"
fi

status=0
for target in fuzz_exposed_rects fuzz_modifiers_map; do
    [ -x "$OUT/$target" ] || continue
    echo
    echo "=== running $target for ${DURATION}s ==="
    mkdir -p "$OUT/corpus-$target"
    if ! "$OUT/$target" "$OUT/corpus-$target" \
            -max_total_time="$DURATION" -max_len=4096 -print_final_stats=1; then
        echo "!!! $target FAILED"
        status=1
    fi
done

exit $status
