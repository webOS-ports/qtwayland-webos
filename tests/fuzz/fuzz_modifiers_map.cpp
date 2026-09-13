// Copyright (c) 2026 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
//
// libFuzzer harness for XkbQtModifiersMap::applyModifiersMap(), which parses
// the text_model.modifiers_map event payload: a sequence of NUL-terminated
// modifier names. The compositor controls this buffer completely and it is
// scanned with strnlen()/strcmp(); this target exercises it against
// arbitrary content (in particular, buffers with no NUL at all) under
// ASan/UBSan.

// For struct wl_array, which keysymhelper.h's applyWaylandModifiersMap()
// dereferences. We call the byte-oriented applyModifiersMap() directly, but
// the class still has to compile.
#include <wayland-util.h>

#include "keysymhelper.h"

#include <cstdint>
#include <cstddef>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    XkbQtModifiersMap map;
    map.applyModifiersMap(reinterpret_cast<const char *>(data), size);

    // Exercise the consumer side too, across the full bit range a real
    // wl_keyboard.modifiers event can carry.
    for (uint32_t bits : {0u, 1u, 0xffffffffu, 1u << 31})
        (void)map.convertNativeModifiersToQt(bits);

    return 0;
}
