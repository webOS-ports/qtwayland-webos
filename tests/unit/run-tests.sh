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
# Host unit tests for the input context's pure conversions, under ASan and
# UBSan. Not part of the cross build.
#
# Usage: WAYLAND_TEXT_INCLUDE=<dir with wayland-text-client-protocol.h> ./run-tests.sh
# (webos-wayland-extensions installs it, e.g. a Yocto sysroot's usr/include).

set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
OUT="${UNIT_BUILD_DIR:-$HERE/build}"
CXX="${CXX:-c++}"

: "${WAYLAND_TEXT_INCLUDE:?set it to the directory holding wayland-text-client-protocol.h}"
pkg-config --exists Qt6Core || { echo "Qt6Core dev package not found" >&2; exit 1; }

mkdir -p "$OUT"
# shellcheck disable=SC2046
"$CXX" -std=c++17 -g -O1 -Wall -Wextra -Werror -fPIC \
    -fsanitize=address,undefined -fno-sanitize-recover=all \
    -I"$REPO/src/plugins/platforminputcontexts/wayland" -I"$WAYLAND_TEXT_INCLUDE" \
    $(pkg-config --cflags Qt6Core) \
    "$HERE/test_contentpurpose.cpp" -o "$OUT/test_contentpurpose" \
    $(pkg-config --libs Qt6Core)
"$OUT/test_contentpurpose"
