// Copyright (c) 2014-2021 LG Electronics, Inc.
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

#include "webossurfacegroup.h"
#include "webossurfacegroup_p.h"
#include "webossurfacegrouplayer.h"
#include "webossurfacegrouplayer_p.h"

#include <QWindow>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QDebug>
#include <QMutableListIterator>

WebOSSurfaceGroupPrivate::WebOSSurfaceGroupPrivate()
    : QtWayland::wl_webos_surface_group()
    , q_ptr(0)
{

}

WebOSSurfaceGroupPrivate::~WebOSSurfaceGroupPrivate()
{
    // object() is null when creating the group failed or init() never ran;
    // wl_webos_surface_group_destroy() does not tolerate a null proxy.
    if (object())
        wl_webos_surface_group_destroy(object());
}

void WebOSSurfaceGroupPrivate::setAllowAnonymousLayers(bool allow)
{
    allow_anonymous_layers(allow);
}

// A QWaylandWindow's wl_surface is null before the window is shown and after
// it is hidden; marshalling a null non-nullable argument makes libwayland
// abort the process, so every attach/detach has to check it first.
static inline struct ::wl_surface *surfaceOf(QWaylandWindow* window)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return window ? window->wlSurface() : nullptr;
#else
    return window ? window->object() : nullptr;
#endif
}

void WebOSSurfaceGroupPrivate::attachAnonymousSurface(QWaylandWindow* surface, WebOSSurfaceGroup::ZHint hint)
{
    struct ::wl_surface *ws = surfaceOf(surface);
    if (!ws) {
        qWarning("attachAnonymousSurface: window has no wl_surface");
        return;
    }
    attach_anonymous(ws, (uint32_t)hint);
}

WebOSSurfaceGroupLayer* WebOSSurfaceGroupPrivate::createLayer(const QString& name, int z)
{
     struct ::wl_webos_surface_group_layer* layer = create_layer(name, z);
     WebOSSurfaceGroupLayer* l = new WebOSSurfaceGroupLayer;
     WebOSSurfaceGroupLayerPrivate* p_layer = WebOSSurfaceGroupLayerPrivate::get(l);
     p_layer->init(layer);
     l->setName(name);
     l->setZ(z);
     return l;
}

void WebOSSurfaceGroupPrivate::webos_surface_group_owner_destroyed()
{
    Q_Q(WebOSSurfaceGroup);
    emit q->ownerDestroyed();

    while (!m_attachedSurfaces.isEmpty()) {
        // The QPointer is null when the window was destroyed without
        // detachSurface(), and even a live window may have lost its
        // wl_surface by now - both would crash in detach().
        QPointer<QWaylandWindow> item = m_attachedSurfaces.takeFirst();
        struct ::wl_surface *ws = surfaceOf(item.data());
        if (ws)
            detach(ws);
    }
}

void WebOSSurfaceGroupPrivate::attachSurface(QWaylandWindow* surface, const QString& layer)
{
    struct ::wl_surface *ws = surfaceOf(surface);
    if (!ws) {
        qWarning("attachSurface: window has no wl_surface");
        return;
    }
    attach(ws, layer);
    m_attachedSurfaces << surface;
}

void WebOSSurfaceGroupPrivate::detachSurface(QWaylandWindow* surface)
{
    struct ::wl_surface *ws = surfaceOf(surface);
    if (ws)
        detach(ws);
    m_attachedSurfaces.removeAll(surface);
}

void WebOSSurfaceGroupPrivate::focusOwner()
{
    focus_owner();
}

void WebOSSurfaceGroupPrivate::focusLayer(const QString& layerName)
{
    if (!layerName.isEmpty()) {
        focus_layer(layerName);
    }
}

void WebOSSurfaceGroupPrivate::commitKeyIndex(bool commit)
{
    commit_key_index(commit);
}

// Public class
WebOSSurfaceGroup::WebOSSurfaceGroup()
    : d_ptr(new WebOSSurfaceGroupPrivate)

{
    Q_D(WebOSSurfaceGroup);
    d->q_ptr = this;
}

WebOSSurfaceGroup::~WebOSSurfaceGroup()
{
}

void WebOSSurfaceGroup::setAllowAnonymousLayers(bool allow)
{
    Q_D(WebOSSurfaceGroup);
    d->setAllowAnonymousLayers(allow);
}


void WebOSSurfaceGroup::attachAnonymousSurface(QWindow* surface, ZHint hint)
{
    Q_D(WebOSSurfaceGroup);
    if (surface->handle())
        d->attachAnonymousSurface(static_cast<QWaylandWindow*>(surface->handle()), hint);
}

WebOSSurfaceGroupLayer* WebOSSurfaceGroup::createNamedLayer(const QString& name, int z)
{
    Q_D(WebOSSurfaceGroup);
    return d->createLayer(name, z);
}

void WebOSSurfaceGroup::attachSurface(QWindow* surface, const QString& layer)
{
    Q_D(WebOSSurfaceGroup);
    if (surface->handle())
        d->attachSurface(static_cast<QWaylandWindow*>(surface->handle()), layer);
}

void WebOSSurfaceGroup::detachSurface(QWindow* surface)
{
    Q_D(WebOSSurfaceGroup);
    if (surface->handle())
        d->detachSurface(static_cast<QWaylandWindow*>(surface->handle()));
}

void WebOSSurfaceGroup::focusOwner()
{
    Q_D(WebOSSurfaceGroup);
    d->focusOwner();
}

void WebOSSurfaceGroup::focusLayer(const QString& layerName)
{
    Q_D(WebOSSurfaceGroup);
    d->focusLayer(layerName);
}

void WebOSSurfaceGroup::commitKeyIndex(bool commit)
{
    Q_D(WebOSSurfaceGroup);
    d->commitKeyIndex(commit);
}
