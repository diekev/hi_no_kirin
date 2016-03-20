# ***** BEGIN GPL LICENSE BLOCK *****
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# The Original Code is Copyright (C) 2015 Kévin Dietrich.
# All rights reserved.
#
# ***** END GPL LICENSE BLOCK *****

QT += core opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hi_no_kirin
TEMPLATE = app

include(../global.pri)

INCLUDEPATH += $$PWD/../

SOURCES += main.cc

LIBS += $$OUT_PWD/../ui/libui.a
LIBS += $$OUT_PWD/../render/librender.a
LIBS += $$OUT_PWD/../objects/libobjects.a
LIBS += $$OUT_PWD/../smoke/libsmoke.a
LIBS += $$OUT_PWD/../extension/libextension.a
LIBS += $$OUT_PWD/../util/libutils.a
LIBS += $$OUT_PWD/../sculpt/libsculpt.a
LIBS += $$OUT_PWD/../sdk/libsdk.a

LIBS += -L/opt/lib/openvdb/lib -lopenvdb -ltbb
LIBS += -L/opt/lib/openexr/lib -lHalf
LIBS += -L/opt/lib/blosc/lib -lblosc -lz
LIBS += /opt/lib/ego/lib/libego.a
LIBS += -lGL -lGLEW

unix {
	copy_files.commands = cp -r $$PWD/../render/shaders/ .
}

QMAKE_EXTRA_TARGETS += copy_files
POST_TARGETDEPS += copy_files
