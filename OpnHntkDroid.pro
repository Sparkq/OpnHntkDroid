TEMPLATE = app

QT += qml quick widgets opengl printsupport

SOURCES += main.cpp \
    src/hantek/control.cpp \
    src/hantek/device.cpp \
    src/hantek/types.cpp \
    src/dataanalyzer.cpp \
    src/dso.cpp \
    src/dsocontrol.cpp \
    src/glgenerator.cpp \
    src/hardcontrol.cpp \
    src/helper.cpp \
    src/settings.cpp \
    src/viewer.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    src/hantek/control.h \
    src/hantek/device.h \
    src/hantek/types.h \
    src/dataanalyzer.h \
    src/dso.h \
    src/dsocontrol.h \
    src/glgenerator.h \
    src/hardcontrol.h \
    src/helper.h \
    src/requests.h \
    src/settings.h \
    src/viewer.h

DEFINES += DEBUG


unix:!macx: LIBS += -L$$PWD/libs/fftw3-armv7a/ -lfftw3

INCLUDEPATH += $$PWD/libs/fftw3-armv7a
DEPENDPATH += $$PWD/libs/fftw3-armv7a

unix:!macx: PRE_TARGETDEPS += $$PWD/libs/fftw3-armv7a/libfftw3.a

unix:!macx: LIBS += -L$$PWD/libs/libusb-armv7a/ -lusb

INCLUDEPATH += $$PWD/libs/libusb-armv7a
DEPENDPATH += $$PWD/libs/libusb-armv7a

unix:!macx: PRE_TARGETDEPS += $$PWD/libs/libusb-armv7a/libusb.a

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
    android/AndroidManifest.xml
