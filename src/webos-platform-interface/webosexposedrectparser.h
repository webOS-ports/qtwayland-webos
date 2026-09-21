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

#ifndef WEBOSEXPOSEDRECTPARSER_H
#define WEBOSEXPOSEDRECTPARSER_H

#include <QRect>
#include <QVector>

#include <cstddef>

// Parses the wl_webos_shell_surface.exposed event payload: a sequence of
// (x, y, w, h) int32 quadruples terminated by a -1 sentinel. byteSize is the
// wl_array size in bytes, exactly as delivered by libwayland - the
// compositor controls every byte of this buffer, so this is deliberately
// isolated from the Wayland/Qt-private headers the rest of the plugin needs,
// making it straightforward to fuzz on its own.
QVector<QRect> parseWebOSExposedRects(const void *data, size_t byteSize);

#endif // WEBOSEXPOSEDRECTPARSER_H
