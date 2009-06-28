# -------------------------------------------------
# Project created by QtCreator 2009-06-27T18:35:33
# -------------------------------------------------
TARGET = twobody
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    preview.cpp
HEADERS += mainwindow.h \
    preview.h
FORMS += mainwindow.ui \
    preview.ui
CONFIG += link_pkgconfig
PKGCONFIG += libexif
RESOURCES += twobody.qrc

