# -------------------------------------------------
# Project created by QtCreator 2009-06-27T18:35:33
# -------------------------------------------------
TARGET = twobody
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    picturelistview.cpp \
    thumbdelegate.cpp
HEADERS += mainwindow.h \
    picturelistview.h \
    thumbdelegate.h \
    common.h
FORMS += mainwindow.ui
CONFIG += link_pkgconfig release static
PKGCONFIG += libexif
RESOURCES += twobody.qrc
#OTHER_FILES += twobody_ko_KR.ts
TRANSLATIONS += twobody_ko_KR.ts
RC_FILE = twobody.rc
ICON = images/twobody.icns
