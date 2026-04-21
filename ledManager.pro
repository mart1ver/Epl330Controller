QT += core gui network widgets

TARGET = ledManager
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    tcpclient.cpp \
    udpclient.cpp \
    ledbar.cpp

HEADERS += \
    mainwindow.h \
    tcpclient.h \
    udpclient.h \
    ledbar.h

FORMS += \
    mainwindow.ui
