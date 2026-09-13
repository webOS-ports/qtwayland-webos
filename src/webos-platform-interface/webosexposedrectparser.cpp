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

// QRect(x, y, w, h) stores the far edge as "x + w - 1", and evaluates it in
// that order, so both the intermediate sum and the final edge have to fit in
// an int - overflow in either aborts under Qt's checked-integer assertions
// and silently wraps to garbage geometry otherwise. The compositor supplies
// these values, so check before constructing rather than after.
static inline bool fitsInInt32(int64_t v)
{
    return v >= INT32_MIN && v <= INT32_MAX;
}

static inline bool rectFitsInInt(int32_t x, int32_t y, int32_t w, int32_t h)
{
    const int64_t rightSum = int64_t(x) + int64_t(w);
    const int64_t bottomSum = int64_t(y) + int64_t(h);
    return fitsInInt32(rightSum) && fitsInInt32(rightSum - 1)
        && fitsInInt32(bottomSum) && fitsInInt32(bottomSum - 1);
}

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
            const int32_t x = *(pos + 0);
            const int32_t y = *(pos + 1);
            const int32_t w = *(pos + 2);
            const int32_t h = *(pos + 3);
            if (!rectFitsInInt(x, y, w, h)) {
                qWarning() << "ignoring out-of-range expose rect" << x << y << w << h;
                continue;
            }
            rects << QRect(x, y, w, h);
        } else {
            qWarning() << "missing data from expose rects";
            break;
        }
    }
    return rects;
}
