QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += $$PWD/pcan

LIBS += $$PWD/pcan/PCANBasic.lib

SOURCES += \
    mTcpServer.cpp \
    main.cpp \
    mainwindow.cpp \
    mtcpsocket.cpp \
    pcancontrol.cpp

HEADERS += \
    headstruct.h \
    mTcpServer.h \
    mainwindow.h \
    mtcpsocket.h \
    pcancontrol.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DEFINES += _AMD64_
