// Copyright (c) 2021 LG Electronics, Inc.
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

#include "webospresentationtime_p.h"
#include "webospresentationtime.h"

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandscreen_p.h>

#include "securecoding.h"

#include <limits>

WebOSPresentationFeedbackPrivate::WebOSPresentationFeedbackPrivate(struct ::wp_presentation_feedback *object)
: wp_presentation_feedback(object)
{
}

WebOSPresentationFeedbackPrivate::~WebOSPresentationFeedbackPrivate()
{
    // The generated qtwaylandscanner destructor leaves the wl proxy alive;
    // without this every frame's feedback object leaked one proxy.
    if (object())
        wp_presentation_feedback_destroy(object());
}

void WebOSPresentationFeedbackPrivate::wp_presentation_feedback_sync_output(struct ::wl_output *output)
{
    emit syncOutput(QtWaylandClient::QWaylandScreen::fromWlOutput(output));
}

void WebOSPresentationFeedbackPrivate::wp_presentation_feedback_presented(uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec, uint32_t refresh, uint32_t seq_hi, uint32_t seq_lo, uint32_t flags)
{
    emit presented(tv_sec_hi, tv_sec_lo, tv_nsec, refresh, seq_hi, seq_lo, flags);
}

void WebOSPresentationFeedbackPrivate::wp_presentation_feedback_discarded()
{
    emit discarded();
}

WebOSPresentationTimePrivate::WebOSPresentationTimePrivate(struct ::wl_registry *registry, uint32_t id, int version)
    : QtWayland::wp_presentation(registry, uint2int(id), version)
{

}

void WebOSPresentationTimePrivate::wp_presentation_clock_id(uint32_t clk_id)
{
    qInfo() << "WebOSPresentationTime set clock id to" << clk_id;
    m_clock_id = clk_id;
}

WebOSPresentationTime::WebOSPresentationTime(QWaylandDisplay *display, uint32_t id)
    : QObject(*new WebOSPresentationTimePrivate(display->wl_registry(), id, 1))
{
}

void WebOSPresentationTime::requestFeedback(QWaylandWindow *window)
{
    Q_D(WebOSPresentationTime);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto *surface = window ? window->wlSurface() : nullptr;
#else
    auto *surface = window ? window->object() : nullptr;
#endif
    // The surface is null before the window is shown and after it is hidden;
    // wp_presentation.feedback takes a non-nullable argument.
    if (!surface)
        return;

    auto *feedback = new WebOSPresentationFeedbackPrivate(d->feedback(surface));
    // Parented so feedbacks whose events never arrive (window torn down) are
    // reclaimed with this object instead of accumulating for ever.
    feedback->setParent(this);

    connect(feedback, &WebOSPresentationFeedbackPrivate::syncOutput, this, &WebOSPresentationTime::feedbackSyncOutput);
    connect(feedback, &WebOSPresentationFeedbackPrivate::presented, this, &WebOSPresentationTime::feedbackPresented);
    connect(feedback, &WebOSPresentationFeedbackPrivate::discarded, this, &WebOSPresentationTime::feedbackDiscarded);

    struct timespec ts;
    clock_gettime(uint2int(d->clock_id()), &ts);

    mFeedbacks[feedback] = ts;
}

static uint32_t
timespec_diff_to_usec(const struct timespec *a, const struct timespec *b)
{
    // 64-bit math: an int overflows after ~35 minutes of CLOCK_MONOTONIC,
    // which is exactly what the first frame measures against a zero timespec.
    qint64 secs = qint64(a->tv_sec) - qint64(b->tv_sec);
    qint64 nsec = qint64(a->tv_nsec) - qint64(b->tv_nsec);
    qint64 usec = secs * 1000000 + nsec / 1000;

    if (usec < 0)
        return 0;
    if (usec > qint64(std::numeric_limits<uint32_t>::max()))
        return std::numeric_limits<uint32_t>::max();
    return uint32_t(usec);
}

static inline void
timespec_from_proto(struct timespec *a, uint32_t tv_sec_hi,
        uint32_t tv_sec_lo, uint32_t tv_nsec)
{
    if (sizeof(a->tv_sec) == sizeof(int64_t)) {
        int64_t tv_sec = (int64_t)tv_sec_hi << 32;
        tv_sec += tv_sec_lo;
        a->tv_sec = tv_sec;
    } else {
        a->tv_sec = tv_sec_lo;
    }

    a->tv_nsec = tv_nsec;
}


void WebOSPresentationTime::feedbackSyncOutput(QWaylandScreen *screen)
{
    //TODO: Distinguish screens for the presentation
}

void WebOSPresentationTime::feedbackPresented(uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec, uint32_t refresh, uint32_t seq_hi, uint32_t seq_lo, uint32_t flags)
{
    static struct timespec prevPt= {0, 0};
    auto *feedback = qobject_cast<WebOSPresentationFeedbackPrivate *>(sender());
    if (!feedback) {
        qWarning("Invalid feedback");
        return;
    }

    if (mFeedbacks.contains(feedback)) {
        struct timespec pt;
        timespec_from_proto(&pt, tv_sec_hi, tv_sec_lo, tv_nsec);

        // deliverUpdateRequestToPresentation
        uint32_t d2p = timespec_diff_to_usec(&pt, &mFeedbacks[feedback]);
        // between Presentations; the first frame has no previous
        // presentation, so report 0 instead of the time since boot
        uint32_t p2p = (prevPt.tv_sec == 0 && prevPt.tv_nsec == 0)
            ? 0 : timespec_diff_to_usec(&pt, &prevPt);

        emit presented(d2p, p2p);

        mFeedbacks.remove(feedback);
        prevPt = pt;
    }
    disconnect(feedback);
    feedback->deleteLater();
}

void WebOSPresentationTime::feedbackDiscarded()
{
    auto *feedback = qobject_cast<WebOSPresentationFeedbackPrivate *>(sender());
    if (!feedback) {
        qWarning("Invalid feedback");
        return;
    }
    qWarning() << "feedback discarded";

    mFeedbacks.remove(feedback);
    disconnect(feedback);
    feedback->deleteLater();
}
