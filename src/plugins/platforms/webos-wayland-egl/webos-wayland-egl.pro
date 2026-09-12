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
    # integration as a plugin rather than a module. See the LIBS below for
    # where the implementation comes from instead.
    QT += wl_shell_integration-private opengl-private
} else {
    QT += egl_support-private
}

# QtGui's qt_egl_p.h defines USE_X11 unless QT_EGL_NO_X11 is set, and
# eglplatform.h then pulls in <X11/Xlib.h>, which LuneOS does not ship:
#   EGL/eglplatform.h:115:10: fatal error: X11/Xlib.h: No such file or directory
# The define used to arrive with wayland_egl_client_hw_integration-private;
# set it here now that the module is gone.
DEFINES += QT_EGL_NO_X11

versionAtLeast(QT_VERSION, 6.10.0) {
    # This plugin subclasses QWaylandEglWindow and
    # QWaylandEglClientBufferIntegration. Since Qt 6.10 they live in qtbase's
    # wayland-egl client buffer plugin, which publishes no module - but it
    # does export the types (Q_WAYLANDCLIENT_EXPORT covers their vtables and
    # typeinfo), so link the implementation straight out of it. The
    # declarations come from the private headers that qtbase installs via
    # meta-webos-ports' 9908-wayland-egl-install-the-client-integration-
    # private-headers.patch.
    #
    # The plugin has no SONAME and does not live on the default loader path,
    # so link it by file name and resolve it relative to this plugin's own
    # location: both sit under ${libdir}/plugins/.
    WAYLAND_EGL_CLIENT_PLUGIN_DIR = $$[QT_INSTALL_PLUGINS]/wayland-graphics-integration-client
    LIBS += -L$$WAYLAND_EGL_CLIENT_PLUGIN_DIR -l:libqt-plugin-wayland-egl.so
    QMAKE_RPATHDIR += \$\$ORIGIN/../wayland-graphics-integration-client
}

qtConfig(xkbcommon) {
    QMAKE_USE_PRIVATE += xkbcommon
}

INCLUDEPATH += ../../../webos-platform-interface
INCLUDEPATH += ../../../../include

LIBS += -L../../../webos-platform-interface -lwebos-platform-interface

OTHER_FILES += \
    webos-wayland-egl.json

SOURCES += \
    main.cpp \
    webosintegration.cpp \
    webosplatformwindow.cpp \
    webosnativeinterface.cpp \
    weboscursor.cpp \
    webosinputdevice.cpp \
    webosscreen.cpp

HEADERS += \
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
