#-------------------------------------------------
#
# Project created by QtCreator 2015-08-04T22:07:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = subway
TEMPLATE = app

SOURCES += main.cpp\
        widget.cpp \
    playThread.cpp

HEADERS  += widget.h \
    playThread.h

FORMS    += widget.ui

QMAKE_CXXFLAGS += -fpermissive

GST_DIR = /opt/poky/1.7/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi/usr/

INCLUDEPATH +=  $${GST_DIR}include/gstreamer-1.0 \
                $${GST_DIR}include/glib-2.0 \
                $${GST_DIR}lib/glib-2.0/include


LIBS += -L$${GST_DIR}lib/gstreamer-1.0 \
   -lgstreamer-1.0 -lglib-2.0 -lgobject-2.0

CONFIG += c++11

RESOURCES += \
    image.qrc
