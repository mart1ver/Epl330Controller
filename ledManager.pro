QT += core gui network widgets

TARGET = ledManager
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    tcpclient.cpp \
    udpclient.cpp \
    ledbar.cpp \
    panelconfig.cpp \
    networkscanner.cpp \
    paneldialog.cpp \
    orfeoclient.cpp \
    orfeoconfig.cpp

HEADERS += \
    mainwindow.h \
    tcpclient.h \
    udpclient.h \
    ledbar.h \
    panelconfig.h \
    networkscanner.h \
    paneldialog.h \
    orfeoclient.h \
    orfeoconfig.h \
    orfeoevent.h

FORMS += \
    mainwindow.ui
