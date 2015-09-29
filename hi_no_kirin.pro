#QT += core gui opengl

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hi_no_kirin
TEMPLATE = app

CONFIG += c++11
CONFIG -= qt

QMAKE_CXXFLAGS += -O3 -msse -msse2 -msse3

QMAKE_CXXFLAGS_DEBUG += -g -Wall -Og -Wno-error=unused-function \
	-Wextra -Wno-missing-field-initializers -Wno-sign-compare -Wno-type-limits  \
	-Wno-unknown-pragmas -Wno-unused-parameter -Wno-ignored-qualifiers          \
	-Wmissing-format-attribute -Wno-delete-non-virtual-dtor                     \
	-Wsizeof-pointer-memaccess -Wformat=2 -Wno-format-nonliteral -Wno-format-y2k\
	-fstrict-overflow -Wstrict-overflow=2 -Wno-div-by-zero -Wwrite-strings      \
	-Wlogical-op -Wundef -DDEBUG_THREADS -Wnonnull -Wstrict-aliasing=2          \
	-fno-omit-frame-pointer -Wno-error=unused-result -Wno-error=clobbered       \
	-fstack-protector-all --param=ssp-buffer-size=4 -Wno-maybe-uninitialized    \
	-Wunused-macros -Wmissing-include-dirs -Wuninitialized -Winit-self          \
	-Wtype-limits -fno-common -fno-nonansi-builtins -Wformat-extra-args         \
	-Wno-error=unused-local-typedefs -DWARN_PEDANTIC -Winit-self -Wdate-time    \
	-Warray-bounds -Werror -fdiagnostics-color=always -fsanitize=address

QMAKE_LFLAGS += -fsanitize=address

SOURCES +=\
	main.cc \
	objects/cube.cc \
	objects/grid.cc \
	objects/levelset.cc \
	objects/treetopology.cc \
	objects/volume.cc \
	render/camera.cc \
	render/GPUBuffer.cc \
	render/GPUShader.cc \
	render/GPUTexture.cc \
	render/scene.cc \
	render/viewer.cc \
	util/util_opengl.cc \
	util/util_openvdb.cc \
	util/utils.cc

HEADERS += \
	objects/cube.h \
	objects/grid.h \
	objects/levelset.h \
	objects/treetopology.h \
	objects/volume.h \
	render/camera.h \
	render/GPUBuffer.h \
	render/GPUShader.h \
	render/GPUTexture.h \
	render/scene.h \
	render/viewer.h \
	util/util_opengl.h \
	util/util_openvdb.h \
	util/utils.h

OTHER_FILES += \
	shader/flat_shader.frag \
	shader/flat_shader.vert \
	shader/object.frag \
	shader/object.vert \
	shader/texture_slicer.frag \
	shader/texture_slicer.vert \
	shader/tree_topo.frag \
	shader/tree_topo.vert

INCLUDEPATH += objects/ render/ util/
INCLUDEPATH += /opt/lib/openvdb/include /opt/lib/openexr/include

LIBS += -lGL -lglut -lGLEW
LIBS += -L/opt/lib/openvdb/lib -lopenvdb -ltbb
LIBS += -L/opt/lib/openexr/lib -lHalf
LIBS += -L/opt/lib/blosc/lib -lblosc -lz

unix {
	copy_files.commands = cp -r ../shader/ .
}

QMAKE_EXTRA_TARGETS += copy_files
POST_TARGETDEPS += copy_files
