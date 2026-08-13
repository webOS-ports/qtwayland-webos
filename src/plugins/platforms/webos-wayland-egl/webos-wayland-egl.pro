# Copyright (c) 2015-2021 LG Electronics, Inc.
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

PLUGIN_TYPE = platforms
load(qt_plugin)

QT += waylandclient-private
versionAtLeast(QT_VERSION, 6.0.0) {
    # wayland_egl_client_hw_integration-private is gone: Qt 6.10 moved the
    # QtWayland client into qtbase and now builds the client-side EGL
    # integration as a plugin only, publishing no module. Its sources are
    # compiled into this plugin instead - see qt-wayland-egl-client/.
    QT += wl_shell_integration-private opengl-private
} else {
    QT += egl_support-private
}

# qt-wayland-egl-client holds the imported qtbase sources; the directory above
# it carries a QtWaylandEglClientHwIntegration/private/ tree of forwarding
# headers so the existing includes in this plugin keep resolving unchanged.
INCLUDEPATH += $$PWD/qt-wayland-egl-client

# QtGui's qt_egl_p.h defines USE_X11 unless QT_EGL_NO_X11 is set, and
# eglplatform.h then pulls in <X11/Xlib.h>, which LuneOS does not ship:
#   EGL/eglplatform.h:115:10: fatal error: X11/Xlib.h: No such file or directory
# The define used to arrive with wayland_egl_client_hw_integration-private;
# set it here now that the module is gone.
DEFINES += QT_EGL_NO_X11

# qtbase links these sources against Wayland::Egl; without it the imported
# code leaves wl_egl_window_create/destroy/resize/get_attached_size undefined.
LIBS += -lwayland-egl

qtConfig(xkbcommon) {
    QMAKE_USE_PRIVATE += xkbcommon
}

INCLUDEPATH += ../../../webos-platform-interface
INCLUDEPATH += ../../../../include

LIBS += -L../../../webos-platform-interface -lwebos-platform-interface

OTHER_FILES += \
    webos-wayland-egl.json

SOURCES += \
    qt-wayland-egl-client/qwaylandeglclientbufferintegration.cpp \
    qt-wayland-egl-client/qwaylandeglwindow.cpp \
    qt-wayland-egl-client/qwaylandglcontext.cpp \
    main.cpp \
    webosintegration.cpp \
    webosplatformwindow.cpp \
    webosnativeinterface.cpp \
    weboscursor.cpp \
    webosinputdevice.cpp \
    webosscreen.cpp

HEADERS += \
    qt-wayland-egl-client/qwaylandeglclientbufferintegration_p.h \
    qt-wayland-egl-client/qwaylandeglinclude_p.h \
    qt-wayland-egl-client/qwaylandeglwindow_p.h \
    qt-wayland-egl-client/qwaylandglcontext_p.h \
    weboseglplatformintegration.h \
    webosintegration_p.h \
    webosplatformwindow_p.h \
    webosnativeinterface_p.h \
    weboscursor_p.h \
    webosinputdevice_p.h \
    webosscreen_p.h \
    qtwaylandwebostrace.h

criu {
    DEFINES += HAS_CRIU
    SOURCES += webosappsnapshotmanager.cpp
    HEADERS += webosappsnapshotmanager.h
}

lttng {
    DEFINES += HAS_LTTNG
    SOURCES +=  pmtrace_qtwaylandwebos_provider.c
    HEADERS +=  pmtrace_qtwaylandwebos_provider.h
    !contains(QT_CONFIG, no-pkg-config) {
        CONFIG += link_pkgconfig
        PKGCONFIG += lttng-ust
    } else {
        LIBS += -llttng-ust
    }
}
