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

#include "webosexposedrectparser.h"

#include <QDebug>

#include <cstdint>

QVector<QRect> parseWebOSExposedRects(const void *data, size_t byteSize)
{
    // The event payload is "x,y,w,h, x,y,w,h, ..., -1". The size is in
    // bytes; keep all arithmetic in elements so a payload without the -1
    // sentinel (or a truncated one) cannot send the loop past the array.
    const int32_t* pos = static_cast<const int32_t*>(data);
    const int32_t* const end = pos + byteSize / sizeof(int32_t);

    QVector<QRect> rects;
    for (; pos < end && *pos != -1; pos += 4) {
        if (end - pos >= 4) {
            QRect r(*(pos + 0), *(pos + 1), *(pos + 2), *(pos + 3));
            rects << r;
        } else {
            qWarning() << "missing data from expose rects";
            break;
        }
    }
    return rects;
}
