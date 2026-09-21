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
// libFuzzer harness for parseWebOSExposedRects(), which parses the
// wl_webos_shell_surface.exposed event payload. The compositor controls
// this buffer completely; this target exercises the parser against
// arbitrary byte content and lengths the way a real (or malicious)
// compositor could send them, under ASan/UBSan.

#include "webosexposedrectparser.h"

#include <cstdint>
#include <cstddef>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Matches the real call site exactly: rectangles->data, rectangles->size
    // straight off the wire, with no length massaging.
    (void)parseWebOSExposedRects(data, size);
    return 0;
}
