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

#ifndef CONTENTPURPOSE_H
#define CONTENTPURPOSE_H

// Kept apart from waylandinputcontext.cpp so tests/unit can check it on the
// host without a Wayland connection.

#include <QtCore/qnamespace.h>
#include <wayland-text-client-protocol.h>

#include <cstdint>

/*
 * The field's purpose: one of the protocol's values, never a mix of them.
 *
 * The purposes are a plain enum (DIGITS 2, PASSWORD 8, TIME 10 ...), not flags,
 * so they cannot be or'ed together: a PIN field - digits, hidden - used to go
 * out as 2 | 8 = TIME, which the input method does not know, so it showed the
 * full keyboard instead of a keypad. What the field may hold comes first; that
 * it is hidden still reaches the input method through the PASSWORD hint (see
 * contentHintFromQtHints), so the PASSWORD purpose is only for a hidden field
 * that says nothing else.
 */
inline uint32_t contentPurposeFromQtHints(Qt::InputMethodHints hints)
{
    switch (hints & Qt::ImhExclusiveInputMask) {
        case Qt::ImhDigitsOnly: return TEXT_MODEL_CONTENT_PURPOSE_DIGITS;
        case Qt::ImhFormattedNumbersOnly: return TEXT_MODEL_CONTENT_PURPOSE_NUMBER;
        case Qt::ImhDialableCharactersOnly: return TEXT_MODEL_CONTENT_PURPOSE_PHONE;
        case Qt::ImhEmailCharactersOnly: return TEXT_MODEL_CONTENT_PURPOSE_EMAIL;
        case Qt::ImhUrlCharactersOnly: return TEXT_MODEL_CONTENT_PURPOSE_URL;
        default: break;
    }

    if (hints & Qt::ImhPreferNumbers) { return TEXT_MODEL_CONTENT_PURPOSE_NUMBER; }
    if ((hints & Qt::ImhDate) && (hints & Qt::ImhTime)) { return TEXT_MODEL_CONTENT_PURPOSE_DATETIME; }
    if (hints & Qt::ImhDate) { return TEXT_MODEL_CONTENT_PURPOSE_DATE; }
    if (hints & Qt::ImhTime) { return TEXT_MODEL_CONTENT_PURPOSE_TIME; }
    if (hints & Qt::ImhHiddenText) { return TEXT_MODEL_CONTENT_PURPOSE_PASSWORD; }

    return TEXT_MODEL_CONTENT_PURPOSE_NORMAL;
}

#endif // CONTENTPURPOSE_H
