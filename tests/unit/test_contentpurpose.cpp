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
// contentPurposeFromQtHints(): a field's Qt hints to the one text_model
// purpose the input method is told. The purposes are an enum, so whatever
// mix of hints a field has, exactly one known value must come out - a PIN
// field (digits, hidden) used to come out as TIME.

#include "contentpurpose.h"

#include <cstdio>

static int failures = 0;

static void expect(const char *what, Qt::InputMethodHints hints, uint32_t want)
{
    const uint32_t got = contentPurposeFromQtHints(hints);
    if (got != want) {
        std::fprintf(stderr, "FAIL %s: purpose %u, want %u\n", what, got, want);
        ++failures;
    }
}

int main()
{
    expect("nothing", Qt::ImhNone, TEXT_MODEL_CONTENT_PURPOSE_NORMAL);
    expect("digits", Qt::ImhDigitsOnly, TEXT_MODEL_CONTENT_PURPOSE_DIGITS);
    expect("number", Qt::ImhFormattedNumbersOnly, TEXT_MODEL_CONTENT_PURPOSE_NUMBER);
    expect("phone", Qt::ImhDialableCharactersOnly, TEXT_MODEL_CONTENT_PURPOSE_PHONE);
    expect("email", Qt::ImhEmailCharactersOnly, TEXT_MODEL_CONTENT_PURPOSE_EMAIL);
    expect("url", Qt::ImhUrlCharactersOnly, TEXT_MODEL_CONTENT_PURPOSE_URL);
    expect("password", Qt::ImhHiddenText | Qt::ImhSensitiveData | Qt::ImhNoAutoUppercase,
           TEXT_MODEL_CONTENT_PURPOSE_PASSWORD);

    // The case that was wrong: Settings' PIN popup, a TextInput.Password
    // field (which adds ImhHiddenText) with ImhDigitsOnly.
    expect("PIN", Qt::ImhDigitsOnly | Qt::ImhNoPredictiveText | Qt::ImhHiddenText | Qt::ImhSensitiveData,
           TEXT_MODEL_CONTENT_PURPOSE_DIGITS);
    expect("hidden phone", Qt::ImhDialableCharactersOnly | Qt::ImhHiddenText, TEXT_MODEL_CONTENT_PURPOSE_PHONE);
    expect("hidden number", Qt::ImhPreferNumbers | Qt::ImhHiddenText, TEXT_MODEL_CONTENT_PURPOSE_NUMBER);
    expect("prefer numbers", Qt::ImhPreferNumbers | Qt::ImhNoPredictiveText, TEXT_MODEL_CONTENT_PURPOSE_NUMBER);
    expect("date", Qt::ImhDate, TEXT_MODEL_CONTENT_PURPOSE_DATE);
    expect("time", Qt::ImhTime, TEXT_MODEL_CONTENT_PURPOSE_TIME);
    expect("date and time", Qt::ImhDate | Qt::ImhTime, TEXT_MODEL_CONTENT_PURPOSE_DATETIME);

    // Every mix of hints gives one of the protocol's values.
    for (uint32_t bits = 0; bits < (1u << 24); bits += 0x101) {
        const uint32_t got = contentPurposeFromQtHints(Qt::InputMethodHints(int(bits)));
        if (got > TEXT_MODEL_CONTENT_PURPOSE_TERMINAL) {
            std::fprintf(stderr, "FAIL hints 0x%x: purpose %u is not a protocol value\n", bits, got);
            ++failures;
            break;
        }
    }

    if (failures == 0) std::printf("contentpurpose: all passed\n");
    return failures == 0 ? 0 : 1;
}
